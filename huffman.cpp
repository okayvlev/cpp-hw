#include "huffman.h"
#include <fstream>
#include <limits>
#include <iostream>
#include <cstring>
#include <vector>
#include <queue>
#include <memory>

namespace // Implementation details
{
    constexpr int CHAR_MIN { std::numeric_limits<char>::min() };
    constexpr int CHAR_MAX { std::numeric_limits<char>::max() };
    constexpr unsigned CHAR_RANGE { CHAR_MAX - CHAR_MIN + 1 };
    constexpr unsigned CHAR_DIGITS { 8 }; // FIXME /* { std::numeric_limits<char>::digits }; */
    constexpr unsigned BUFFER_SIZE { 64 * 1024 * 1024 }; // TODO DEBUG
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

    std::ifstream is { };
    std::ofstream os { };
    char read_buffer[BUFFER_SIZE];
    char write_buffer[BUFFER_SIZE];  // We don't want to zero the buffers as its size can be too large
    code code_table_[CHAR_RANGE];   // We don't need to default initialize it before every usage
                                    // as all necessary elements will be reset with new values
                                    // at every initialization
    code* code_table { code_table_ - CHAR_MIN };
    ull char_count_[CHAR_RANGE];    // We NEED to zero this array at every initialization
    ull* char_count { char_count_ - CHAR_MIN };
    code seq { };
    unsigned buffer_length { };

    void init_streams(const char* src, const char* dst)
    {
        is = std::ifstream(src, std::ios_base::binary);
        os = std::ofstream(dst, std::ios_base::binary);

        if (!is.is_open()) throw std::runtime_error { "Couldn't open the source file" };
        if (!os.is_open()) throw std::runtime_error { "Couldn't open the destination file" };
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
            seq.append(0);
            traverse(cur->left);
            seq.pop();
            seq.append(1);
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

    void read_block(int size = BUFFER_SIZE)
    {
        is.read(read_buffer, size);
    }

    void write_header()
    {
        unsigned unique_chars { };
        for (int c = CHAR_MIN; c <= CHAR_MAX; ++c) { if (char_count[c] > 0) ++unique_chars; }
        os.write(reinterpret_cast<char*>(&unique_chars), sizeof(unsigned)); // TODO consider smaller type
        long long sz = 0;

        for (int i = CHAR_MIN; i <= CHAR_MAX; ++i)
        {
            if (char_count[i] > 0ull)
            {
                /*
                std::cout << i << std::endl;

                std::cout << code_table[i].size << " : ";
                if (code_table[i].digits.size() > 0)
                for (int j = 7; j >= 0; --j) {
                    std::cout << bool(code_table[i].digits[0] & (1 << j));
                }
                std::cout << "\n";
*/
                sz += (long long)(char_count[i]) * (long long)(code_table[i].size);
                char c { static_cast<char>(i) }; // TODO static_cast
                os.write(&c, sizeof(char));
                os.write(reinterpret_cast<char*>(&code_table[static_cast<int>(c)].size), sizeof(unsigned));
                for (auto& c : code_table[static_cast<int>(c)].digits) { os.write(&c, sizeof(char)); }
            }
        }
        std::cout << sz << "\n";
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
        }
    }

    long long sz = 0;

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

    void write_to_buffer(const code& c)
    {
        const char offset { static_cast<char>(buffer_length % CHAR_DIGITS) };
        const char roffset { static_cast<char>(CHAR_DIGITS - offset) };

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
        write_buffer[buffer_length / CHAR_DIGITS] ^= c.digits.back() >> offset;
        if (left <= roffset)
        {
            buffer_length += left;
            check_buffer();
            return;
        }
        buffer_length += roffset;
        check_buffer();
        write_buffer[buffer_length / CHAR_DIGITS] ^= c.digits.back() << roffset;
        buffer_length += left - roffset;
        check_buffer();
    }

    void flush_buffer()
    {
        write_block(buffer_length / CHAR_DIGITS + ((buffer_length % CHAR_DIGITS) > 0)); // TODO consider parenthesis
    }

    void process_file(auto func, std::ios_base::seekdir it = is.beg)
    {
        is.seekg(0, is.end);
        long long remainder { is.tellg() };
        is.seekg(0, it);
        remainder -= is.tellg();

        std::cout << remainder << std::endl;

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
}

void compress(const char* src, const char* dst)
{
    std::fill(char_count_, char_count_ + CHAR_RANGE, 0);
    init_streams(src, dst);
    memset(char_count_, 0, sizeof(ull) * CHAR_RANGE);
    process_file([](char c) { char_count[static_cast<int>(c)]++; });
    encode(char_count);
    buffer_length = 0;
    write_header();
    process_file([](char c) { write_to_buffer(code_table[static_cast<int>(c)]); });
    flush_buffer();
    std::cout << sz << std::endl;
}

void decompress(const char* src, const char* dst)
{
    init_streams(src, dst);
    read_header();
    process_file([](char c) {}, is.cur);
    
/*
    while (!is.eof()) {
        check_ifstream(is);
        int len;
        is.read(reinterpret_cast<char*>(&len), sizeof(int));
        if (is.eof()) {
            break;
        }
        memset(buf, 0, ext_buf_size);
        is.read(buf, min(len / 8 + 1, ext_buf_size));
        memset(ext_buf, 0, ext_buf_size);
        int v = 0;
        int k = 0;
        for (int i = 0; i < len; ++i) {
            int to = 0;
            if ((buf[i / 8] & (1 << (i % 8))) > 0) to = 1;
            v = trie[v].to[to];
            if (trie[v].leaf) {
                ext_buf[k++] = trie[v].c;
                v = 0;
            }
        }
        check_ofstream(os);
        os.write(ext_buf, k);
    }
    */
}
