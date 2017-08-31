#include <cstdio>
#include <cstring>
#include <chrono>
#include "huffman.h"

using namespace std;

const char* DEFAULT_FILE = "dst.huf";

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
