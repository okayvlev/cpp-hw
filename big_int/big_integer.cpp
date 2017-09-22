#include <iostream>
#include <cstring>
#include <algorithm>
#include "big_integer.h"

big_integer::big_integer() : big_integer { 0 } { }

void big_integer::quick_copy(const big_integer& other)
{
    //std::cout << "quick copy\n";
    if (other.state == SMALL)
    {
        state = SMALL;
        number = other.number;
    }
    else
    {
        state = BIG;
        big_number = other.big_number;
    }
}

big_integer::big_integer(const big_integer& other)
{
    //std::cout << "copy constructor\n";
    try
    {
        quick_copy(other);
    }
    catch (...) { /* reporting error */ };
}

big_integer::big_integer(int a)
{
    //std::cout << "int constructor " << a << "\n";
    try // FIXME understand try policy
    {
        state = SMALL;
        sign = a < 0;
        number = sign ? -a : a;
    }
    catch (...) { /* reporting error */ };
}

big_integer::big_integer(std::string const& str) : big_integer { }
{
    //std::cout << "string constructor\n";
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
        if (sign_)
            tmp = -tmp;
        tmp.trim();
        swap(tmp);
    }
    catch (...) { /* reporting error */ };
}

big_integer::~big_integer()
{
    //std::cout << "destructor\n";
    if (state == BIG)
        big_number.~vector();
}

void big_integer::swap(big_integer& other)
{
    //std::cout << "swap\n";
    // Union must be fully swapped, therefore the largest member of union should be chosen
    if (sizeof(value_type) >= sizeof(vector<value_type>))   // Relying on a comiler to optimize this constexpr at compile time
        std::swap(number, other.number);
    else
        std::swap(big_number, other.big_number);
    std::swap(sign, other.sign);
    std::swap(state, other.state);
}

big_integer& big_integer::operator=(const big_integer& other)
{
    //std::cout << "=\n";
    try
    {
        if (&other != this)
        {
            big_integer tmp { other };
            tmp.swap(*this);
        }
    }
    catch (...) { /* reporting error */ };
    return *this;
}

big_integer& big_integer::operator+=(big_integer const& rhs)
{
    if (sign != rhs.sign)
    {
        return *this -= -rhs;
    }

    vector<value_type> ans;
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
    //std::cout << "-=\n";
    big_integer a { *this };
    big_integer b { rhs };
    a.detach();
    b.detach();

    if (a.sign() != b.sign())
    {
        big_integer tmp { a + (-b) };
        swap(tmp);
        return *this;
    }

    if (a == b)
    {
        big_integer tmp { };
        swap(tmp);
        return *this;
    }

    a.to_big_object();
    b.to_big_object();

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
    if (carry)
    {
        a[a.size() - 1] = BASE - a[a.size() - 1];
    }
    a.convert_to_2s(carry ^ sign_);
    a.trim();
    //a.out();
    swap(a);
    //out();
    //std::cout << "-----------\n";
    return *this;
}

big_integer& big_integer::operator*=(big_integer const& rhs)
{
    big_integer a { *this };
    big_integer b { rhs };
    a.out();
    b.out();
    a.to_big_object();
    b.to_big_object();
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
    //std::cout << "/=\n";
    big_integer a { *this };
    big_integer b { rhs };
    a.detach();
    b.detach();

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

    b.to_big_object();

    bool sign_ { a.convert_to_signed() != b.convert_to_signed() };

    tr_value_type f { BASE / (((b.size() > 1 && b[b.size() - 1] == 0) ?
        b[b.size() - 2] : b[b.size() - 1]) + 1) };
    a.convert_to_2s(0);
    b.convert_to_2s(0);
    if (a < b)
    {
        swap(tmp);
        return *this;
    }
    if (f == BASE)  // will only increase length of every number
        f = 1;
    tmp.to_big_object();
    //a.out();
    //b.out();
    big_integer r { a * f };
    big_integer d { b * f };

    r.to_big_object();
    d.to_big_object();
    r.convert_to_signed();
    d.convert_to_signed();  // sign is always positive
    size_t size_r { (r.size() > 1 && r[r.size() - 1] == 0u) ? r.size() - 1 : r.size() };
    size_t size_d { (d.size() > 1 && d[d.size() - 1] == 0u) ? d.size() - 1 : d.size() };

    //r.out();
    //d.out();
    tmp.reallocate(size_r - size_d + 1);
    big_integer dq { };

    big_integer h { };

    static const big_integer BIG_BASE { big_integer { 1 } << BITS };

    tr_value_type d1 { d[size_d - 1] };

    for (int k = size_r - 1; k > static_cast<int>(size_r - size_d); --k)
    {
        h *= BIG_BASE;
        h += from_value_type(r[k]);
    }

    for (size_t k = size_r - size_d + 1; k--;)
    {
        //std::cout << "----====\n";
        h *= BIG_BASE;
        //h.out();
        h += from_value_type(r[k]);
        //std::cout << k << " " << r[k] << "\n";
        //from_value_type(r[k]).out();
        //std::cout << "h: ";
        //h.out();
        if (h.state == SMALL)
            h.to_big_object();

        size_t size_h { (h.size() > 1 && h[h.size() - 1] == 0u) ? h.size() - 1 : h.size() };
        tr_value_type r2 { h[size_h - 1] }; // FIXME

        if (size_h > size_d)
        {
            r2 *= BASE;
            r2 += h[size_h - 2];
        }

        tr_value_type qt { std::min(r2 / d1, static_cast<tr_value_type>(BASE - 1)) };
        std::cout << "======\n";
        dq = d * qt;
        std::cout << "======\n";
        std::cout << "h: ";
        h.out();
        std::cout << qt << "qt\n";
        std::cout << "dq: ";
        dq.out();
        std::cout << "d: ";
        d.out();

        while (h < dq) {
            qt--;
            dq -= d;
        }
        tmp[k] = qt;
        //std::cout << "H: ";
        //h.out();
        //std::cout << "dq: ";
        //dq.out();
        h -= dq;
        //std::cout << "H': ";
        //h.out();
        //std::cout << "=====---\n";
    }
    tmp.convert_to_2s(sign_);
    tmp.trim();
    swap(tmp);
    return *this;
}

