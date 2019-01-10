#include <iostream>
#include "bimap.h"
#include <map>
#include <chrono>

int main()
{
    // bimap<int, int> c;
    // bimap<int, int>::left_iterator i = c.insert(5, 5);
    // c.insert(2, 2);
    // c.insert(8, 8);
    // c.insert(9, 9);
    // c.erase(i);
    //
    // std::cout << (c.end_left() == c.find_left(2)) << "\n";
    // std::cout << (c.end_right() == c.find_right(2)) << "\n";
    // std::cout << (c.end_left() == c.find_left(8)) << "\n";
    // std::cout << (c.end_right() == c.find_right(8)) << "\n";
    // std::cout << (c.end_left() == c.find_left(9)) << "\n";
    // std::cout << (c.end_right() == c.find_right(9)) << "\n";
    // std::cout << (c.end_left() == c.find_left(5)) << "\n";
    // std::cout << (c.end_right() == c.find_right(5)) << "\n";


    bimap<int, int> a;
    using namespace std::chrono;
    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    int seed = ms.count();
    std::cout << seed << ":\n";

    srand(seed);
    std::map<int, int> b;
    for (int i = 0; i < 15; ++i)
    {
        int x { rand() };
        int y { rand() };
        // std::cout << "insert " << x << " " << y << "\n";
        if (rand() % 5 < 3)
        {
            a.insert(x, y);
            if (b.find(x) == b.end())
                b[x] = y;
        }
        else
        {
            if (a.find_left(x) != a.end_left())
                a.erase(a.find_left(x));
            b.erase(x);
        }
        // a.right_tree.out();
        // a.left_tree.out();
    }
    size_t k { };
    std::cout << b.size() << "\n";
    for (auto it = a.begin_left(); it != a.end_left(); ++it)
    {
        ++k;
        // std::cout << *it << " " << *it.flip() << "\n";
        // std::cout << &it.node_ptr->left << ":"
        //         << it.node_ptr->left.left << " " << it.node_ptr->left.right << " " << it.node_ptr->left.parent << "\n";
        // std::cout << &it.node_ptr->right << ":"
        //         << it.node_ptr->right.left << " " << it.node_ptr->right.right << " " << it.node_ptr->right.parent << "\n";
    }
    std::cout << k << "\n";
    if (k != b.size())
        return 5;
    for (auto it = b.begin(); it != b.end(); ++it)
    {
        ++k;
        if (*a.find_right(it->second).flip() != it->first || *a.find_left(it->first).flip() != it->second)
        {
            std::cout << "WA\n";
            std::cout << k << "\n";
            return 5;
        }
    }
    std::cout << "OK\n";
    // if (b.size() > 0)
    // std::cout << (--b.end())->first << " " << *--a.end_left() << "\n";

    int ans { };
    for (auto it = a.begin_left(); it != a.end_left(); ++it)
    {
        ans = std::max(ans, it.node_ptr->left.height);
        // std::cout << *it << " " << *it.flip() << "\n";
        // std::cout << &it.node_ptr->left << ":"
        //         << it.node_ptr->left.left << " " << it.node_ptr->left.right << " " << it.node_ptr->left.parent << "\n";
        // std::cout << &it.node_ptr->right << ":"
        //         << it.node_ptr->right.left << " " << it.node_ptr->right.right << " " << it.node_ptr->right.parent << "\n";
    }
    std::cout << ans << "\n";

    return 0;
}
