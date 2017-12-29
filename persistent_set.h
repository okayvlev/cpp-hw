#ifndef PERSISTENT_SET_H
#define PERSISTENT_SET_H

#include <iostream>
#include <vector>
#include "smart_pointers/shared_ptr.h"

template <typename T, template <typename> class P = shared_ptr>
class persistent_set
{
public:
    using value_type = T;

    template <typename I>
    using ptr_type = P<I>;

    class iterator;

    persistent_set() noexcept
    { }

    persistent_set(const persistent_set& other) noexcept
        : root { other.root }
    { }

    persistent_set& operator=(persistent_set const& rhs) noexcept
    {
        root = rhs.root;
        return *this;
    }

    ~persistent_set()
    { }

    iterator find(const value_type& v);
    std::pair<iterator, bool> insert(value_type);
    void erase(iterator);

    iterator begin() const;

    iterator end() const noexcept
    {
        return iterator { root };  // storing root for operator--
    }

    void swap(const persistent_set& other) noexcept
    {
        swap(root, other.root);
    }

private:
    struct node;

    using node_ptr = ptr_type<node>;

    struct node
    {
        T value;
        node_ptr left;
        node_ptr right;

        node(const T& value, node_ptr left = nullptr, node_ptr right = nullptr)
            : value { value }, left { left }, right { right }
        { }

        node(T&& value, node_ptr left = nullptr, node_ptr right = nullptr)
            : value { std::move(value) }, left { left }, right { right }
        { }

        node(const node_ptr& ptr)
            : value { ptr.get()->value },
            left { ptr.get()->left },
            right { ptr.get()->right }
        { }
    };

    node_ptr root = nullptr;
};

/*
    NOTE: The tree is unbalanced, hence its height can be O(n),
          where n is the number of elements in the tree. Which means
          that the recursive approach to the tree traversing is
          unacceptable.
*/

template <typename T, template <typename> class P>
typename persistent_set<T, P>::iterator
persistent_set<T, P>::find(const value_type& v)
{
    iterator it { root };
    node_ptr cur { root };

    while (cur != nullptr) {
        it.push(cur);
        if (cur.get()->value == v) {
            return it;
        }
        else {
            cur = cur.get()->value > v ? cur.get()->left : cur.get()->right;
        }
    }

    return end();
}

template <typename T, template <typename> class P>
std::pair<typename persistent_set<T, P>::iterator, bool>
persistent_set<T, P>::insert(value_type v)
{
    if (root == nullptr) {
        root = new node(std::move(v));
        iterator it { root };
        it.push(root);

        return { it, true };
    }

    iterator it { root };
    node_ptr cur { root };
    while (cur != nullptr) {
        it.push(new node(cur));
        if (cur.get()->value == v) {
            return { it, false };
        }
        else {
            cur = cur.get()->value > v ? cur.get()->left : cur.get()->right;
        }
    }

    it.push(new node(std::move(v)));

    /* Linking new branch to the tree */

    for (int i = it.path.size() - 2; i >= 0; --i) {
        if (v < it.path[i].get()->value) {
            it.path[i].get()->left = it.path[i + 1];
        }
        else {
            it.path[i].get()->right = it.path[i + 1];
        }
    }

    root = it.path.front();
    return { it, true };
}

