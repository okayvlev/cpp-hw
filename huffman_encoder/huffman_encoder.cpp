#include <vector>
#include <queue>
#include <iostream>
#include "huffman_encoder.h"

void huffman_encoder::init_for_compressing()
{
    std::fill(char_count_, char_count_ + CHAR_RANGE, 0);
    file_size = 0;
}

void huffman_encoder::init_for_decompressing()
{
    ca.cur = 0;
    written_bytes = 0;
}

void huffman_encoder::traverse(ptr cur)
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

void huffman_encoder::encode()
{
    std::priority_queue<puu, std::vector<puu>, std::greater<puu>> q;

    for (int c = CHAR_MIN; c <= CHAR_MAX; ++c)
    {
        ull count { char_count[c] };
        if (count > 0ull)
            q.push(std::make_pair(count, ptr { new node { static_cast<char>(c) } }));
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

bool huffman_encoder::read_header(std::istream& is)
{
    unsigned unique_chars { };

    is.read(reinterpret_cast<char*>(&unique_chars), sizeof(unsigned));
    if (unique_chars > CHAR_RANGE)
        return false;
    is.read(reinterpret_cast<char*>(&file_size), sizeof(ull));

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
    }
    if (!is)
        return false;
    return true;
}

void huffman_encoder::write_header(std::ostream& os)
{
    unsigned unique_chars { };

    for (int c = CHAR_MIN; c <= CHAR_MAX; ++c)
    {
        if (char_count[c] > 0)
            ++unique_chars;
    }
    os.write(reinterpret_cast<char*>(&unique_chars), sizeof(unsigned));
    os.write(reinterpret_cast<char*>(&file_size), sizeof(ull));

    for (int i = CHAR_MIN; i <= CHAR_MAX; ++i)
    {
        if (char_count[i] > 0ull)
        {
            char c { static_cast<char>(i) };
            os.write(&c, sizeof(char));
            os.write(reinterpret_cast<char*>(&code_table[i].size), sizeof(unsigned));
            for (auto& c : code_table[i].digits) { os.write(&c, sizeof(char)); }
        }
    }
}
