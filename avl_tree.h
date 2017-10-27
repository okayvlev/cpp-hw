#ifndef AVL_TREE_H
#define AVL_TREE_H

template<class T>
class AVL_tree
{
    typedef T node;

    static constexpr int HEIGHT_DIFF { 1 };

    int get_height(node* t) const
    {
        return t == nullptr ? -1 : t->height;
    }

    void update_height(node* t)
    {
        if (t == nullptr)
            return;
        t->height = std::max(get_height(t->left), get_height(t->right)) + 1;
    }

    void rotate_left(node* t)
    {
        node* p { t->left };
        if (t->parent)
        {
            if (t->parent->left == t)
                t->parent->left = p;
            else
                t->parent->right = p;
        }
        p->parent = t->parent;
        if (p->right)
            p->right->parent = t;
        t->parent = p;
        t->left = p->right;
        p->right = t;
        update_height(p->right);
        update_height(p);
        if (!p->parent)
            root = p;
    }

    void rotate_right(node* t)
    {
        node* p { t->right };
        if (t->parent)
        {
            if (t->parent->left == t)
                t->parent->left = p;
            else
                t->parent->right = p;
        }
        p->parent = t->parent;
        if (p->left)
            p->left->parent = t;
        t->parent = p;
        t->right = p->left;
        p->left = t;
        update_height(p->left);
        update_height(p);
        if (!p->parent)
            root = p;
    }

    void double_rotate_left(node* t)
    {
        rotate_right(t->left);
        rotate_left(t);
    }

    void double_rotate_right(node* t)
    {
        rotate_left(t->right);
        rotate_right(t);
    }

    void balance(node* t)
    {
        if (t && !t->parent)
            root = t;
        while (t)
        {
            node* next { t->parent };
            if (next && !next->parent)
                root = next;
            if (get_height(t->left) - get_height(t->right) > HEIGHT_DIFF)
            {
                if (get_height(t->left->right) > get_height(t->left->left))
                {
                    double_rotate_left(t);
                }
                else
                {
                    rotate_left(t);
                }
            }
            else
            if (get_height(t->right) - get_height(t->left) > HEIGHT_DIFF)
            {
                if (get_height(t->right->left) > get_height(t->right->right))
                {
                    double_rotate_right(t);
                }
                else
                {
                    rotate_right(t);
                }
            }
            update_height(t);
            t = next;
        }
    }

    node* find(node* t, const typename node::value_type& v) const
    {
        if (t == nullptr || v == t->value)
            return t;
        if (v < t->value)
            return find(t->left, v);
        else
            return find(t->right, v);
    }

    void insert(node*& t, node* ptr, node* parent = nullptr)
    {
        if (t == nullptr)
        {
            t = ptr;
            t->parent = parent;
            balance(t);
        }
        else
        if (ptr->value < t->value)
        {
            insert(t->left, ptr, t);
        }
        else
        {
            insert(t->right, ptr, t);
        }
    }

    void remove(node* t, node* ptr)
    {
        if (t == nullptr)
            return;
        if (ptr->value < t->value)
        {
            remove(t->left, ptr);
        }
        else
        if (ptr->value > t->value)
        {
            remove(t->right, ptr);
        }
        else
        if (t->left == nullptr || t->right == nullptr)
        {
            node* ptr { t->parent };
            node* child { t->left ? t->left : t->right };
            if (ptr)
            {
                if (ptr->left == t)
                {
                    ptr->left = child;
                }
                else
                {
                    ptr->right = child;
                }
            }
            if (t->left)
            {
                t->left->parent = ptr;
            }
            else if (t->right)
            {
                t->right->parent = ptr;
            }
            update_height(ptr);
            if (!ptr)
                root = child;
            balance(ptr);
        }
        else
        {
            node* minptr { find_min(t->right) };
            if (minptr == t->right)
            {
                minptr->left = t->left;
                if (t->left)
                    t->left->parent = minptr;
                minptr->parent = t->parent;
                if (t->parent)
                {
                    if (t->parent->left == t)
                        t->parent->left = minptr;
                    else
                        t->parent->right = minptr;
                }
            }
            else
            {
                if (minptr->parent->left == minptr)
                    minptr->parent->left = minptr->right;
                else
                    minptr->parent->right = minptr->right;
                if (minptr->right)
                {
                    minptr->right->parent = minptr->parent;
                }
                minptr->left = t->left;
                minptr->parent = t->parent;
                minptr->right = t->right;
                if (t->parent)
                {
                    if (t->parent->left == t)
                        t->parent->left = minptr;
                    else
                        t->parent->right = minptr;
                }
                if (minptr->left)
                {
                    minptr->left->parent = minptr;
                }
                if (minptr->right)
                {
                    minptr->right->parent = minptr;
                }
            }
            update_height(minptr);
            balance(minptr);
        }
    }

    // void out(node* t)
    // {
    //     if (t == nullptr)
    //         return;
    //     out(t->left);
    //     std::cout << t->value << "[" << t << "]" << ": h=" << t->height << " p=" << t->parent << " l=" << t->left << " r=" << t->right << "\n";
    //     out(t->right);
    // }

public:
    node* root;

    AVL_tree()
        : root { nullptr }
    { }

    ~AVL_tree()
    {
        clear();
    }

    node* find(const typename node::value_type& v)
    {
        return find(root, v);
    }

    void insert(node* ptr)
    {
        insert(root, ptr);
    }

    void remove(node* ptr)
    {
        remove(root, ptr);
    }

    bool empty()
    {
        return root == nullptr;
    }

    void clear()
    {
        root = nullptr;
    }

    node* find_min()
    {
        return find_min(root);
    }

    // void out()
    // {
    //     out(root);
    // }

    static node* find_min(node* t)
    {
        if (t == nullptr)
            return t;
        if (t->left != nullptr)
            return find_min(t->left);
        return t;
    }

    static node* find_max(node* t)
    {
        if (t == nullptr)
            return t;
        if (t->right != nullptr)
            return find_max(t->right);
        return t;
    }

    static node* get_root(node* t)
    {
        if (t == nullptr)
            return t;
        if (t->parent != nullptr)
            return get_root(t->parent);
        return t;
    }

    static node* next(node* t)
    {
        if (t == nullptr)
            return t;
        if (t->right)
            return find_min(t->right);
        node* p { t };
        t = t->parent;
        while (t && p == t->right)
        {
            p = t;
            t = t->parent;
        }
        return t;
    }

    static node* prev(node* t)
    {
        if (t == nullptr)
            return t;
        if (t->left)
            return find_max(t->left);
        node* p { t };
        t = t->parent;
        while (t && p == t->left)
        {
            p = t;
            t = t->parent;
        }
        return t;
    }
};

#endif  // AVL_TREE_H
