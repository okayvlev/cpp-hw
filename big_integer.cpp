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
// explicit big_integer(std::string const& str);
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
//
// big_integer operator+() const;
// big_integer operator-() const;
// big_integer operator~() const;
//
// big_integer& operator++();
// big_integer operator++(int);
//
// big_integer& operator--();
// big_integer operator--(int);
//
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
    bool sign { array[size() - 1] >> (BITS - 1) };
    return sign;
}

void big_integer::convert_to_2s(bool sign)
{
    if (sign) return;
    operator-();
}
