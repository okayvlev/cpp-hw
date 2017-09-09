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
        representation.number = other.representation.number;
    }
    else
    {
        state = BIG;
        representation.array = other.representation.array;
        ++other.representation.array[-2];  // accessing ref_count
    }
}

big_integer::big_integer(const big_integer& other) : big_integer { }
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
    //std::cout << "int constructor\n";
    try // FIXME understand try policy
    {
        state = SMALL;
        representation.number = a;
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
    {
        //std::cout << representation.array << std::endl;
        if (ref_count() != 0)
        {
            detach();
        }
        //std::cout << "deleting at " << representation.array << "\n";
        delete[](representation.array - 2);
    }
}

void big_integer::swap(big_integer& tmp)
{
    //std::cout << "swap\n";
    if (state == BIG && tmp.state == BIG)
    {
        std::swap(representation.array, tmp.representation.array);
    }
    else if (state == SMALL && tmp.state == SMALL)
    {
        std::swap(representation.number, tmp.representation.number);
    }
    else if (state == SMALL && tmp.state == BIG)
    {
        value_type* ptr { tmp.representation.array };
        tmp.representation.number = representation.number;
        representation.array = ptr;
    }
    else if (state == BIG && tmp.state == SMALL)
    {
        value_type* ptr { representation.array };
        representation.number = tmp.representation.number;
        tmp.representation.array = ptr;
    }
    //std::swap(representation, tmp.representation); TODO
    std::swap(state, tmp.state);
}

big_integer& big_integer::operator=(const big_integer& other)
{
    //std::cout << "=\n";
    try
    {
        big_integer tmp { };
        tmp.swap(*this);
        quick_copy(other);
    }
    catch (...) { /* reporting error */ };
    return *this;
}

