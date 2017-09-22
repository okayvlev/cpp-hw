#include <iostream>
#include <limits>
#include "big_integer.h"

using namespace std;

int main()
{
    // big_integer a("10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    // big_integer b(                                                     "100000000000000000000000000000000000000");
    // big_integer c("100000000000000000000000000000000000000000000000000000");

    big_integer a { "1024" };
    big_integer b { 1 };
    b <<= 32;
    a *= b;

    std::cout << a / b << " " << b  << "\n";

    return 0;
}
