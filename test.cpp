#include <iostream>
#include <limits>
#include "big_integer.h"

using namespace std;

int main()
{
    big_integer a;
    big_integer b = -a;
    a.out();
    b.out();

    std::cout << (a == b) << "\n";

    return 0;
}
