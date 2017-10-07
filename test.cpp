#include <iostream>
#include <limits>
#include "big_int/big_integer.h"

using namespace std;

int main()
{
    big_integer a { "30000000000000000000000" };
    big_integer b = a;
    a.out();
    b.out();
    b += 1;
    a.out();
    b.out();

    return 0;
}