big_integer& big_integer::operator%=(big_integer const& rhs)
{
    return *this -= (*this / rhs) * rhs;
}

big_integer& big_integer::operator&=(big_integer const& rhs)
{
    // big_integer tmp { };
    //
    // if (state == rhs.state && rhs.state == SMALL)
    // {
    //     tmp.number = number & rhs.number;
    //     swap(tmp);
    //     return *this;
    // }
    // big_integer a { *this };
    // big_integer b { rhs };
    // a.detach();
    // b.detach();
    // if (a.state == SMALL)
    //     a.to_big_object();
    // if (b.state == SMALL)
    //     b.to_big_object();
    //
    // tmp.to_big_object();
    // tmp.reallocate(std::max(a.size(), b.size()));
    //
    // const value_type zero_a { a.sign() ? ~0u : 0u };
    // const value_type zero_b { b.sign() ? ~0u : 0u };
    //
    // for (size_t i = 0; i < tmp.size(); ++i)
    // {
    //     value_type x { i < a.size() ? a[i] : zero_a };
    //     value_type y { i < b.size() ? b[i] : zero_b };
    //     tmp[i] = x & y;
    // }
    // tmp.trim();
    // swap(tmp);
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs)
{
    // big_integer tmp { };
    //
    // if (state == rhs.state && rhs.state == SMALL)
    // {
    //     tmp.number = number | rhs.number;
    //     swap(tmp);
    //     return *this;
    // }
    // big_integer a { *this };
    // big_integer b { rhs };
    // a.detach();
    // b.detach();
    // if (a.state == SMALL)
    //     a.to_big_object();
    // if (b.state == SMALL)
    //     b.to_big_object();
    //
    // tmp.to_big_object();
    // tmp.reallocate(std::max(a.size(), b.size()));
    //
    // const value_type zero_a { a.sign() ? ~0u : 0u };
    // const value_type zero_b { b.sign() ? ~0u : 0u };
    //
    // for (size_t i = 0; i < tmp.size(); ++i)
    // {
    //     value_type x { i < a.size() ? a[i] : zero_a };
    //     value_type y { i < b.size() ? b[i] : zero_b };
    //     tmp[i] = x | y;
    // }
    // tmp.trim();
    // swap(tmp);
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs)
{
    // big_integer tmp { };
    //
    // if (state == rhs.state && rhs.state == SMALL)
    // {
    //     tmp.number = number ^ rhs.number;
    //     swap(tmp);
    //     return *this;
    // }
    // big_integer a { *this };
    // big_integer b { rhs };
    // a.detach();
    // b.detach();
    // a.to_big_object();
    // b.to_big_object();
    //
    // tmp.to_big_object();
    // tmp.reallocate(std::max(a.size(), b.size()));
    //
    // const value_type zero_a { a.sign() ? ~0u : 0u };
    // const value_type zero_b { b.sign() ? ~0u : 0u };
    //
    // for (size_t i = 0; i < tmp.size(); ++i)
    // {
    //     value_type x { i < a.size() ? a[i] : zero_a };
    //     value_type y { i < b.size() ? b[i] : zero_b };
    //     tmp[i] = x ^ y;
    // }
    // tmp.trim();
    // swap(tmp);
    return *this;
}
// FIXME add trim everywhere
big_integer& big_integer::operator<<=(int rhs)
{
    // //std::cout << "<<=\n";
    // value_type d { 1 };
    // big_integer tmp { *this };
    // bool sign_ { sign() };
    //
    // while (rhs % BITS != 0)
    // {
    //     d *= 2;
    //     --rhs;
    // }
    //
    // tmp.detach();
    // if (d > 1)
    //     tmp *= from_value_type(d);
    // tmp.to_big_object();
    //
    // int h { rhs / BITS };
    // tmp.reallocate(tmp.size() + h);
    // for (int i = tmp.size() - 1; i >= h; --i)
    // {
    //     tmp[i] = tmp[i - h];
    // }
    // for (int i = 0; i < h; ++i)
    //     tmp[i] = 0u;
    // if (sign() != sign_)
    // {
    //     tmp.reallocate(tmp.size() + 1);
    //     if (!sign_)
    //         tmp.operator[](tmp.size() - 1) = ~0u;
    // }
    // tmp.trim();
    // swap(tmp);
    return *this;
}

