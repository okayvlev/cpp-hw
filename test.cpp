#include <iostream>
#include "big_integer.h"

using namespace std;

int main()
{
    big_integer a { 1 };
    big_integer b { 55 };
    a += b;

    std::cout << (a == b) << std::endl;

    return 0;
}
