#include "huffman.h"
#include <fstream>
#include <limits>
#include <iostream>
#include <cstring>
#include <vector>
#include <queue>
#include <memory>
#include <functional>

namespace // Implementation details
{
    constexpr int CHAR_MIN { std::numeric_limits<char>::min() };
    constexpr int CHAR_MAX { std::numeric_limits<char>::max() };
    constexpr unsigned CHAR_RANGE { CHAR_MAX - CHAR_MIN + 1 };
    constexpr unsigned CHAR_DIGITS { 8 }; // FIXME /* { std::numeric_limits<char>::digits }; */
    constexpr unsigned BUFFER_SIZE { 64 * 1024 * 1024 };
    constexpr unsigned MAX_BUFFER_LENGTH { CHAR_DIGITS * BUFFER_SIZE };

    typedef unsigned long long ull;
    struct node;
    using ptr = std::shared_ptr<node>;
    using puu = std::pair<ull, ptr>;
    struct node
    {
        ptr left;
        ptr right;
        char c;

        node(char c)
        : left { }, right { }, c { c } { };

        node(ptr left, ptr right, char c)
        : left { left }, right { right }, c { c } { };
    };
    struct code
    {
        unsigned size;  // size in bits
        std::vector<char> digits;

        code()
        : size { }, digits { } { };

        code(const code& other)
        : size { other.size }, digits { other.digits } { };

        code(unsigned size, std::vector<char> digits)
        : size { size }, digits { digits } { };

        const char& operator[](unsigned n) const { return digits[n]; }

        void append(char bit)
        {
            ++size;
            if (size / CHAR_DIGITS == digits.size()) digits.push_back(0);
            if (bit) digits.back() ^= 1 << (CHAR_DIGITS - 1 - (size - 1) % CHAR_DIGITS);
        }

        void pop()
        {
            --size;
            if (size / CHAR_DIGITS < digits.size() - 1) digits.pop_back();
            else digits.back() &= (CHAR_RANGE - 1) ^ (1 << (CHAR_DIGITS - 1 - (size) % CHAR_DIGITS));
        }
    };
    struct anode    // Automata node
    {               // IDEA allocate on heap to fit a huge automata
    private: // NOTE compiler bug???
        //size_t links_[CHAR_RANGE] { };
        //std::vector<char> chars_[CHAR_RANGE] { };
    public:
        size_t small_links[2] { };
        //size_t* links { links_ - CHAR_MIN };
        //std::vector<char>* chars { chars_ - CHAR_MIN };
        char leaf { };

        anode()
        {
            //std::cout << &chars[CHAR_MIN] << " " << chars_ << std::endl;
        }

        friend void build_automata(); // TODO temporary
    };

    std::ifstream is { };
    std::ofstream os { };
    char read_buffer[BUFFER_SIZE];
    char write_buffer[BUFFER_SIZE];  // We don't want to zero the buffers as their size can be too large
    code code_table_[CHAR_RANGE];    // We don't need to default initialize it before every usage
                                     // as all necessary elements will be reset with new values at every initialization
    code* code_table { code_table_ - CHAR_MIN };
    ull char_count_[CHAR_RANGE];     // We NEED to zero this array at every initialization
    ull* char_count { char_count_ - CHAR_MIN };
    code seq { };
    unsigned buffer_length { };
    unsigned buffer_counter { };
    struct
    {
        size_t cur;
        std::vector<anode> v { { } };    // Root is 0

        void add(const code& c, char ch)
        {
            cur = 0;
            for (unsigned i = 0; i < c.size; ++i)
            {
                size_t bit { (c.digits[i / CHAR_DIGITS] & (1 << (CHAR_DIGITS - i % CHAR_DIGITS - 1))) != 0 };
                if (v[cur].small_links[bit] == 0)   // Node is absent
                {
                    v[cur].small_links[bit] = v.size();
                    v.push_back({ });
                }
                cur = v[cur].small_links[bit];
            }
            v[cur].leaf = ch;
        }
/*
        std::vector<char>& go(int c)    // sic!
        {
            return v[cur = v[cur].links[c]].chars[c];
        }
        */
    } ca;  // Code automata

    void init_streams(const char* src, const char* dst)
    {
        is = std::ifstream(src, std::ios_base::binary);
        os = std::ofstream(dst, std::ios_base::binary);

        if (!is.is_open()) throw std::runtime_error { "Couldn't open the source file" };
        if (!os.is_open()) throw std::runtime_error { "Couldn't open the destination file" };
    }

