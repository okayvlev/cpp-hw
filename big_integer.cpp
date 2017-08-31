#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <cstring>
#include "big_int/big_integer.h"
#include <cstring>
#include <stdexcept>

const big_integer ZERO(0);
const big_integer ONE(1);
const big_integer TEN(10);
const big_integer big_base("4294967296");

big_integer::~big_integer() {
    digits.resize(0);
    reset();
}

big_integer::big_integer() {
    reset();
    reallocate(1);
    trim();
}

big_integer::big_integer(const big_integer &other) {
    reset();
    copy(other);
    trim();
}

big_integer::big_integer(int a) {
    reset();
    reallocate(1);
    is_negative = a < 0;
    if (is_negative) {
        digits[0] = (uint32_t) -static_cast<uint32_t>(a);
    } else {
        digits[0] = (uint32_t) a;
    }
    trim();
}

big_integer::big_integer(uint32_t a) {
    reset();
    reallocate(1);
    digits[0] = (uint32_t) a;
}

big_integer::big_integer(uint64_t a) {
    reset();
    reallocate(2);
    digits[1] = (uint32_t) (a / base);
    digits[0] = (uint32_t) (a % base);
    trim();
}


big_integer::big_integer(const std::string &str) {
    big_integer pow = ONE;
    big_integer result = ZERO;

    size_t begin = 0;
    if (str[0] == '-') {
        begin = 1;
    }
    for (size_t i = 0; i < str.length() - begin; ++i) {
        result += (str[str.length() - i - 1] - '0') * pow;
        pow *= TEN;
    }
    if (str[0] == '-') {
        result.is_negative = 1;
    }
    reset();
    copy(result);
    trim();
}

big_integer &big_integer::operator=(const big_integer &other) {
    if (this != &other) {
        copy(other);
        trim();
    }
    return *this;
}

big_integer &big_integer::operator+=(const big_integer &other) {
    if (is_negative != other.is_negative) {
        if (other.is_negative) {
            operator-=(-other);
            return *this;
        } else {
            copy(other - (-*this));
            return *this;
        }
    }
    reallocate(std::max(size, other.size) + 1);
    uint32_t carry = 0;
    for (size_t i = 0; i < size; ++i) {
        uint64_t res = (uint64_t) carry + digits[i];
        if (i < other.size) {
            res += other.digits[i];
        }
        carry = (uint32_t) (res / base);
        digits[i] = (uint32_t) (res % base);
    }
    trim();
    return *this;
}

big_integer &big_integer::operator-=(const big_integer &other) {
    if (is_negative != other.is_negative) {
        if (other.is_negative) {
            operator+=(-other);
            return *this;
        } else {
            copy(-(-(*this) + other));
            return *this;
        }
    }
    if (*this < other) {
        copy(-(other - *this));
        return *this;
    }
    reallocate(std::max(size, other.size));
    uint32_t carry = 0;
    for (size_t i = 0; i < size; ++i) {
        uint64_t res = (uint64_t) base + digits[i] - carry;
        if (i < other.size) {
            res -= other.digits[i];
        }
        if (res < base) {
            carry = 1;
        } else {
            carry = 0;
        }
        digits[i] = (uint32_t) (res % base);
    }
    trim();
    return *this;
}

big_integer big_integer::quotient(big_integer& y, uint64_t k) {
    size_t m = y.size;
    big_integer x = 0;
    x.reallocate(m);
    uint64_t carry = 0, temp;
    for (size_t i = m; i-- > 0;) {
        temp = carry * base + y.digits[i];
        x.digits[i] = (uint32_t) (temp / k);
        carry = temp % k;
    }
    return x;
}

big_integer &big_integer::operator/=(const big_integer &other) {
    return *this = *this / other;
}

big_integer &big_integer::operator*=(const big_integer &other) {
    std::vector<uint32_t> tmp;
    tmp.assign(size + other.size, 0);
    for (size_t i = 0; i < size; ++i) {
        uint32_t carry = 0;
        for (size_t j = 0; j < other.size; ++j) {
            uint64_t res = (uint64_t) digits[i] * other.digits[j] + carry;
            carry = (uint32_t) (res / base);
            if (uint64_t(tmp[i + j]) + (res % base) >= base) {
                tmp[i + j + 1] += 1;
            }
            tmp[i + j] += (uint32_t) (res % base);
        }
        tmp[i + other.size] += carry;
    }
    digits = tmp;
    size = size + other.size;
    trim();
    is_negative = other.is_negative != is_negative;
    return *this;
}

