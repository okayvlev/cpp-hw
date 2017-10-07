#include <cstdio>
#include <cstring>
#include <chrono>
#include <memory>
#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>
#include "huffman_encoder.h"

#ifndef COLOR_SUPPORT
#define COLOR_SUPPORT 1
#endif

using namespace std;

const char* DEFAULT_FILE = "dst.huf";

std::ifstream is { };
std::ofstream os { };
char read_buffer[BUFFER_SIZE];
char write_buffer[BUFFER_SIZE] { };
unsigned buffer_length;
unsigned buffer_counter;

void init_streams(const char* src, const char* dst)
{
    is = std::ifstream(src, std::ios_base::binary);
    os = std::ofstream(dst, std::ios_base::binary);

    if (!is.is_open()) throw std::runtime_error { "Couldn't open the source file" };
    if (!os.is_open()) throw std::runtime_error { "Couldn't open the destination file" };
}

bool is_file_empty()
{
    return (is.peek() == std::ifstream::traits_type::eof());
}

void bad_file()
{
    std::cout << "Error while decoding file, perhaps the file is corrupted\n";
    exit(0);
}

void read_block(int size = BUFFER_SIZE)
{
    is.read(read_buffer, size);
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
        memset(write_buffer, 0, sizeof(char) * BUFFER_SIZE);    // XOR can mess with dirty bits
    }
}

void write_to_buffer(const huffman_encoder::code& c)
{
    const unsigned offset { buffer_length % CHAR_DIGITS };
    const unsigned roffset { CHAR_DIGITS - offset };

    for (unsigned i = 0; i < c.digits.size() - 1; ++i)
    {
        write_buffer[buffer_length / CHAR_DIGITS] ^= static_cast<unsigned char>(c[i]) >> offset;
        buffer_length += roffset;
        check_buffer();
        write_buffer[buffer_length / CHAR_DIGITS] ^= static_cast<unsigned char>(c[i]) << roffset;
        buffer_length += offset;
        check_buffer();
    }
    unsigned char left { static_cast<unsigned char>(c.size - (c.digits.size() - 1) * CHAR_DIGITS) };

    write_buffer[buffer_length / CHAR_DIGITS] ^= static_cast<unsigned char>(c.digits.back()) >> offset;
    if (left <= roffset)
    {
        buffer_length += left;
        check_buffer();
        return;
    }
    buffer_length += roffset;
    check_buffer();
    write_buffer[buffer_length / CHAR_DIGITS] ^= static_cast<unsigned char>(c.digits.back()) << roffset;
    buffer_length += left - roffset;
    check_buffer();
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
    write_block(buffer_length / CHAR_DIGITS + ((buffer_length % CHAR_DIGITS) > 0));
}

void flush_buffer_to_counter()
{
    write_block(buffer_counter);
}


void compress(const char* src, const char* dst)
{
    init_streams(src, dst);
    if (is_file_empty()) return;
    huffman_encoder encoder { };
    encoder.init_for_compressing();
    buffer_length = 0;
    process_file(encoder.compress_first_iteration);
    encoder.encode();
    encoder.write_header(os);
    process_file([&](char c)
        {
            write_to_buffer(encoder.compress_second_iteration(c));
        });
    flush_buffer();
}

void decompress(const char* src, const char* dst)
{
    init_streams(src, dst);
    if (is_file_empty()) return;
    try
    {
        huffman_encoder encoder { };
        if (!encoder.read_header(is))
        {
            bad_file();
        }
        encoder.init_for_decompressing();
        process_file([&](char c)
        {
            std::vector<char> codes = encoder.decompress_iteration(c);
            for (char c : codes)
            {
                write_char_to_buffer(c);
            }
        }, is.cur);
        if (!is)
            bad_file();
        flush_buffer_to_counter();
    }
    catch (...)
    {
        bad_file();
    }
}


int main(int argc, const char* argv[])
{
    using namespace std::chrono;
    auto t0 { high_resolution_clock::now() };

    if (argc <= 2 || (strcmp(argv[1], "compress") != 0 && strcmp(argv[1], "decompress") != 0))
    {
#if COLOR_SUPPORT == 1
        printf("\033[1;33mUsage\033[0m: %s [compress|decompress] [source] [destination=%s]\n", argv[0], DEFAULT_FILE);
#else
        printf("Usage: %s [compress|decompress] [source] [destination=%s]\n", argv[0], DEFAULT_FILE);
#endif
        return 0;
    }

    const char* src { argv[2] };
    const char* dst { (argc > 3) ? argv[3] : DEFAULT_FILE };

    if (strcmp(src, dst) == 0)
    {
#if COLOR_SUPPORT == 1
        printf("\033[1;31mError\033[0m: source file matches destination file\n");
#else
        printf("Error: source file matches destination file\n");
#endif
        return 0;
    }

    (strcmp(argv[1], "compress") == 0) ? compress(src, dst) : decompress(src, dst);

    auto t1 { high_resolution_clock::now() };

    printf("Duration: %ld ms\n", duration_cast<milliseconds>(t1 - t0).count());

    return 0;
}
