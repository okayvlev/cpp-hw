#ifndef BIMAP_H
#define BIMAP_H

#include "avl_tree.h"

template<typename L, typename R>
struct bimap
{
    typedef L left_t;
    typedef R right_t;

    template<typename Left, typename Right>
    struct node;

    template<typename T>
    struct avl_node
    {
        typedef T value_type;

        const value_type value;
        avl_node<value_type>* left { };
        avl_node<value_type>* right { };
        avl_node<value_type>* parent { };
        int height { };

        void* origin;

        avl_node(void* origin, const value_type& value)
            : value { value }, origin { origin }
        { }

        avl_node<value_type>* next()
        {
            return AVL_tree<avl_node<value_type>>::next(this);
        }

        avl_node<value_type>* prev()
        {
            return AVL_tree<avl_node<value_type>>::prev(this);
        }
    };

private:
    mutable AVL_tree<avl_node<left_t>>  left_tree  { };
    mutable AVL_tree<avl_node<right_t>> right_tree { };

public:
    template<typename Left, typename Right>
    struct node
    {
        typedef Left left_t;
        typedef Right right_t;

        avl_node<left_t> left;
        avl_node<right_t> right;

        node(const left_t& left_value, const right_t& right_value)
            : left { this, left_value }, right { this, right_value }
        { }
    };

    template<typename Left, typename Right, bool Half>
    struct node_traits;

    template<typename Left, typename Right>
    struct node_traits<Left, Right, 0>
    {
        typedef Left value_type;

        static avl_node<value_type>& get_avl_node(node<Left, Right>* v)
        {
            return v->left;
        }
    };

    template<typename Left, typename Right>
    struct node_traits<Left, Right, 1>
    {
        typedef Right value_type;

        static avl_node<value_type>& get_avl_node(node<Left, Right>* v)
        {
            return v->right;
        }
    };

    template<typename Left, typename Right, bool Half>
    struct iterator
    {
        typedef Left left_t;
        typedef Right right_t;
        static constexpr bool half { Half };
        typedef avl_node<typename node_traits<left_t, right_t, half>::value_type> root_t;

        node<left_t, right_t>* node_ptr;
        root_t* root;

        iterator(root_t* root)
            : node_ptr { nullptr }, root { root }
        { }

        iterator(avl_node<typename node_traits<left_t, right_t, half>::value_type>* avl_node_ptr, root_t* root)
            : node_ptr { static_cast<node<left_t, right_t>*>(!avl_node_ptr ? nullptr : avl_node_ptr->origin) },
            root { root }
        { }

        iterator(node<left_t, right_t>* node_ptr, root_t* root)
            : node_ptr { node_ptr }, root { root }
        { }

        const typename node_traits<left_t, right_t, half>::value_type& operator*() const
        {
            return node_traits<left_t, right_t, half>::get_avl_node(node_ptr).value;
        }

        iterator<left_t, right_t, half>& operator++()
        {
            auto ptr { node_traits<left_t, right_t, half>::get_avl_node(node_ptr).next() };
            node_ptr = static_cast<node<left_t, right_t>*>(!ptr ? nullptr : ptr->origin);
            return *this;
        }

        iterator<left_t, right_t, half> operator++(int)
        {
            iterator<left_t, right_t, half> tmp { node_ptr, root };
            operator++();
            return tmp;
        }

        iterator<left_t, right_t, half>& operator--()
        {
            auto ptr { !node_ptr
                ? AVL_tree<avl_node<typename node_traits<left_t, right_t, half>::value_type>>::find_max(root)
                : node_traits<left_t, right_t, half>::get_avl_node(node_ptr).prev() };

            node_ptr = static_cast<node<left_t, right_t>*>(!ptr ? nullptr : ptr->origin);
            return *this;
        }

        iterator<left_t, right_t, half> operator--(int)
        {
            iterator<left_t, right_t, half> tmp { node_ptr, root };
            operator--();
            return tmp;
        }

        iterator<left_t, right_t, !half> flip() const
        {
            return iterator<left_t, right_t, !half>
            {
                node_ptr,
                AVL_tree<avl_node<typename node_traits<left_t, right_t, !half>::value_type>>::get_root
                    (&node_traits<left_t, right_t, !half>::get_avl_node(node_ptr))
            };
        }

        friend bool operator==(iterator<left_t, right_t, half> a, iterator<left_t, right_t, half> b)
        {
            return a.node_ptr == b.node_ptr && a.root == b.root;
        }

        friend bool operator!=(iterator<left_t, right_t, half> a, iterator<left_t, right_t, half> b)
        {
            return a.node_ptr != b.node_ptr || a.root != b.root;
        }
    };

    typedef iterator<left_t, right_t, 0> left_iterator;
    typedef iterator<left_t, right_t, 1> right_iterator;

    bimap()
    { }

    ~bimap()
    {
        while (begin_left() != end_left())
        {
            erase(begin_left());
        }
    }

    bimap(const bimap&) = delete;
    bimap& operator=(const bimap&) = delete;

    left_iterator insert(const left_t& left_value, const right_t& right_value)
    {
        if (find_left(left_value) != end_left() || find_right(right_value) != end_right())
            return end_left();
        node<L, R>* ptr { new node<L, R> { left_value, right_value } };
        left_tree.insert(&ptr->left);
        right_tree.insert(&ptr->right);
        return left_iterator { ptr, left_tree.root };
    }

    void erase(left_iterator  it)
    {
        left_tree.remove(&it.node_ptr->left);
        right_tree.remove(&it.node_ptr->right);
        delete it.node_ptr;
    }
    void erase(right_iterator it)
    {
        left_tree.remove(&it.node_ptr->left);
        right_tree.remove(&it.node_ptr->right);
        delete it.node_ptr;
    }

    left_iterator find_left(const left_t& v) const
    {
        return left_iterator { left_tree.find(v), left_tree.root };
    }

    right_iterator find_right(const right_t& v) const
    {
        return right_iterator { right_tree.find(v), right_tree.root };
    }

    left_iterator begin_left() const
    {
        return left_iterator { left_tree.find_min(), left_tree.root };
    }

    left_iterator end_left() const
    {
        return left_iterator { left_tree.root };
    }

    right_iterator begin_right() const
    {
        return right_iterator { right_tree.find_min(), right_tree.root };
    }

    right_iterator end_right() const
    {
        return right_iterator { right_tree.root };
    }
};

#endif  // BIMAP_H
