#include <iostream>
#include <limits>
#include "big_integer.h"

using namespace std;

int main()
{
    big_integer a = std::numeric_limits<int>::min();
    big_integer b = -a;
    //std::cout << (a + b == -15) << "\n";
    std::cout << b - 1 << "\n";

    return 0;
}
