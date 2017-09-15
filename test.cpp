#include <iostream>
#include "big_integer.h"

using namespace std;

int main()
{
    big_integer a { 1 };
    a <<= 33;
    std::cout << "==============\n";
    a /= 2;
    a.out();
    //std::cout << a << "\n";
    //std::cout << (a == b) << std::endl;

    return 0;
}
