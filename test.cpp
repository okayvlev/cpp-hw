#include <cstdio>
#include <cstring>
#include <chrono>
#include "huffman.h"

using namespace std;

const char* DEFAULT_FILE = "dst.huffman";

int main(int argc, const char* argv[])
{
    using namespace std::chrono;
    auto t0 {high_resolution_clock::now()};

    if (argc <= 2 ||
        (strcmp(argv[1], "compress") != 0 && strcmp(argv[1], "decompress") != 0))
    {
        printf("usage: %s [compress|decompress] [source] [destination=%s]\n", argv[0], DEFAULT_FILE);
        return 0;
    }

    const char* src = argv[2];
    const char* dst = (argc > 3) ? argv[3] : DEFAULT_FILE;

    (strcmp(argv[1], "compress") == 0) ? compress(src, dst) : decompress(src, dst);

    auto t1 {high_resolution_clock::now()};

    printf("Duration: %ld ms\n", duration_cast<milliseconds>(t1 - t0).count());

    return 0;
}
