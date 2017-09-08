#include <iostream>
#include "big_integer.h"

using namespace std;

int main()
{
    big_integer a { 0 };
    a += 101010;
    a *= 3;

    std::cout << "-------=------\n";
    a.out();
    std::cout << "-------=------\n";

    //std::cout << (a == b) << std::endl;

    return 0;
}
