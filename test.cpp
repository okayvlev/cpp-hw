#include <iostream>
#include <limits>
#include "big_integer.h"

using namespace std;

int main()
{
    big_integer a = std::numeric_limits<int>::min();
    big_integer b = std::numeric_limits<int>::max();
    std::cout << a << " " << b << "\n";
    big_integer res { a + b };

    std::cout << res << "\n";
    //std::cout << (a == b) << std::endl;

    return 0;
}