big_integer& big_integer::operator>>=(int rhs)
{
    // //std::cout << ">>=\n";
    // value_type d { 1 };
    // big_integer tmp { *this };
    // bool sign_ { sign() };
    // while (rhs % BITS != 0)
    // {
    //     d *= 2;
    //     --rhs;
    // }
    // tmp.detach();
    // tmp /= from_value_type(d);
    // tmp.to_big_object();
    //
    // value_type h { static_cast<value_type>(rhs / BITS) };
    //
    // if (h >= tmp.size())
    // {
    //     big_integer tmp { };
    //     swap(tmp);
    //     return *this;
    // }
    // for (size_t i = 0; i < tmp.size() - h; ++i) // TODO get rid of misuse of value_type
    // {
    //     tmp[i] = tmp[i + h];
    // }
    // tmp.reallocate(tmp.size() - h);
    // tmp.trim();
    // if (sign_)
    //     --tmp;
    // if (sign() != sign_)
    // {
    //     tmp.reallocate(tmp.size() + 1);
    //     if (!sign_)
    //         tmp.operator[](tmp.size() - 1) = ~0u;
    // }
    // swap(tmp);
    return *this;
}

big_integer big_integer::operator+() const
{
    return *this;
}

big_integer big_integer::operator-() const
{
    if (*this == 0)
        return *this;

    big_integer tmp { *this };
    tmp.sign ^= 1;
    return tmp;
}

big_integer big_integer::operator~() const
{
    if (state == SMALL)
        return ~number;
    big_integer tmp { *this };
    tmp.detach();
    tmp.convert_to_2s(sign);
    tmp.reverse_bytes();
    tmp.sign = tmp.convert_to_signed();
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

bool operator==(big_integer const& a, big_integer const& b) // TODO compare pointers
{
    if (a.state != b.state || a.sign != b.sign)
        return false;
    if (a.state == big_integer::SMALL)
        return a.number == b.number;
    if (a.size() != b.size())
        return false;

    for (size_t i = 0; i < a.size(); ++i)
        if (a[i] != b[i])
            return false;
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
    if (!a.sign && b.sign)
        return true;
    if (a.sign && !b.sign)
        return false;
    if (a.sign && b.sign)
        return a < b;
    // Only positive pass
    if (a.state == big_integer::SMALL && b.state == big_integer::SMALL)
        return a.number > b.number;
    if (a.state == big_integer::BIG && b.state == big_integer::SMALL)
        return true;
    if (a.state == big_integer::SMALL && b.state == big_integer::BIG)
        return false;
    // Only big pass
    if (a.size() > b.size())
        return true;
    if (b.size() > a.size())
        return false;
    for (int i = a.size() - 1; i >= 0; --i)
    {
        if (a[i] > b[i])
            return true;
        if (a[i] < b[i])
            return false;
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

    while (tmp != 0)
    {
        //(tmp % 10).out();
        str += static_cast<char>('0' + (tmp % 10).number);
        tmp /= 10;
    }
    if (a.sign)
        str += '-';
    std::reverse(str.begin(), str.end());
    return str;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a)
{
    return s << to_string(a);
}

void big_integer::detach()
{
    //std::cout << "detach\n";
    if (state == SMALL) return;
    big_number.detach();
}

bool big_integer::convert_to_signed()
{
    //std::cout << "convert\n";
    if (!get_sign())
        return false;
    simple_conversion();
    return true;
}

void big_integer::convert_to_2s(bool sign)
{
    //std::cout << "convert to 2s\n";
    assert(state == BIG);
    if (sign)
        simple_conversion();
    if (get_sign())
        ensure_capacity(big_number.size() + 1); // Adding 0 at the beginning to not confuse with negative
}

void big_integer::simple_conversion()
{
    for (size_t i = 0; i < size(); ++i)
        big_number[i] = ~big_number[i];
    operator++();
}

void big_integer::trim()
{
    if (state == BIG)
    {
        big_number.shrink_to_fit();
        if (big_number.size() > 1)
            return;
        big_integer tmp { *this };
        tmp.state = SMALL;
        tmp.number = big_number[0];
        swap(tmp);
    }
    if (number == 0u)
        sign = 0;
}

void big_integer::out() const
{
    std::cout << (state ? "BIG" : "SMALL") << "\n";
    if (state == SMALL)
        std::cout << number << std::endl;
    else
    {
        big_number.out();
    }
}
