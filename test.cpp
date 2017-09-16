#include <iostream>
#include <limits>
#include "big_integer.h"

using namespace std;

int main()
{
    big_integer a = 5;
    big_integer b = -20;
    std::cout << (a + b) << "\n";

    return 0;
}
