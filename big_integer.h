#include <string>
#include <vector>

#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H


class big_integer {
public:
    ~big_integer();
    big_integer();
    big_integer(const big_integer& other);
    big_integer(int a);
    explicit big_integer(const std::string& str);

    big_integer& operator=(const big_integer& other);
    big_integer& operator+=(const big_integer& other);
    big_integer& operator-=(const big_integer& other);
    big_integer& operator/=(const big_integer& other);
    big_integer& operator*=(const big_integer& other);
    big_integer& operator%=(const big_integer& other);
    big_integer& operator&=(const big_integer& other);
    big_integer& operator|=(const big_integer& other);
    big_integer& operator^=(const big_integer& other);
    big_integer& operator<<=(int other);
    big_integer& operator>>=(int other);

    friend bool operator==(const big_integer& a, const big_integer& b);
    friend bool operator!=(const big_integer& a, const big_integer& b);
    friend bool operator<(const big_integer& a, const big_integer& b);
    friend bool operator>(const big_integer& a, const big_integer& b);
    friend bool operator<=(const big_integer& a, const big_integer& b);
    friend bool operator>=(const big_integer& a, const big_integer& b);

    friend big_integer operator/(big_integer a, big_integer b);

    big_integer operator+() const;
    big_integer operator-() const;
    big_integer operator~() const;

    big_integer& operator++();
    big_integer operator++(int);

    big_integer& operator--();
    big_integer operator--(int);

    friend std::string to_string(const big_integer& a);


private:
    big_integer(uint32_t a);
    big_integer(uint64_t a);
    std::vector<uint32_t> digits;
    size_t size;
    bool is_negative;
    uint64_t base = (uint64_t) 1 << 32;

    big_integer to_2s_complement(big_integer a) const;
    big_integer to_normal(big_integer a) const;
    big_integer reverse_bits(big_integer a) const;

    void reset();
/*
 *  @Contract("size == 1")
 */
    friend uint32_t to_uint(const big_integer& a);

    void reallocate(size_t new_size);
    void copy(const big_integer& other);
    void trim();

    big_integer quotient(big_integer& y, uint64_t k);
};

bool operator==(const big_integer& a, const big_integer& b);
bool operator!=(const big_integer& a, const big_integer& b);
bool operator<(const big_integer& a, const big_integer& b);
bool operator>(const big_integer& a, const big_integer& b);
bool operator<=(const big_integer& a, const big_integer& b);
bool operator>=(const big_integer& a, const big_integer& b);

big_integer operator+(big_integer a, const big_integer& b);
big_integer operator-(big_integer a, const big_integer& b);
big_integer operator*(big_integer a, const big_integer& b);
big_integer operator/(big_integer a, big_integer b);
big_integer operator%(big_integer a, const big_integer& b);

big_integer operator&(big_integer a, const big_integer& b);
big_integer operator|(big_integer a, const big_integer& b);
big_integer operator^(big_integer a, const big_integer& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

std::string to_string(const big_integer& a);
uint32_t to_uint(const big_integer& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);

#endif //BIG_INTEGER_H
