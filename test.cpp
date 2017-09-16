#include <iostream>
#include <limits>
#include "big_integer.h"

using namespace std;

int main()
{
    big_integer a = 5;
    big_integer b = -20;
    //std::cout << (a + b == -15) << "\n";
    a += b;
    std::cout << a << "\n";

    return 0;
}