    void init_buffers()
    {
        std::fill(char_count_, char_count_ + CHAR_RANGE, 0);
        buffer_length = 0;
    }

    void read_block(int size = BUFFER_SIZE)
    {
        is.read(read_buffer, size);
    }

    void write_header()
    {
        unsigned unique_chars { };
        for (int c = CHAR_MIN; c <= CHAR_MAX; ++c) { if (char_count[c] > 0) ++unique_chars; }
        os.write(reinterpret_cast<char*>(&unique_chars), sizeof(unsigned)); // TODO consider smaller type

        for (int i = CHAR_MIN; i <= CHAR_MAX; ++i)
        {
            if (char_count[i] > 0ull)
            {
                char c { static_cast<char>(i) }; // TODO static_cast
                os.write(&c, sizeof(char));
                os.write(reinterpret_cast<char*>(&code_table[i].size), sizeof(unsigned));
                for (auto& c : code_table[i].digits) { os.write(&c, sizeof(char)); }
            }
        }
    }

    void read_header()
    {
        unsigned unique_chars { };

        is.read(reinterpret_cast<char*>(&unique_chars), sizeof(unsigned));
        for (unsigned i = 0; i < unique_chars; ++i) {
            char c;
            unsigned size;
            std::vector<char> digits { };

            is.read(&c, sizeof(char));
            is.read(reinterpret_cast<char*>(&size), sizeof(unsigned));
            unsigned len = size / CHAR_DIGITS + (size % CHAR_DIGITS > 0);
            for (unsigned j = 0; j < len; ++j) {
                char c;
                is.read(&c, sizeof(char));
                digits.push_back(c);
            }
            code_table[static_cast<int>(c)] = { size, digits };
            ca.add(code_table[static_cast<int>(c)], c);
/*
            std::cout << (int)c << " " << size << " ";
            for (int k = 7; k >= 0; --k) {
                std::cout << bool((1 << k) & (code_table[static_cast<int>(c)].digits[0]));
            }
            std::cout << "\n";
            */
        }
    }

    void write_block(int size = BUFFER_SIZE)
    {
        os.write(write_buffer, size);
    }

    void check_buffer()
    {
        if (buffer_length == MAX_BUFFER_LENGTH)
        {
            buffer_length = 0;
            write_block();
        }
    }

    void buffer_out()
    {
        std::cout << buffer_length << "\n";
        for (int k = 7; k >= 0; --k) {
            unsigned bit { ((1 << k) & write_buffer[0]) > 0 };
            std::cout << bit;
        }
        for (int k = 7; k >= 0; --k) {
            unsigned bit { ((1 << k) & write_buffer[1]) > 0 };
            std::cout << bit;
        }
        std::cout << "\n";

    }

    void write_to_buffer(const code& c)
    {
        /*
        std::cout << c.size << ":";
        for (int k = 7; k >= 0; --k) {
            unsigned bit { ((1 << k) & c.digits[0]) > 0 };
            std::cout << bit;
        }
        std::cout << "\n";
*/
        const unsigned offset { static_cast<char>(buffer_length % CHAR_DIGITS) };
        const unsigned roffset { static_cast<char>(CHAR_DIGITS - offset) };

        for (unsigned i = 0; i < c.digits.size() - 1; ++i)
        {
            write_buffer[buffer_length / CHAR_DIGITS] ^= c[i] >> offset;
            buffer_length += roffset;
            check_buffer();
            write_buffer[buffer_length / CHAR_DIGITS] ^= c[i] << roffset;
            buffer_length += offset;
            check_buffer();
        }
        char left { static_cast<char>(c.size - (c.digits.size() - 1) * CHAR_DIGITS) };
        /*unsigned int kk = 0;
        std::cout << "xor:" << " " << (static_cast<unsigned char>(c.digits.back())) << " ";
        for (int k = 7; k >= 0; --k) {
            unsigned bit { ((1 << k) & ((static_cast<unsigned char>(c.digits.back())) >> offset)) > 0 };
            std::cout << bit;
        }
        std::cout << "\n";
        */
        write_buffer[buffer_length / CHAR_DIGITS] ^= static_cast<unsigned char>(c.digits.back()) >> offset;
        if (left <= roffset)
        {
            buffer_length += left;
            check_buffer();
            buffer_out();
            return;
        }
        buffer_length += roffset;
        check_buffer();
        write_buffer[buffer_length / CHAR_DIGITS] ^= c.digits.back() << roffset;
        buffer_length += left - roffset;
        check_buffer();

        buffer_out();
    }