big_integer& big_integer::operator+=(big_integer const& rhs)
{
    std::cout << "+=\n";
    big_integer a { *this };
    big_integer b { rhs };
    a.detach();
    b.detach();

    if (a.sign() != b.sign())
        return a -= -b;

    a.to_big_object();
    b.to_big_object();

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

    if (a.state == SMALL)
        a.to_big_object();
    if (b.state == SMALL)
        b.to_big_object();

    if (a.sign() != b.sign())
        return a += -b;

    a.detach();
    b.detach();

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

    if (a.state == SMALL)
        a.to_big_object();
    if (b.state == SMALL)
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
            std::cout << BASE << "\n";
            carry = res / BASE;
            if (static_cast<tr_value_type>(tmp[i + j]) + res % BASE >= BASE)
                tmp[i + j + 1] += 1;
            tmp[i + j] += res % BASE;
        }
        tmp[i + b.size()] += carry;
    }
    tmp.out();
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
        tmp.representation.number = a.representation.number / b.representation.number;
        swap(tmp);
        return *this;
    }

    if (a.state == SMALL && b.state == BIG)
    {
        swap(tmp);
        return *this;
    }

    if (b.state == SMALL)
        b.to_big_object();
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

    static const big_integer BIG_BASE { big_integer { 1 } << BITS };

    for (int k = r.size() - 1; k > static_cast<int>(r.size() - d.size()); --k)
    {
        h *= BIG_BASE;
        h += r[k];
    }

    for (size_t k = r.size() - d.size() + 1; k--;)
    {
        h *= BIG_BASE;
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
        tmp.representation.number = representation.number & rhs.representation.number;
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
    tmp.trim();
    swap(tmp);
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs)
{
    big_integer tmp { };

    if (state == rhs.state && rhs.state == SMALL)
    {
        tmp.representation.number = representation.number | rhs.representation.number;
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
    tmp.trim();
    swap(tmp);
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs)
{
    big_integer tmp { };

    if (state == rhs.state && rhs.state == SMALL)
    {
        tmp.representation.number = representation.number ^ rhs.representation.number;
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
    tmp.trim();
    swap(tmp);
    return *this;
}
// FIXME add trim everywhere
big_integer& big_integer::operator<<=(int rhs)
{
    value_type d { 1 };
    big_integer tmp { *this };

    while (rhs % BITS != 0)
    {
        d *= 2;
        --rhs;
    }
    tmp.detach();
    if (tmp.state == SMALL)
        tmp.to_big_object();
    tmp *= d;

    int h { rhs / BITS };

    tmp.reallocate(tmp.size() + h);
    for (int i = tmp.size() - 1; i >= h; --i)
    {
        tmp[i] = tmp[i - h];
    }
    for (int i = 0; i < h; ++i)
        tmp[i] = 0u;
    tmp.trim();
    swap(tmp);
    return *this;
}

big_integer& big_integer::operator>>=(int rhs)
{
    value_type d { 1 };
    big_integer tmp { *this };

    while (rhs % BITS != 0)
    {
        d *= 2;
        --rhs;
    }
    tmp.detach();
    if (tmp.state == SMALL)
        tmp.to_big_object();
    tmp /= d;

    value_type h { static_cast<value_type>(rhs / BITS) };

    if (h >= tmp.size())
    {
        big_integer tmp { };
        swap(tmp);
        return *this;
    }
    for (size_t i = 0; i < tmp.size() - h; ++i) // TODO get rid of misuse of value_type
    {
        tmp[i] = tmp[i + h];
    }
    tmp.reallocate(tmp.size() - h);
    tmp.trim();
    swap(tmp);
    return *this;
}

big_integer big_integer::operator+() const
{
    return *this;
}

big_integer big_integer::operator-() const
{
    return ++~*this;
}

void big_integer::reverse_bytes()
{
    for (size_t i = 0; i < size(); ++i)
        representation.array[i] = ~representation.array[i];
}

big_integer big_integer::operator~() const
{
    if (state == SMALL) return ~representation.number;
    big_integer tmp { *this };
    tmp.detach();
    tmp.reverse_bytes();
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
    if (a.state != b.state)
        return false;
    if (a.state == big_integer::SMALL)
        return a.representation.number == b.representation.number;
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
    if (a.state == big_integer::SMALL && b.state == big_integer::SMALL) return a.representation.number > b.representation.number;
    big_integer x { a };
    big_integer y { b };
    x.detach();
    y.detach();
    if (x.state == big_integer::SMALL)
        x.to_big_object();
    if (y.state == big_integer::SMALL)
        y.to_big_object();
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
        str += static_cast<char>('0' + (tmp % 10).representation.number);
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
    if (state == BIG) return;
    int number_ { representation.number };
    state = BIG;
    representation.array = new value_type[3] + 2;  // shift for optimizing operator[] call
    //std::cout << "to big:" << representation.array << std::endl;
    size() = 1;
    ref_count() = 0;
    operator[](0) = abs(number_);
    convert_to_2s(number_ < 0);
}

void big_integer::detach()
{
    //std::cout << "detach\n";
    if (state == SMALL || ref_count() == 0) return;
    --ref_count();
    value_type* new_array { new value_type[size() + 2] + 2 };
    //std::cout << "detach:" << new_array << " from:" << representation.array << "\n";
    memcpy(new_array - 1, representation.array - 1, sizeof(value_type) * (size() + 1));
    std::swap(new_array, representation.array);
    ref_count() = 0;
    //delete[](new_array - 2);
}

bool big_integer::convert_to_signed()
{
    //std::cout << "convert\n";
    if (!sign())
        return 0;
    big_integer a { *this };
    a.detach();
    bool sign_ { a.sign() };
    a.reverse_bytes();
    bool carry { ++a[0] == 0u };
    for (size_t i = 1; i < a.size(); ++i)
    {
        if (!carry)
            break;
        if (++a[i] != 0u) // no overflow
            carry = 0;
    }
    if (carry)
    {
        a.reallocate(a.size() + 1);
        a[a.size() - 1] = 1;
    }

    swap(a);
    return sign_;
}

void big_integer::convert_to_2s(bool sign)
{
    //std::cout << "convert to 2s\n";
    if (!sign) return;
    detach();
    reverse_bytes();
    bool carry { ++operator[](0) == 0u };
    for (size_t i = 1; i < size(); ++i)
    {
        if (!carry)
            break;
        if (++operator[](i) != 0u) // no overflow
            carry = 0;
    }
    if (carry)
    {
        reallocate(size() + 1);
        operator[](size() - 1) = 1;
    }
}

void big_integer::reallocate(value_type new_size)
{
    if (state == SMALL || size() == new_size) return;
    detach();
    new_size += 2;
    value_type* new_arr = new value_type[new_size] { };
    //std::cout << "reallocate:" << new_arr + 2 << " from:" << representation.array << std::endl;
    memcpy(new_arr, representation.array - 2, std::min(size() + 2, new_size) * sizeof(value_type));
    //std::cout << "deleting at " << representation.array << "\n";
    delete[](representation.array - 2);
    representation.array = new_arr + 2;
    size() = new_size - 2;
}

void big_integer::trim()
{
    if (state == SMALL) return;
    detach();
    value_type zero { sign() ? ~0u : 0u };

    size_t size_ { size() };
    while (size_ > 1 && operator[](size_ - 1) == zero)
        --size_;

    if ((representation.array[size_ - 1] >> (BITS - 1)) != (~0u == zero)) ++size_;
    if (size_ == 1)
    {
        state = SMALL;
        value_type* old { representation.array };
        representation.number = operator[](0);
        //std::cout << "deleting at " << old << "\n";
        delete[](old - 2);
    }
    else
    {
        reallocate(size_);
    }
}

void big_integer::out()
{
    std::cout << (state ? "BIG" : "SMALL") << "\n";
    if (state == SMALL)
        std::cout << representation.number << std::endl;
    else
    {
        std::cout << ref_count() << "_" << size() << ": ";
        for (size_t i = 0; i < size(); ++i)
        {
            std::cout << representation.array[i] << " ";
        }
        std::cout << std::endl;
    }
}

void big_integer::test()
{
}
