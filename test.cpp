#include <iostream>
#include <limits>
#include "big_int/big_integer.h"
#include "vector/vector.h"

using namespace std;

int main()
{
    big_integer a = 0x55;
    big_integer b = 0xaa;

    std::cout << "testing " << a << " " << b << "\n";

    std::cout << (a | b) << " " << 0xff << "\n";
    std::cout << (b & -1) << " " << 0xaa << "\n";
    std::cout << (a & (0xaa - 256)) << " " << 0 << "\n";
    std::cout << (a & (0xcc - 256)) << " " << 0x44 << "\n";

    return 0;
}
