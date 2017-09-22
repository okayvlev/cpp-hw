#include <iostream>
#include <limits>
#include "big_integer.h"

using namespace std;

int main()
{
    // big_integer a("10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    // big_integer b(                                                     "100000000000000000000000000000000000000");
    // big_integer c("100000000000000000000000000000000000000000000000000000");

    big_integer a { 1 };
    big_integer b { 1 };
    (a + b).out();

    // std::cout << a / b << " " << b  << "\n";

    return 0;
}
