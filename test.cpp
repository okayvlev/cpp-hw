#include <iostream>
#include <limits>
#include "big_int/big_integer.h"
#include "vector/vector.h"

using namespace std;

int main()
{
    // big_integer a("10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
    // big_integer b(                                                     "100000000000000000000000000000000000000");
    // big_integer c("100000000000000000000000000000000000000000000000000000");
    //
    big_integer a { 1 };
    big_integer b { 2 };

    big_integer c { a + b };
    c.out();
    // std::cout << a / b << " " << b  << "\n";

    return 0;
}
