#include <iostream>
#include <cstring>
#include "big_integer.h"

big_integer::big_integer() : big_integer { 0 } { }

void big_integer::quick_copy(const big_integer& other)
{
    if (other.state == SMALL)
    {
        state = SMALL;
        number = other.number;
    }
    else
    {
        state = BIG;
        array = other.array;
        other.array[-2]++;  // accessing ref_count
    }
}

big_integer::big_integer(const big_integer& other)
{
    try
    {
        big_integer tmp;
        tmp.quick_copy(other);
        swap(std::move(tmp));
    }
    catch (...) { /* reporting error */ };
}

big_integer::big_integer(int a)
{
    state = SMALL;
    number = a;
}
/*
explicit big_integer(std::string const& str)
{

}
*/
big_integer::~big_integer()
{
    if (state == BIG)
    {
        if (ref_count() != 0)
        {
            detach();
        }
        else
        {
            delete[](array - 2);
        }
    }
}

void big_integer::swap(big_integer&& tmp)   // assuming tmp won't be used anymore
{
    if (tmp.state == BIG)   // if something will happen here, nothing will be spoiled
        std::swap(number, tmp.number);
    else
        std::swap(array, tmp.array);
    std::swap(state, tmp.state);
}

big_integer& big_integer::operator=(const big_integer& other)
{
    try
    {
        big_integer tmp { other };
        swap(std::move(tmp));
    }
    catch (...) { /* reporting error */ };

    return *this;
}

big_integer& big_integer::operator+=(big_integer const& rhs)
{
    if (sign() != rhs.sign())
        return *this -= -rhs;

    big_integer a { *this };
    big_integer b { rhs };
    bool sign_ { a.convert_to_signed() }; // equal signs
    b.convert_to_signed();
    a.reallocate(std::max(a.size(), b.size()) + 1);

    tr_value_type carry = 0;
    for (size_t i = 0; i < a.size(); ++i)
    {
        tr_value_type res { carry + a[i] };
        if (i < b.size())
            res += b[i];
        carry = res / BASE;
        a[i] = res % BASE;
    }
    a.convert_to_2s(sign_);
    a.trim();
    swap(std::move(a));
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs)
{
    if (sign() != rhs.sign())
        return *this += -rhs;

    big_integer a { *this };
    big_integer b { rhs };
    bool sign_ { a.convert_to_signed() }; // equal signs
    b.convert_to_signed();
    a.reallocate(std::max(a.size(), b.size()));

    tr_value_type carry = 0;
    for (size_t i = 0; i < a.size(); ++i)
    {
        tr_value_type res { static_cast<tr_value_type>(BASE) + a[i] - carry };
        if (i < b.size())
            res -= b[i];
        if (res < BASE)
            carry = 1;
        else
            carry = 0;
        a[i] = res % BASE;
    }
    a.convert_to_2s(sign_);
    a.trim();
    swap(std::move(a));
    return *this;
}
// big_integer& operator*=(big_integer const& rhs);
// big_integer& operator/=(big_integer const& rhs);
big_integer& big_integer::operator%=(big_integer const& rhs)
{
    return *this = *this - (*this / rhs) * rhs;
}

// big_integer& operator&=(big_integer const& rhs);
// big_integer& operator|=(big_integer const& rhs);
// big_integer& operator^=(big_integer const& rhs);
//
// big_integer& operator<<=(int rhs);
// big_integer& operator>>=(int rhs);

big_integer big_integer::operator+() const
{
    return *this;
}

big_integer big_integer::operator-() const
{
    return ++~*this;
}

big_integer big_integer::operator~() const
{
    if (state == SMALL) return ~number;
    big_integer tmp { *this };
    tmp.detach();
    for (size_t i = 0; i < size(); ++i)
    {
        tmp[i] = ~tmp[i];
    }
    tmp.trim();
    return tmp;
}

big_integer& big_integer::operator++()
{
    return *this += 1;
}

big_integer big_integer::operator++(int)
{
    big_integer tmp { *this };
    tmp.detach();
    ++*this;
    return tmp;
}

big_integer& big_integer::operator--()
{
    return *this -= 1;
}

big_integer big_integer::operator--(int)
{
    big_integer tmp { *this };
    tmp.detach();
    --*this;
    return tmp;
}

big_integer operator+(big_integer a, big_integer const& b)
{
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b)
{
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b)
{
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b)
{
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b)
{
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const& b)
{
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b)
{
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b)
{
    return a ^= b;
}

big_integer operator<<(big_integer a, int b)
{
    return a <<= b;
}

big_integer operator>>(big_integer a, int b)
{
    return a >>= b;
}

bool operator==(big_integer const& a, big_integer const& b)
{
    if (a.state != b.state) return false;
    if (a.state == big_integer::SMALL) return a.number == b.number;
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i] != b[i]) return false;
    return true;
}

bool operator!=(big_integer const& a, big_integer const& b)
{
    return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b)
{
    return b > a;
}

bool operator>(big_integer const& a, big_integer const& b)
{
    if (a.state == big_integer::BIG && b.state == big_integer::SMALL) return true;
    if (a.state == big_integer::SMALL && b.state == big_integer::BIG) return false;
    if (a.state == big_integer::SMALL) return a.number > b.number;
    if (a.size() > b.size()) return true;
    if (b.size() > a.size()) return false;
    for (int i = a.size() - 1; i >= 0; --i)
    {
        if (a[i] > b[i]) return true;
        if (a[i] < b[i]) return false;
    }
    return false;
}

bool operator<=(big_integer const& a, big_integer const& b)
{
    return !(a > b);
}

bool operator>=(big_integer const& a, big_integer const& b)
{
    return !(a < b);
}

// std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a)
{
    return s << to_string(a);
}

void big_integer::to_big_object()
{
    state = BIG;
    array = new value_type[3] + 2;  // shift for optimizing operator[] call
    size() = 1;
    ref_count() = 0;
    operator[](0) = 0;
}

void big_integer::detach()
{
    if (state == SMALL) return;
    big_integer tmp;
    value_type* old { array };
    tmp.array = new value_type[*(old - 1) + 2] + 2;
    memcpy(tmp.array - 1, old - 1, sizeof(value_type) * (*(old - 1) + 1));
    tmp.ref_count() = 0;
    swap(std::move(tmp));
    --array[-2];    // if no exception
}

bool big_integer::convert_to_signed()
{
    bool sign_ { sign() };
    *this = ++~*this;
    return sign_;
}

void big_integer::convert_to_2s(bool sign)
{
    if (sign) return;
    *this = ++~*this;
}

void big_integer::reallocate(value_type new_size)
{
    if (state == SMALL) return;
    detach();
    new_size += 2;
    value_type* new_arr = new value_type[new_size] { };
    memcpy(new_arr, array - 2, std::min(size() + 2, new_size) * sizeof(value_type));
    delete[](array - 2);
    array = new_arr + 2;
    size() = new_size;
}

void big_integer::trim()
{
    if (state == SMALL) return;
    value_type zero { sign() ? ~0u : 0u };
    size_t size_ { size() };
    while (size_ > 1 && operator[](size_ - 1) == zero)
        --size_;
    if (sign() != (0u == zero)) ++size_;
    if (size_ == 1)
    {
        state = SMALL;
        value_type* old { array };
        number = operator[](0);
        delete[](old - 2);
    }
    else
    {
        reallocate(size_);
    }
}
