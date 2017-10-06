#ifndef VECTOR_H
#define VECTOR_H

#include <cassert>
#include <cstring>

template<typename T>
struct vector
{
    using value_type = T;

    vector();
    vector(value_type);
    vector(const vector&);
    ~vector();
    vector& operator=(const vector& other);

    value_type& operator[](size_t n) { return array[n]; }
    value_type& ref_counter() { return array[-2]; }
    value_type& size() { return array[-1]; }
    const value_type& operator[](size_t n) const { return array[n]; }
    const value_type& ref_counter() const { return array[-2]; }
    const value_type& size() const { return array[-1]; }

    void shrink_to_fit();
    void ensure_capacity(size_t new_size);
    void detach();

    void out() const;   // debugging

    template <typename U>
    friend void swap(vector<U>& a, vector<U>& b);
private:
    value_type* array;
    static constexpr size_t OFFSET { 2 };

    void allocate(size_t new_size);
    void quick_allocate(size_t new_size);
    void quick_copy(const vector& other);
};

template <typename T>
vector<T>::vector()
{
    allocate(0);   // This will also zero-initialize size and sign
    ref_counter() = 1;
}

template <typename T>
vector<T>::vector(value_type e)
{
    allocate(1);   // This will also zero-initialize sign
    size() = 1;
    ref_counter() = 1;
    array[0] = e;
}

template <typename T>
void vector<T>::quick_copy(const vector& other)
{
    array = other.array;
    ++ref_counter();
}

template <typename T>
vector<T>::vector(const vector& other)
{
    std::cout << "&copy\n";
    quick_copy(other);
}

template <typename T>
vector<T>& vector<T>::operator=(const vector& other)
{
    std::cout << "&=\n";
    quick_copy(other);
    return *this;
}

template <typename T>
vector<T>::~vector()
{
    std::cout << "~vector\n";
    if (--ref_counter() == 0)
    {
        std::cout << "delete at " << array << "\n";
        delete[](array - OFFSET);
    }
}

template <typename T>
void vector<T>::allocate(size_t new_size)
{
    array = new value_type[new_size + OFFSET] { } + OFFSET;
    std::cout << "allocate at " << array << "\n";
}

template <typename T>
void vector<T>::quick_allocate(size_t new_size)
{
    array = new value_type[new_size + OFFSET] + OFFSET;
    std::cout << "quick allocate at " << array << "\n";
}

template <typename T>
void vector<T>::detach()
{
    std::cout << "detach " << ref_counter() << "\n";
    if (ref_counter() == 1)
        return;
    --ref_counter();
    value_type* old_array { array };
    size_t size_ { size() };
    quick_allocate(size_);
    memcpy(array - 1, old_array - 1, size_ + 1);
    ref_counter() = 1;
}

template <typename T>
void vector<T>::shrink_to_fit()
{
    assert(ref_counter() == 1);
    size_t size_ { size() };
    while (size_ > 1 && array[size_ - 1] == 0u)
        --size_;
    if (size_ == size())
        return;
    value_type* old_array { array };
    quick_allocate(size_);
    memcpy(array - OFFSET, old_array - OFFSET, size_ + OFFSET);
    std::cout << "delete[s] at " << old_array << "\n";
    delete[](old_array - OFFSET);
    size() = size_;
}

template <typename T>
void vector<T>::ensure_capacity(size_t new_size)
{
    assert(ref_counter() == 1);
    if (size() >= new_size)
        return;
    size_t size_ { size() };
    value_type* old_array { array };
    allocate(new_size);
    memcpy(array - OFFSET, old_array - OFFSET, size_ + OFFSET);
    std::cout << "delete[e] at " << old_array << "\n";
    delete[](old_array - OFFSET);
    size() = new_size;
}

template <typename T>
void vector<T>::out() const
{
    std::cout << "out\n";
    std::cout << array << "\n";
    std::cout << '[' << ref_counter() << ", " << size() << "]: ";
    for (size_t i = 0; i < size(); ++i)
    {
        std::cout << " " << array[i];
    }
    std::cout << "\n";
}

template <typename T>
inline void swap(vector<T>& a, vector<T>& b)
{
    std::swap(a.array, b.array);
}

#endif // VECTOR_H
