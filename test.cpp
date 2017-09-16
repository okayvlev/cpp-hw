#include <iostream>
#include <limits>
#include "big_integer.h"

using namespace std;

int main()
{
    big_integer a = 65536;
    a *= a;
    a >>= 31;
    //std::cout << (a + b == -15) << "\n";
    std::cout << a << "\n";

    return 0;
}