template <typename T, template <typename> class P>
void persistent_set<T, P>::erase(typename persistent_set<T, P>::iterator it)
{
    node* v { it.path.back().get() };
    node_ptr new_v { nullptr };

    if (v->left == nullptr) {
        new_v = v->right;
    }
    else if (v->right == nullptr) {
        new_v = v->left;
    }
    else {
        node* min_ptr { v->right.get() };

        while (min_ptr->left != nullptr) {
            min_ptr = min_ptr->left.get();
        }

        /* Creating a new path excluding minimal node */

        node_ptr cur { new node(v->right) };
        node_ptr right { cur };

        if (v->right.get() != min_ptr) {
            while (cur.get()->left.get() != min_ptr) {
                cur.get()->left = new node(cur.get()->left);
                cur = cur.get()->left;
            }
            cur.get()->left = cur.get()->left.get()->right;
        }
        else {
            right = right.get()->right;
        }
        /* ---------------------------------------------- */

        new_v = new node(min_ptr->value, v->left, right);
    }

    /* Linking new branch to the tree */

    bool is_changed { false };
    node_ptr cur { new_v };
    node* pred = v;

    for (size_t i = it.path.size() - 1; i > 0; --i) {
        if (it.path[i].get() == v) {
            is_changed = true;
        }
        if (is_changed) {
            node* pr { it.path[i - 1].get() };
            cur = new node(
                pr->value,
                (pr->left.get() == pred) ? cur : pr->left,
                (pr->right.get() == pred) ? cur : pr->right);
            pred = pr;
        }
    }

    root = cur;
}


template <typename T, template <typename> class P>
typename persistent_set<T, P>::iterator persistent_set<T, P>::begin() const
{
    iterator it { root };
    node_ptr cur { root };

    while (cur != nullptr) {
        it.push(cur);
        cur = cur.get()->left;
    }

    return it;
}

template <typename T, template <typename> class P>
class persistent_set<T, P>::iterator
{
public:
    iterator() noexcept
    { }

    iterator(const iterator& other) noexcept
    {
        copy(other);
    }

    iterator& operator=(const iterator& rhs) noexcept
    {
        iterator tmp { rhs };
        swap(tmp);
        return *this;
    }

    const value_type& operator*() const
    {
        return path.back().get()->value;
    }

    iterator& operator++();

    iterator operator++(int)
    {
        iterator it { *this };
        operator++();
        return it;
    }

    iterator& operator--();

    iterator operator--(int)
    {
        iterator it { *this };
        operator--();
        return it;
    }

    friend bool operator==(
        const typename persistent_set<T, P>::iterator& a,
        const typename persistent_set<T, P>::iterator& b) noexcept
    {
        return (a.path.empty() ? nullptr : a.path.back()) ==
               (b.path.empty() ? nullptr : b.path.back());
    }

    friend bool operator!=(
        const typename persistent_set<T, P>::iterator& a,
        const typename persistent_set<T, P>::iterator& b) noexcept
    {
        return !(a == b);
    }

private:
    node_ptr origin;
    std::vector<node_ptr> path;

    iterator(const node_ptr& origin) noexcept // doesn't make sense outside of class
        : origin { origin }
    { }

    void copy(const iterator& other) noexcept
    {
        origin = other.origin;
        path = other.path;
    }

    void push(node_ptr v)
    {
        path.push_back(std::move(v));
    }

    void swap(iterator& other) noexcept
    {
        std::swap(other.origin, origin);
        std::swap(other.path, path);
    }

    friend class persistent_set;
};

template <typename T, template <typename> class P>
typename persistent_set<T, P>::iterator& persistent_set<T, P>::iterator::operator++()
{
    if (path.back().get()->right != nullptr) {
        push(path.back().get()->right);
        while (path.back().get()->left != nullptr) {
            push(path.back().get()->left);
        }
    }
    else {
        node_ptr pr { path.back() };
        path.pop_back();
        while (!path.empty() && path.back().get()->right == pr) {
            pr = path.back();
            path.pop_back();
        }
    }

    return *this;   // if path is empty, it will be equal to end()
}

template <typename T, template <typename> class P>
typename persistent_set<T, P>::iterator& persistent_set<T, P>::iterator::operator--()
{
    if (path.size() == 0) {
        node_ptr cur { origin };

        while (cur) {
            push(cur);
            cur = cur.get()->right;
        }

        return *this;
    }
    if (path.back().get()->left != nullptr) {
        push(path.back().get()->left);
        while (path.back().get()->right != nullptr) {
            push(path.back().get()->right);
        }
    }
    else {
        node_ptr pr { path.back() };
        path.pop_back();
        while (path.back().get()->left == pr) { // --begin() is undefined
            pr = path.back();
            path.pop_back();
        }
    }

    return *this;
}

#endif // PERSISTENT_SET_H