    void write_char_to_buffer(char c)   // Don't use it with write_to_buffer
    {
        write_buffer[buffer_counter] = c;
        if (++buffer_counter == BUFFER_SIZE)
        {
            buffer_counter = 0;
            write_block();
        }
    }

    void flush_buffer()
    {
        write_block(buffer_length / CHAR_DIGITS + ((buffer_length % CHAR_DIGITS) > 0)); // TODO consider parenthesis
    }

    void flush_buffer_to_counter()
    {
        write_block(buffer_counter);
    }

    void process_file(std::function<void(char)> func, std::ios_base::seekdir it = is.beg)
    {
        is.seekg(0, it);
        auto pos = is.tellg();
        is.seekg(0, is.end);
        long long remainder { is.tellg() };
        is.seekg(pos);
        remainder -= is.tellg();

        while (remainder > BUFFER_SIZE)
        {
            read_block();
            for (auto& c : read_buffer) { func(c); }
            remainder -= BUFFER_SIZE;
        }

        read_block(remainder);

        for (int i = 0; i < remainder; ++i) { func(read_buffer[i]); }
        is.clear();
    }

    void traverse(ptr cur)
    {
        if (!cur->left && !cur->right)
        {
            if (seq.size == 0) seq.append(0);
            code_table[static_cast<int>(cur->c)] = seq;
        }
        else
        {
            seq.append(1);  // First 1 won't allow "0" code, which can appear at the end of the final char
            traverse(cur->left);
            seq.pop();
            seq.append(0);
            traverse(cur->right);
            seq.pop();
        }
    }

    void encode(ull* char_count)
    {
        std::priority_queue<puu, std::vector<puu>, std::greater<puu>> q;

        for (int c = CHAR_MIN; c <= CHAR_MAX; ++c)
        {
            ull count { char_count[c] };
            if (count > 0ull) q.push(std::make_pair(count, ptr { new node { static_cast<char>(c) } }));
        }
        while (q.size() > 1)
        {
            puu u { q.top() };
            q.pop();
            puu v { q.top() };
            q.pop();
            q.push(std::make_pair(u.first + v.first, ptr { new node { u.second, v.second, std::min(u.second->c, v.second->c) } } ));
        }
        traverse(q.top().second);
    }

/*    void build_automata()   // TODO automata for fast char decoding
    {
        for (size_t i = 0; i < ca.v.size(); ++i)
        {
            for (int c = CHAR_MIN; c <= CHAR_MAX; ++c)
            {
                ca.cur = i;
                for (unsigned j = 0; j < CHAR_DIGITS; ++j)
                {
                    size_t bit { (c & (1u << j)) != 0 };
                    if (ca.v[ca.cur].small_links[bit] == 0) // Assuming there are no new codes in the file
                    {
                        std::cout << c << " " << ca.v[i].chars_ << ":" << &(ca.v[i].chars[CHAR_MIN]) << " " << ca.v[i].chars[c].size() << "\n";
                        ca.v[i].chars[c].push_back(ca.v[ca.cur].leaf);
                    }
                    ca.cur = ca.v[ca.cur].small_links[bit];
                }
                ca.v[i].links[c] = ca.cur;
            }
        }
    }
*/
}

void compress(const char* src, const char* dst)
{
    init_streams(src, dst);
    init_buffers();
    process_file([](char c) { char_count[static_cast<int>(c)]++; });
    encode(char_count);
    write_header();
    process_file([](char c) { write_to_buffer(code_table[static_cast<int>(c)]); });
    flush_buffer();
}

void decompress(const char* src, const char* dst)
{
    init_streams(src, dst);
    read_header();
    //build_automata();
    ca.cur = 0;
    process_file([](char c)
    {
        //for (char& ch : ca.go(c)) { write_char_to_buffer(ch); }       // TODO use automata
        for (int k = 7; k >= 0; --k) {
            unsigned bit { ((1 << k) & c) > 0 };
            //std::cout << bit;
            ca.cur = ca.v[ca.cur].small_links[bit];
            if (ca.v[ca.cur].small_links[0] == 0 && ca.v[ca.cur].small_links[1] == 0)   // is leaf
            {
                write_char_to_buffer(ca.v[ca.cur].leaf);
                ca.cur = 0;
            }
        }
    }, is.cur);
    flush_buffer_to_counter();
    //std::cout << std::endl;
}
