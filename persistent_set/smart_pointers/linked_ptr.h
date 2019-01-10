#ifndef LINKED_PTR_H
#define LINKED_PTR_H

template <typename T>
struct linked_ptr
{
    typedef T value_type;

    linked_ptr() noexcept
    { }

    linked_ptr(value_type* ptr) noexcept
        : data { ptr }
    { }

    linked_ptr(const linked_ptr& other) noexcept
        : data { other.data }
    {
        if (!data)
            return;
        left = &other;      // pointer must be const
        right = other.right;
        update_neighbours();
    }

    linked_ptr(linked_ptr&& other) noexcept
        : data { other.data }, left { other.left }, right { other.right }
    {
        other.data = nullptr;    // preventing data from deleting
        update_neighbours();
    }

    ~linked_ptr() noexcept
    {
        if (data) {
            if (!left && !right)
                delete data;
            if (left)
                left->right = right;
            if (right)
                right->left = left;
        }
    }

    linked_ptr& operator=(linked_ptr other) noexcept    // making copy right away instead of explicit copying
    {
        swap(other);
        return *this;
    }

    value_type* get() const noexcept
    {
        return data;
    }

    friend void swap(linked_ptr& a, linked_ptr& b) noexcept
    {
        a.swap(b);
    }

    friend bool operator==(const linked_ptr& a, const linked_ptr& b) noexcept
    {
        return a.data == b.data;
    }

    friend bool operator!=(const linked_ptr& a, const linked_ptr& b) noexcept
    {
        return a.data != b.data;
    }

    operator bool() const noexcept
    {
        return data != nullptr;
    }

private:
    value_type* data { };
    mutable const linked_ptr* left { }; // is mutable to be able to change pointer 
    mutable const linked_ptr* right { };

    void swap(linked_ptr& other) noexcept
    {
        if (other.data == data)
            return;
        std::swap(other.data, data);
        std::swap(other.left, left);
        std::swap(other.right, right);
        update_neighbours();
        other.update_neighbours();
    }

    void update_neighbours() noexcept
    {
        if (!data)
            return;
        if (left)
            left->right = this;
        if (right)
            right->left = this;
    }
};

#endif // LINKED_PTR_H