big_integer &big_integer::operator%=(const big_integer &other) {
    return *this = *this - (*this / other) * other;
}

big_integer &big_integer::operator&=(const big_integer &other) {
    big_integer a = to_2s_complement(*this);
    big_integer b = to_2s_complement(other);
    for (size_t i = 0; i < std::min(a.size, b.size); ++i) {
        (*this).digits[i] = a.digits[i] & b.digits[i];
    }
    for (size_t i = std::min(a.size, b.size); i < a.size; ++i) {
        (*this).digits[i] = 0;
    }
    this->is_negative = a.is_negative & b.is_negative;
    *this = to_normal(*this);
    return *this;
}

big_integer &big_integer::operator|=(const big_integer &other) {
    big_integer a = to_2s_complement(*this);
    big_integer b = to_2s_complement(other);
    a.reallocate(std::max(a.size, b.size));
    b.reallocate(std::max(a.size, b.size));
    for (size_t i = 0; i < a.size; ++i) {
        (*this).digits[i] = a.digits[i] | b.digits[i];
    }
    this->is_negative = a.is_negative | b.is_negative;
    *this = to_normal(*this);
    return *this;
}

big_integer &big_integer::operator^=(const big_integer &other) {
    big_integer a = to_2s_complement(*this);
    big_integer b = to_2s_complement(other);
    a.reallocate(std::max(a.size, b.size));
    b.reallocate(std::max(a.size, b.size));
    for (size_t i = 0; i < a.size; ++i) {
        (*this).digits[i] = a.digits[i] ^ b.digits[i];
    }
    this->is_negative = a.is_negative ^ b.is_negative;
    *this = to_normal(*this);
    return *this;
}

big_integer &big_integer::operator<<=(int shl) {
    big_integer a = *this;
    a *= (1 << (shl % 32));
    size_t k = (size_t) (shl / 32);
    reallocate(a.size + k);
    for (size_t i = 0; i < a.size; ++i) {
        this->digits[i + k] = a.digits[i];
    }
    for (size_t i = 0; i < k; ++i) {
        this->digits[i] = 0;
    }
    *this = *this;
    return *this;
}

big_integer &big_integer::operator>>=(int shr) {
    big_integer a = *this;
    a /= (1 << (shr % 32));
    size_t k = (size_t) (shr / 32);
    reallocate(a.size + k);
    for (size_t i = k; i < a.size; ++i) {
        this->digits[i - k] = a.digits[i];
    }
    if (is_negative) {
        *this -= 1;
    }
    return *this;
}

