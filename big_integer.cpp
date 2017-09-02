#include <iostream>
#include <cstring>
#include <algorithm>
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
        swap(tmp);
    }
    catch (...) { /* reporting error */ };
}

big_integer::big_integer(int a)
{
    state = SMALL;
    number = a;
}

big_integer::big_integer(std::string const& str)
{
    try
    {
        bool sign_ { str.size() > 0 && str[0] == '-' };
        big_integer tmp { };

        for (char c : str)
        {
            if (isdigit(c))
            {
                tmp *= 10;
                tmp += c - '0';
            }
        }
        if (sign_) tmp = -tmp;
        swap(tmp);
    }
    catch (...) { /* reporting error */ };
}

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

void big_integer::swap(big_integer& tmp)   // assuming tmp won't be used anymore
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
        swap(tmp);
    }
    catch (...) { /* reporting error */ };

    return *this;
}

big_integer& big_integer::operator+=(big_integer const& rhs)
{
    big_integer a { *this };
    big_integer b { rhs };

    if (a.state == SMALL) to_big_object();
    if (b.state == SMALL) to_big_object();
    a.detach();
    b.detach();
    if (a.sign() != b.sign())
        return a -= -b;

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
    swap(a);
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs)
{
    big_integer a { *this };
    big_integer b { rhs };

    if (a.state == SMALL) to_big_object();
    if (b.state == SMALL) to_big_object();
    a.detach();
    b.detach();
    if (a.sign() != b.sign())
        return a += -b;

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
    swap(a);
    return *this;
}

big_integer& big_integer::operator*=(big_integer const& rhs)
{
    big_integer a { *this };
    big_integer b { rhs };

    if (a.state == SMALL) to_big_object();
    if (b.state == SMALL) to_big_object();
    a.detach();
    b.detach();

    bool sign_ { a.convert_to_signed() != b.convert_to_signed() };
    big_integer tmp { };
    tmp.to_big_object();
    tmp.reallocate(a.size() + b.size());

    for (size_t i = 0; i < a.size(); ++i) {
        tr_value_type carry { };
        for (size_t j = 0; j < b.size(); ++j) {
            tr_value_type res { static_cast<tr_value_type>(a[i]) * b[j] + carry };
            carry = res / BASE;
            if (static_cast<tr_value_type>(tmp[i + j]) + res % BASE >= BASE)
                tmp[i + j + 1] += 1;
            tmp[i + j] += res % BASE;
        }
        tmp[i + b.size()] += carry;
    }
    tmp.convert_to_2s(sign_);
    tmp.trim();
    swap(tmp);
    return *this;
}

big_integer& big_integer::operator/=(big_integer const& rhs)
{
    big_integer a { *this };
    big_integer b { rhs };
    big_integer tmp { };

    if (a.state == SMALL && b.state == SMALL)
    {
        tmp.number = a.number / b.number;
        swap(tmp);
        return *this;
    }

    if (a.state == SMALL && b.state == BIG)
    {
        swap(tmp);
        return *this;
    }

    if (b.state == SMALL) to_big_object();
    a.detach();
    b.detach();

    bool sign_ { a.convert_to_signed() != b.convert_to_signed() };

    tr_value_type f { BASE / (b[b.size() - 1] + 1) };

    a.convert_to_2s(0);
    b.convert_to_2s(0);
    if (a < b)
    {
        swap(tmp);
        return *this;
    }

    tmp.to_big_object();

    big_integer r { a * f };
    big_integer d { b * f };
    r.convert_to_signed();
    d.convert_to_signed();  // sign is always positive

    tmp.reallocate(r.size() - d.size() + 1);
    big_integer dq { };

    big_integer h { };

    for (int k = r.size() - 1; k > r.size() - d.size(); k--)
    {
        h *= BASE;
        h += r[k];
    }

    for (size_t k = r.size() - d.size() + 1; k--;)
    {
        h *= BASE;
        h += r[k];

        tr_value_type r2 { h[h.size() - 1] };
        if (h.size() > d.size())
        {
            r2 *= BASE;
            r2 += h[h.size() - 2];
        }

        tr_value_type d1 { d[d.size() - 1] };
        tr_value_type qt { std::min(r2 / d1, static_cast<tr_value_type>(BASE - 1)) };

        dq = d * qt;
        while (h < dq) {
            qt--;
            dq -= d;
        }
        tmp[k] = qt;
        h -= dq;
    }
    tmp.convert_to_2s(sign_);
    tmp.trim();
    swap(tmp);
    return *this;
}

big_integer& big_integer::operator%=(big_integer const& rhs)
{
    return *this = *this - (*this / rhs) * rhs;
}

