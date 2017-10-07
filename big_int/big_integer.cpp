#include <iostream>
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
        big_number = other.big_number;
    }
    sign = other.sign;
}

big_integer::big_integer(const big_integer& other)
{
    quick_copy(other);
}

big_integer::big_integer(int a)
{
    state = SMALL;
    sign = a < 0;
    number = sign ? -static_cast<value_type>(a) : a;
}

big_integer big_integer::from_value_type(value_type t) const
{
    big_integer tmp { };
    tmp.number = t;
    return tmp;
}

big_integer::big_integer(std::string const& str) : big_integer { }
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

big_integer::~big_integer()
{
    if (state == BIG)
        big_number.~vector();
}

void big_integer::assign_vector(vector<value_type>& tmp)
{
    if (state == SMALL)
    {
        big_number = vector<value_type> { number };
        state = BIG;
    }
    ::swap(tmp, big_number);
}

void big_integer::swap(big_integer& other)
{
    // Union must be fully swapped, therefore the largest member of union should be chosen
    if (sizeof(value_type) >= sizeof(vector<value_type>))   // Relying on a compiler to optimize this constexpr at compile time
        std::swap(number, other.number);
    else
        ::swap(big_number, other.big_number);
    std::swap(sign, other.sign);
    std::swap(state, other.state);
}

big_integer& big_integer::operator=(const big_integer& other)
{
    if (&other != this)
    {
        big_integer tmp { other };
        tmp.swap(*this);
    }
    return *this;
}

big_integer& big_integer::operator+=(big_integer const& rhs)
{
    if (rhs == 0)
        return *this;
    if (sign != rhs.sign)
    {
        return *this -= -rhs;
    }
    detach();
    const vector<value_type> a { (state == BIG) ? big_number : number };
    const vector<value_type> b { (rhs.state == BIG) ? rhs.big_number : rhs.number };
    vector<value_type> ans;
    ans.ensure_capacity(std::max(a.size(), b.size()) + 1);

    tr_value_type carry { };
    for (size_t i = 0; i < ans.size(); ++i)
    {
        tr_value_type res { carry + (i < a.size() ? a[i] : 0u) };
        if (i < b.size())
            res += b[i];
        carry = res / BASE;
        ans[i] = res % BASE;
    }
    assign_vector(ans);
    trim();
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs)
{
    if (rhs == 0)
        return *this;
    if (sign != rhs.sign)
    {
        return *this += -rhs;
    }
    detach();
    if (abs_greater(rhs, *this))
    {
        return *this = -(rhs - *this);
    }

    vector<value_type> ans;
    const vector<value_type> a { (state == BIG) ? big_number : number };
    const vector<value_type> b { (rhs.state == BIG) ? rhs.big_number : rhs.number };
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
    assign_vector(ans);
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
        carry = res / BASE;
        tmp[i] = res % BASE;
    }
    assign_vector(tmp);
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
    const vector<value_type> b { (rhs.state == BIG) ? rhs.big_number : rhs.number };
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
    assign_vector(ans);
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
    assign_vector(tmp);
    trim();
}

