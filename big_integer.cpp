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

// big_integer& operator+=(big_integer const& rhs);
// big_integer& operator-=(big_integer const& rhs);
// big_integer& operator*=(big_integer const& rhs);
// big_integer& operator/=(big_integer const& rhs);
// big_integer& operator%=(big_integer const& rhs);
//
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

// bool operator==(big_integer const& a, big_integer const& b);
// bool operator!=(big_integer const& a, big_integer const& b);
// bool operator<(big_integer const& a, big_integer const& b);
// bool operator>(big_integer const& a, big_integer const& b);
// bool operator<=(big_integer const& a, big_integer const& b);
// bool operator>=(big_integer const& a, big_integer const& b);
//
// big_integer operator+(big_integer a, big_integer const& b);
// big_integer operator-(big_integer a, big_integer const& b);
// big_integer operator*(big_integer a, big_integer const& b);
// big_integer operator/(big_integer a, big_integer const& b);
// big_integer operator%(big_integer a, big_integer const& b);
//
// big_integer operator&(big_integer a, big_integer const& b);
// big_integer operator|(big_integer a, big_integer const& b);
// big_integer operator^(big_integer a, big_integer const& b);
//
// big_integer operator<<(big_integer a, int b);
// big_integer operator>>(big_integer a, int b);
//
// bool operator==(big_integer const& a, big_integer const& b);
// bool operator!=(big_integer const& a, big_integer const& b);
// bool operator<(big_integer const& a, big_integer const& b);
// bool operator>(big_integer const& a, big_integer const& b);
// bool operator<=(big_integer const& a, big_integer const& b);
// bool operator>=(big_integer const& a, big_integer const& b);
//
// std::string to_string(big_integer const& a);
// std::ostream& operator<<(std::ostream& s, big_integer const& a);

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
    memcpy(tmp.array - 1, old - 1, *(old - 1) + 1);
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

void big_integer::trim()
{
    if (state == SMALL) return;
    value_type zero { sign() ? ~0u : 0u };

    while (size() > 1 && operator[](size() - 1) == zero)
        --size();
    if (sign() != (0u == zero)) ++size();
    if (size() == 1)
    {
        state = SMALL;
        number = operator[](0);
    }
}
