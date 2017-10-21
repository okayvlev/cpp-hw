#include <iostream>
#include "bimap.h"
#include <map>

int main()
{
    bimap<int, int> a;
    srand(time(0));
    int seed = rand();
    std::cout << seed << ":\n";
    srand(seed);
    std::map<int, int> b;
    for (int i = 0; i < 8000; ++i)
    {
        int x { rand() };
        int y { rand() };
        //std::cout << "insert " << x << " " << y << "\n";
        if (rand() % 2)
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
        return 0;
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
    std::cout << (--b.end())->first << " " << *--a.end_left() << "\n";
    //
    // int ans { };
    // for (auto it = a.begin_left(); it != a.end_left(); ++it)
    // {
    //     ans = std::max(ans, it.node_ptr->left.height);
    //     // std::cout << *it << " " << *it.flip() << "\n";
    //     // std::cout << &it.node_ptr->left << ":"
    //     //         << it.node_ptr->left.left << " " << it.node_ptr->left.right << " " << it.node_ptr->left.parent << "\n";
    //     // std::cout << &it.node_ptr->right << ":"
    //     //         << it.node_ptr->right.left << " " << it.node_ptr->right.right << " " << it.node_ptr->right.parent << "\n";
    // }
    // std::cout << ans << "\n";

    return 0;
}