big_integer& big_integer::operator/=(big_integer const& rhs)
{
    assert(rhs != 0);
    detach();
    sign ^= rhs.sign;
    if (state == SMALL && rhs.state == SMALL)
    {
        number /= rhs.number;
        trim();
        return *this;
    }
    if (state == SMALL && rhs.state == BIG)
    {
        number = 0;
        trim();
        return *this;
    }
    if (abs_greater(rhs, *this))
    {
        big_integer tmp { };
        swap(tmp);
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
    r.sign = 0;
    d.sign = 0;
    vector<value_type> ans;
    ans.ensure_capacity(r.size() - d.size() + 1);
    big_integer dq { };
    big_integer h { };
    tr_value_type d1 { d[d.size() - 1] };

    for (int k = r.size() - 1; k > static_cast<int>(r.size() - d.size()); --k)
    {
        h <<= BITS;
        h += from_value_type(r[k]);
    }
    for (size_t k = r.size() - d.size() + 1; k--;)
    {
        h <<= BITS;
        h += from_value_type(r[k]);

        tr_value_type r2 { h.state == SMALL ? h.number : h[h.size() - 1] };
        if (h.state != SMALL && h.size() > d.size())
        {
            r2 *= BASE;
            r2 += h[h.size() - 2];
        }
        tr_value_type qt { std::min(r2 / d1, BASE - 1) };
        dq = d * from_value_type(qt);
        while (h < dq)
        {
            qt--;
            dq -= d;
        }
        ans[k] = qt;
        h -= dq;
    }
    assign_vector(ans);
    trim();
    return *this;
}

big_integer& big_integer::operator%=(big_integer const& rhs)
{
    return *this -= (*this / rhs) * rhs;
}

void big_integer::ensure_big_object()
{
    if (state == SMALL)
    {
        big_number = vector<value_type> { number };
        state = BIG;
    }
}

void big_integer::perform_bitwise_operation(std::function<void(value_type&, value_type&)> func, const big_integer& rhs)
{
    big_integer a { *this };
    a.detach();
    big_integer b { rhs };
    b.detach();
    a.ensure_big_object();
    b.ensure_big_object();
    if (a.sign)
        a.simple_conversion();
    if (b.sign)
        b.simple_conversion();
    a.ensure_big_object();
    b.ensure_big_object();
    if (a.size() < b.size())
    {
        value_type zero { a.get_sign() ? ~0u : 0u };
        a.big_number.ensure_capacity(b.size());
        if (zero > 0u)
            for (int i = a.size() - 1; i >= 0; --i)
            {
                if (a[i] == 0u)
                    a[i] = zero;
                else
                    break;
            }
    }
    if (a.size() > b.size())
    {
        value_type zero { b.get_sign() ? ~0u : 0u };
        b.big_number.ensure_capacity(a.size());
        if (zero > 0u)
            for (int i = b.size() - 1; i >= 0; --i)
            {
                if (b[i] == 0u)
                    b[i] = zero;
                else
                    break;
            }
    }
    for (size_t i = 0; i < a.size(); ++i)
    {
        func(a[i], b[i]);
    }

    a.sign = a.get_sign();
    if (a.get_sign())
        a.simple_conversion();
    a.trim();
    swap(a);
}

big_integer& big_integer::operator&=(big_integer const& rhs)
{
    perform_bitwise_operation([&](value_type& a, value_type& b) { a &= b; }, rhs);
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs)
{
    perform_bitwise_operation([&](value_type& a, value_type& b) { a |= b; }, rhs);
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs)
{
    perform_bitwise_operation([&](value_type& a, value_type& b) { a ^= b; }, rhs);
    return *this;
}

big_integer& big_integer::operator<<=(int rhs)
{
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
    tmp.detach();
    int h { rhs / BITS };
    tmp.ensure_capacity(tmp.size() + h);
    for (int i = tmp.size() - 1; i >= h; --i)
    {
        tmp[i] = tmp[i - h];
    }
    for (int i = 0; i < h; ++i)
        tmp[i] = 0u;
    assign_vector(tmp);
    trim();
    return *this;
}

big_integer& big_integer::operator>>=(int rhs)
{
    if (rhs == 0)
        return *this;
    detach();
    value_type d { 1 };
    while (rhs % BITS != 0)
    {
        d *= 2;
        --rhs;
    }
    if (d > 1)
        quotient(d);
    vector<value_type> tmp { (state == BIG) ? big_number : number };
    tmp.detach();
    int h { rhs / BITS };
    if (static_cast<size_t>(h) >= tmp.size())
    {
        big_integer tmp { };
        swap(tmp);
        return *this;
    }
    if (h > 0)
    {
        for (size_t i = 0; i < tmp.size() - h; ++i)
        {
            tmp[i] = tmp[i + h];
            tmp[i + h] = 0u;
        }
    }
    assign_vector(tmp);
    trim();
    if (sign)
    operator--();
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
        return sign ? ~-number : ~number;
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

bool abs_greater(big_integer const& a, big_integer const& b)
{
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

bool operator>(big_integer const& a, big_integer const& b)
{
    if (!a.sign && b.sign)
        return true;
    if (a.sign && !b.sign)
        return false;
    if (a.sign && b.sign)
        return abs_greater(b, a);
    // Only positive pass
    return abs_greater(a, b);
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
    tmp.detach();
    std::string str { };
    while (tmp != 0)
    {
        str += static_cast<char>('0' + (tmp % 10).number);
        tmp /= 10;
    }
    if (str.size() == 0)
        str = "0";
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
    if (state == SMALL) return;
    big_number.detach();
}

bool big_integer::convert_to_signed()
{
    if (!get_sign())
        return false;
    simple_conversion();
    return true;
}

void big_integer::convert_to_2s(bool sign)
{
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
    bool sign_ { sign };
    sign = 0;
    operator++();
    sign = sign_;
}

void big_integer::trim()
{
    if (state == BIG)
    {
        big_number.shrink_to_fit();
        if (size() > 1)
            return;
        big_integer tmp { };
        tmp.sign = sign;
        tmp.number = big_number[0];
        swap(tmp);
    }
    if (number == 0u)
        sign = 0;
}

void big_integer::out() const
{
    std::cout << (state ? "BIG" : "SMALL") << "\n";
    if (sign)
        std::cout << "-";
    if (state == SMALL)
        std::cout << number << std::endl;
    else
    {
        big_number.out();
    }
}
