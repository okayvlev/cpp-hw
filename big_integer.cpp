#include <iostream>
#include <cstring>
#include "big_integer.h"

big_integer::big_integer() : big_integer { 0 } { }

void big_integer::quick_copy(big_integer& other)
{
    state = BIG;
    if (other.state == SMALL) other.to_big_object();
    other.ref_count()++;
    array = other.array;
}

big_integer::big_integer(big_integer& other)
{
    quick_copy(other);
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

big_integer& big_integer::operator=(big_integer& other)
{
    quick_copy(other);
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
    ref_count()--;
    value_type* old { array };
    array = new value_type[*(old - 1) + 2] + 2;
    memcpy(array - 1, old - 1, *(old - 1) + 1);
    ref_count() = 0;
}