bool operator==(const big_integer &a, const big_integer &b) {
    if (a.is_negative != b.is_negative || a.size != b.size) {
        return false;
    }
    for (size_t i = 0; i < a.size; ++i) {
        if (a.digits[i] != b.digits[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(const big_integer &a, const big_integer &b) {
    return !(a == b);
}

bool operator<(const big_integer &a, const big_integer &b) {
    return !(a >= b);
}

bool operator>(const big_integer &a, const big_integer &b) {
    return !(a <= b);
}

bool operator<=(const big_integer &a, const big_integer &b) {
    return (a < b) || (a == b);
}

bool operator>=(const big_integer &a, const big_integer &b) {
    if (a.is_negative && !b.is_negative) {
        return false;
    }
    if (!a.is_negative && b.is_negative) {
        return true;
    }
    if (a.is_negative && b.is_negative) {
        return (-a) >= (-b);
    }
    if (a.size > b.size) {
        return true;
    }
    if (a.size < b.size) {
        return false;
    }
    for (size_t i = a.size; i-->0;) {
        if (a.digits[i] < b.digits[i]) {
            return false;
        }

        if (a.digits[i] > b.digits[i]) {
            return true;
        }
    }
    return true;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer tmp(*this);
    tmp.is_negative ^= true;
    tmp.trim();
    return tmp;
}

big_integer big_integer::operator~() const {
    big_integer a = *this;
    a.is_negative = !is_negative;
    a -= 1;
    return a;
}

big_integer big_integer::reverse_bits(big_integer a) const{
    for (size_t i = 0; i < a.size; ++i) {
        a.digits[i] = ~a.digits[i];
    }
    return a;
}

big_integer &big_integer::operator++() {
    *this += ONE;
    return *this;
}

big_integer big_integer::operator++(int) {
    big_integer tmp(*this);
    operator++();
    return tmp;
}

big_integer &big_integer::operator--() {
    *this -= ONE;
    return *this;
}

big_integer big_integer::operator--(int) {
    big_integer tmp(*this);
    operator++();
    return tmp;
}

std::string to_string(const big_integer& a) {
    std::string str;
    big_integer c = 0;
    big_integer tmp = a;
    bool neg = tmp.is_negative;
    tmp.is_negative = false;

    while (tmp > ZERO) {
        c = tmp % TEN;
        str += char(to_uint(c) + '0');
        tmp /= TEN;
    }
    if (str.size() == 0) {
        str = "0";
    } else if (neg) {
        str += '-';
    }
    std::reverse(str.begin(), str.end());
    return str;
}

void big_integer::reallocate(size_t new_size) {
    digits.resize(new_size, 0);
    size = new_size;
}

void big_integer::copy(const big_integer& other) {
    reallocate(other.size);
    for (size_t i = 0; i < size; ++i) {
        digits[i] = other.digits[i];
    }
    size = other.size;
    is_negative = other.is_negative;
}

big_integer big_integer::to_2s_complement(big_integer a) const{
    if (a.is_negative) {
        a = reverse_bits(a);
        a -= 1;
    }
    return a;
}

big_integer big_integer::to_normal(big_integer a) const{
    if (a.is_negative) {
        a += 1;
        a = reverse_bits(a);
    }
    return a;
}

uint32_t to_uint(const big_integer& a) {
    return a.digits[0];
}

void big_integer::reset() {
    digits.resize(0);
    size = 0;
    is_negative = false;
}

void big_integer::trim() {
    size_t new_size = 1;
    for (size_t i = size - 1; i > 0; --i) {
        if (digits[i] != 0) {
            new_size = i + 1;
            break;
        }
    }
    if (size != new_size) {
        reallocate(new_size);
    }
    if (size == 1 && digits[0] == 0) {
        is_negative = false;
    }
}

big_integer operator+(big_integer a, const big_integer &b) {
    a += b;
    return a;
}

big_integer operator-(big_integer a, const big_integer &b) {
    a -= b;
    return a;
}

big_integer operator*(big_integer a, const big_integer &b) {
    a *= b;
    return a;
}

big_integer operator/(big_integer a, big_integer b) {
    bool neg = (a.is_negative != b.is_negative);
    a.is_negative = false;
    b.is_negative = false;

    //assert(b != 0);

    if (a < b) {
        return ZERO;
    }

    if (b.size == 1) {
        big_integer x = a.quotient(a, (uint32_t)b.digits[0]);
        x.is_negative = neg;
        return x;
    }

    uint64_t f = a.base / (b.digits[b.size - 1] + 1);

    big_integer r = a * f;
    big_integer d = b * f;

    big_integer q = 0;
    q.reallocate(r.size - d.size + 1);
    big_integer dq = 0;

    big_integer h = 0;

    for (int k = r.size - 1; k > r.size - d.size; k--) {
        h *= big_base;
        h += r.digits[k];
    }

    for (size_t k = r.size - d.size + 1; k--;) {
        h *= big_base;
        h += r.digits[k];
        uint64_t qt;
        uint64_t r2 = h.digits.back();
        if (h.size > d.size) {
            r2 *= d.base;
            r2 += h.digits[h.size - 2];
        }

        uint64_t d1 = d.digits.back();

        qt = std::min(r2 / d1, d.base - 1);
        big_integer delta = d;

        dq = delta * qt;
        while (h < dq) {
            qt--;
            dq -= delta;
        }
        q.digits[k] = qt;
        h -= dq;
    }

    q.is_negative = neg;
    q.trim();
    return q;
}

big_integer operator%(big_integer a, const big_integer &b) {
    a %= b;
    return a;
}

big_integer operator&(big_integer a, const big_integer &b) {
    a &= b;
    return a;
}

big_integer operator|(big_integer a, const big_integer &b) {
    a |= b;
    return a;
}

big_integer operator^(big_integer a, const big_integer &b) {
    a ^= b;
    return a;
}

big_integer operator<<(big_integer a, int b) {
    a <<= b;
    return a;
}

big_integer operator>>(big_integer a, int b) {
    a >>= b;
    return a;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a)
{
    return s << to_string(a);
}