big_integer& big_integer::operator&=(big_integer const& rhs)
{
    big_integer tmp { };

    if (state == rhs.state && rhs.state == SMALL)
    {
        tmp.number = number & rhs.number;
        swap(tmp);
        return *this;
    }
    big_integer a { *this };
    big_integer b { rhs };
    a.detach();
    b.detach();
    if (a.state == SMALL)
        a.to_big_object();
    if (b.state == SMALL)
        b.to_big_object();

    tmp.to_big_object();
    tmp.reallocate(std::max(a.size(), b.size()));

    const value_type zero_a { a.sign() ? ~0u : 0u };
    const value_type zero_b { b.sign() ? ~0u : 0u };

    for (size_t i = 0; i < tmp.size(); ++i)
    {
        value_type x { i < a.size() ? a[i] : zero_a };
        value_type y { i < b.size() ? b[i] : zero_b };
        tmp[i] = x & y;
    }

    swap(tmp);
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs)
{
    big_integer tmp { };

    if (state == rhs.state && rhs.state == SMALL)
    {
        tmp.number = number | rhs.number;
        swap(tmp);
        return *this;
    }
    big_integer a { *this };
    big_integer b { rhs };
    a.detach();
    b.detach();
    if (a.state == SMALL)
        a.to_big_object();
    if (b.state == SMALL)
        b.to_big_object();

    tmp.to_big_object();
    tmp.reallocate(std::max(a.size(), b.size()));

    const value_type zero_a { a.sign() ? ~0u : 0u };
    const value_type zero_b { b.sign() ? ~0u : 0u };

    for (size_t i = 0; i < tmp.size(); ++i)
    {
        value_type x { i < a.size() ? a[i] : zero_a };
        value_type y { i < b.size() ? b[i] : zero_b };
        tmp[i] = x | y;
    }

    swap(tmp);
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs)
{
    big_integer tmp { };

    if (state == rhs.state && rhs.state == SMALL)
    {
        tmp.number = number ^ rhs.number;
        swap(tmp);
        return *this;
    }
    big_integer a { *this };
    big_integer b { rhs };
    a.detach();
    b.detach();
    if (a.state == SMALL)
        a.to_big_object();
    if (b.state == SMALL)
        b.to_big_object();

    tmp.to_big_object();
    tmp.reallocate(std::max(a.size(), b.size()));

    const value_type zero_a { a.sign() ? ~0u : 0u };
    const value_type zero_b { b.sign() ? ~0u : 0u };

    for (size_t i = 0; i < tmp.size(); ++i)
    {
        value_type x { i < a.size() ? a[i] : zero_a };
        value_type y { i < b.size() ? b[i] : zero_b };
        tmp[i] = x ^ y;
    }

    swap(tmp);
    return *this;
}

big_integer& operator<<=(int rhs)
{
    
}
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
    if (a.state == big_integer::SMALL && b.state == big_integer::SMALL) return a.number > b.number;
    big_integer x { a };
    big_integer y { b };
    x.detach();
    y.detach();
    if (x.state == big_integer::SMALL) x.to_big_object();
    if (y.state == big_integer::SMALL) y.to_big_object();
    bool sign_a { x.convert_to_signed() };
    bool sign_b { y.convert_to_signed() };

    if (!sign_a && sign_b) return true;
    if (sign_a && !sign_b) return false;
    if (sign_a && sign_b)
    {
        x.convert_to_2s(0);
        y.convert_to_2s(0);
        return x < y;
    }

    if (x.size() > y.size()) return true;
    if (y.size() > x.size()) return false;
    for (int i = x.size() - 1; i >= 0; --i)
    {
        if (x[i] > y[i]) return true;
        if (x[i] < y[i]) return false;
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

std::string to_string(big_integer const& a)
{
    big_integer tmp { a };
    std::string str { };
    tmp.detach();
    bool sign_ = tmp.convert_to_signed();

    while (tmp > 0)
    {
        str += static_cast<char>('0' + (tmp % 10).number);
        tmp /= 10;
    }
    if (sign_)
        str += '-';
    std::reverse(str.begin(), str.end());

    return str;
}

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
    if (state == SMALL || ref_count() == 0) return;
    big_integer tmp;
    value_type* old { array };
    tmp.array = new value_type[*(old - 1) + 2] + 2;
    memcpy(tmp.array - 1, old - 1, sizeof(value_type) * (*(old - 1) + 1));
    tmp.ref_count() = 0;
    swap(tmp);
    --array[-2];    // if no exception
}

bool big_integer::convert_to_signed()
{
    detach();
    bool sign_ { sign() };
    *this = ++~*this;
    return sign_;
}

void big_integer::convert_to_2s(bool sign)
{
    detach();
    if (!sign) return;
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
    detach();
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
