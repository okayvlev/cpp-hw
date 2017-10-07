#ifndef HUFFMAN_ENCODER_H
#define HUFFMAN_ENCODER_H

#include <functional>
#include <limits>
#include <vector>
#include <climits>
#include <memory>

struct huffman_encoder
{
    static constexpr unsigned CHAR_RANGE { CHAR_MAX - CHAR_MIN + 1 };
    static constexpr unsigned CHAR_DIGITS { CHAR_BIT * sizeof(char) };
    static constexpr unsigned BUFFER_SIZE { 64 * 1024 * 1024 };
    static constexpr unsigned MAX_BUFFER_LENGTH { CHAR_DIGITS * BUFFER_SIZE };

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
        unsigned size;  // Size in bits
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
            if (digits.size() * CHAR_DIGITS < size) digits.push_back(0);
            if (bit) digits.back() ^= 1 << (CHAR_DIGITS - 1 - (size - 1) % CHAR_DIGITS);
        }

        void pop()
        {
            --size;
            if ((digits.size() - 1) * CHAR_DIGITS >= size) digits.pop_back();
            else digits.back() &= (CHAR_RANGE - 1) ^ (1 << (CHAR_DIGITS - 1 - (size) % CHAR_DIGITS));
        }
    };
    struct anode    // Automata node
    {               // IDEA build automata of CHAR_DIGITS-links; it can be huge, hence it needs to be allocated on heap
        size_t small_links[2] { };
        char leaf { };
    };

    // --------- Functors for interaction with program --------- //

    std::function<void(char)> compress_first_iteration
    {
        [&](char c)
        {
            ++char_count[static_cast<int>(c)];
            ++file_size;
        }
    };

    std::function<code(char)> compress_second_iteration
    {
        [&](char c)
        {
            return code_table[static_cast<int>(c)];
        }
    };

    std::function<std::vector<char>(char)> decompress_iteration
    {
        [&](char c)
        {
            std::vector<char> res;
            for (int k = CHAR_DIGITS - 1; k >= 0; --k) {
                unsigned bit { ((1 << k) & c) > 0 };
                ca.cur = ca.v[ca.cur].small_links[bit];
                if (ca.v[ca.cur].small_links[0] == 0 && ca.v[ca.cur].small_links[1] == 0)   // Is leaf
                {
                    if (file_size < ++written_bytes) return res;    // Last bit may contain zeroes at the end, which can lead to appearance of extra bytes
                    res.push_back(ca.v[ca.cur].leaf);
                    ca.cur = 0;
                }
            }
            return res;
        }
    };

    // ------------------------------------------------------- //

    code code_table_[CHAR_RANGE];    // We don't need to default initialize it before every usage
                                     // as all necessary elements will be reset with new values at every initialization
    code* code_table { code_table_ - CHAR_MIN };
    ull char_count_[CHAR_RANGE];     // We NEED to zero this array at every initialization
    ull* char_count { char_count_ - CHAR_MIN };
    code seq { };
    ull file_size { };
    ull written_bytes { };
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
    } ca;  // Code automata


    void init_for_compressing();
    void init_for_decompressing();
    void traverse(ptr cur);
    void encode();
    bool read_header(std::istream& is);
    void write_header(std::ostream& os);
};

#endif // HUFFMAN_ENCODER_H
