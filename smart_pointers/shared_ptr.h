#ifndef SHARED_PTR_H
#define SHARED_PTR_H

template <typename T>
struct shared_ptr
{
    typedef T value_type;

    shared_ptr() noexcept
    { }

    shared_ptr(value_type* ptr) noexcept
        : data { ptr }, ref_counter { ptr ? new size_t { 1 } : nullptr }
    { }

    shared_ptr(const shared_ptr& other) noexcept
        : data { other.data }, ref_counter { other.ref_counter }
    {
        if (data)
            ++*ref_counter;
    }

    shared_ptr(shared_ptr&& other) noexcept
        : data { other.data }, ref_counter { other.ref_counter }
    {
        other.data = nullptr;    // preventing data from deleting
    }

    ~shared_ptr() noexcept
    {
        if (data && !--*ref_counter) {
            delete ref_counter;
            delete data;
        }
    }

    shared_ptr& operator=(shared_ptr other) noexcept    // making copy right away instead of explicit copying
    {
        swap(other);
        return *this;
    }

    value_type* get() const noexcept
    {
        return data;
    }

    friend void swap(shared_ptr& a, shared_ptr& b) noexcept
    {
        a.swap(b);
    }

    friend bool operator==(const shared_ptr& a, const shared_ptr& b) noexcept
    {
        return a.data == b.data;
    }

    friend bool operator!=(const shared_ptr& a, const shared_ptr& b) noexcept
    {
        return a.data != b.data;
    }

    operator bool() const noexcept
    {
        return data != nullptr;
    }

private:
    value_type* data { };
    size_t* ref_counter { };

    void swap(shared_ptr& other) noexcept
    {
        std::swap(other.data, data);
        std::swap(other.ref_counter, ref_counter);
    }
};

#endif // SHARED_PTR_H
