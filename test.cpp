#include <iostream>
#include <limits>
#include "big_integer.h"

using namespace std;

int main()
{
    big_integer a = -1234;
    //std::cout << (a + b == -15) << "\n";
    std::cout << (a >> 3) << " " << (-1234 >> 3) << "\n";

    return 0;
}
