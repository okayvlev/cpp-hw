#include <iostream>
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
    quick_copy(other);
}

big_integer::big_integer(int a)
{
    //std::cout << "int constructor " << a << "\n";
    state = SMALL;
    sign = a < 0;
    number = sign ? -a : a;
}

big_integer big_integer::from_value_type(value_type t) const
{
    big_integer tmp { };
    tmp.number = t;
    return tmp;
}

big_integer::big_integer(std::string const& str) : big_integer { }
{
    //std::cout << "string constructor\n";
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
        ::swap(big_number, other.big_number);
    std::swap(sign, other.sign);
    std::swap(state, other.state);
}

big_integer& big_integer::operator=(const big_integer& other)
{
    //std::cout << "=\n";
    if (&other != this)
    {
        big_integer tmp { other };
        tmp.swap(*this);
    }
    return *this;
}

big_integer& big_integer::operator+=(big_integer const& rhs)
{
    if (sign != rhs.sign)
    {
        return *this -= -rhs;
    }
    detach();
    vector<value_type> ans;
    const vector<value_type> a { (state == BIG) ? big_number : number };
    const vector<value_type> b { (state == BIG) ? rhs.big_number : rhs.number };
    ans.ensure_capacity(std::max(a.size(), b.size()) + 1);

    tr_value_type carry { };
    for (size_t i = 0; i < ans.size(); ++i)
    {
        tr_value_type res { carry + (i < a.size()) ? a[i] : 0u };
        if (i < b.size())
            res += b[i];
        carry = res / BASE;
        ans[i] = res % BASE;
    }
    ans.out();
    big_number.out();
    ::swap(ans, big_number);
    std::cout << "okk\n";
    ans.out();
    big_number.out();
    trim();
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs)
{
    if (sign != rhs.sign)
    {
        return *this += -rhs;
    }
    detach();
    if (*this < rhs)
    {
        return *this = -(rhs - *this);
    }

    vector<value_type> ans;
    const vector<value_type> a { (state == BIG) ? big_number : number };
    const vector<value_type> b { (state == BIG) ? rhs.big_number : rhs.number };
    ans.ensure_capacity(std::max(a.size(), b.size()));

    tr_value_type carry = 0;
    for (size_t i = 0; i < ans.size(); ++i)
    {
        tr_value_type res { BASE + a[i] - carry };
        if (i < b.size())
            res -= b[i];
        if (res < BASE)
            carry = 1;
        else
            carry = 0;
        ans[i] = res % BASE;
    }
    ::swap(ans, big_number);
    trim();
    return *this;
}

void big_integer::multiply(const value_type& rhs)
{
    vector<value_type> tmp { (state == BIG) ? big_number : number };
    tmp.detach();
    tmp.ensure_capacity(tmp.size() + 1);
    value_type carry { };
    for (size_t i = 0; i < tmp.size(); ++i)
    {
        tr_value_type res { static_cast<tr_value_type>(rhs) * tmp[i] + carry };
        tmp[i] = res / BASE;
        carry = res % BASE;
    }
    ::swap(tmp, big_number);
    trim();
}

big_integer& big_integer::operator*=(big_integer const& rhs)
{
    detach();
    sign ^= rhs.sign;
    if (rhs.state == SMALL)
    {
        multiply(rhs.number);
        return *this;
    }
    vector<value_type> ans;
    const vector<value_type> a { (state == BIG) ? big_number : number };
    const vector<value_type> b { (state == BIG) ? rhs.big_number : rhs.number };
    ans.ensure_capacity(a.size() + b.size());
    for (size_t i = 0; i < a.size(); ++i) {
        tr_value_type carry { };
        for (size_t j = 0; j < b.size(); ++j) {
            tr_value_type res { static_cast<tr_value_type>(a[i]) * b[j] + carry };
            carry = res / BASE;
            if (static_cast<tr_value_type>(ans[i + j]) + res % BASE >= BASE)
                ans[i + j + 1] += 1;
            ans[i + j] += res % BASE;
        }
        ans[i + rhs.size()] += carry;
    }
    ::swap(ans, big_number);
    trim();
    return *this;
}

void big_integer::quotient(const value_type& rhs)
{
    vector<value_type> tmp { (state == BIG) ? big_number : number };
    tmp.detach();
    tr_value_type carry { };
    for (size_t i = tmp.size(); i-- > 0;)
    {
        tr_value_type res { carry * BASE + tmp[i] };
        tmp[i] = res / rhs;
        carry = res % rhs;
    }
    ::swap(big_number, tmp);
    trim();
}

big_integer& big_integer::operator/=(big_integer const& rhs)
{
    assert(rhs == 0);
    detach();
    sign ^= rhs.sign;
    if (state == SMALL && rhs.state == SMALL)
    {
        number /= rhs.number;
        trim();
        return *this;
    }
    if ((state == SMALL && rhs.state == BIG) || *this < rhs)
    {
        number = 0;
        trim();
        return *this;
    }
    if (rhs.state == SMALL)
    {
        quotient(rhs.number);
        return *this;
    }

    tr_value_type f { BASE / (rhs[rhs.size() - 1] + 1) };
    if (f == BASE)  // will only increase the length of every number
        f = 1;

    big_integer r { *this * f };
    big_integer d { rhs * f };
    vector<value_type> ans;
    ans.ensure_capacity(r.size() - d.size() + 1);
    big_integer dq { };
    big_integer h { };
    tr_value_type d1 { d[d.size() - 1] };

    static const big_integer BIG_BASE { big_integer { 1 } << BITS };

    for (int k = r.size() - 1; k > static_cast<int>(r.size() - d.size()); --k)
    {
        h *= BIG_BASE;
        h += from_value_type(r[k]);
    }

    for (size_t k = r.size() - d.size() + 1; k--;)
    {
        h *= BIG_BASE;
        h += from_value_type(r[k]);
        tr_value_type r2 { h.state == SMALL ? h.number : h[h.size() - 1] };
        if (h.state != SMALL && h.size() > d.size())
        {
            r2 *= BASE;
            r2 += h[h.size() - 2];
        }
        tr_value_type qt { std::min(r2 / d1, BASE - 1) };
        dq = d * qt;
        while (h < dq)
        {
            qt--;
            dq -= d;
        }
        ans[k] = qt;
        h -= dq;
    }
    ::swap(ans, big_number);
    trim();
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

big_integer& big_integer::operator<<=(int rhs)
{
    //std::cout << "<<=\n";
    detach();
    value_type d { 1 };
    while (rhs % BITS != 0)
    {
        d *= 2;
        --rhs;
    }
    if (d > 1)
        multiply(d);
    vector<value_type> tmp { (state == BIG) ? big_number : number };
    int h { rhs / BITS };
    tmp.ensure_capacity(tmp.size() + h);
    for (int i = tmp.size() - 1; i >= h; --i)
    {
        tmp[i] = tmp[i - h];
    }
    for (int i = 0; i < h; ++i)
        tmp[i] = 0u;
    ::swap(tmp, big_number);
    trim();
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
    // for (size_t i = 0; i < tmp.size() - h; ++i)
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
        return ~number; // TODO is it okay?
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
        big_number.ensure_capacity(size() + 1); // Adding 0 at the beginning to not confuse with negative
}

void big_integer::reverse_bytes()
{
    for (size_t i = 0; i < size(); ++i)
        big_number[i] = ~big_number[i];
}

void big_integer::simple_conversion()
{
    reverse_bytes();
    operator++();
}

void big_integer::trim()
{
    if (state == BIG)
    {
        big_number.shrink_to_fit();
        if (size() > 1)
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
