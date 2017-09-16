#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

struct big_integer
{
    using value_type = uint32_t;
    using tr_value_type = uint64_t;

    big_integer();
    big_integer(const big_integer& other);
    big_integer(int a);
    explicit big_integer(std::string const& str);
    ~big_integer();

    big_integer& operator=(const big_integer& other);

    big_integer& operator+=(big_integer const& rhs);
    big_integer& operator-=(big_integer const& rhs);
    big_integer& operator*=(big_integer const& rhs);
    big_integer& operator/=(big_integer const& rhs);
    big_integer& operator%=(big_integer const& rhs);

    big_integer& operator&=(big_integer const& rhs);
    big_integer& operator|=(big_integer const& rhs);
    big_integer& operator^=(big_integer const& rhs);

    big_integer& operator<<=(int rhs);
    big_integer& operator>>=(int rhs);

    big_integer operator+() const;
    big_integer operator-() const;
    big_integer operator~() const;

    big_integer& operator++();
    big_integer operator++(int);

    big_integer& operator--();
    big_integer operator--(int);

    friend bool operator==(big_integer const& a, big_integer const& b);
    friend bool operator!=(big_integer const& a, big_integer const& b);
    friend bool operator<(big_integer const& a, big_integer const& b);
    friend bool operator>(big_integer const& a, big_integer const& b);
    friend bool operator<=(big_integer const& a, big_integer const& b);
    friend bool operator>=(big_integer const& a, big_integer const& b);

    friend std::string to_string(big_integer const& a);
    void out() const;
    void test();

private:
    static constexpr int BITS { 32 }; // TODO
    static constexpr tr_value_type BASE { static_cast<tr_value_type>(1) << BITS }; // TODO
    enum
    {
        SMALL,
        BIG
    } state;
    union
    {
        long long number;
        value_type* array;
    } representation;   // the code would be nicer if the union was in placement,
                        // but the variable is needed for swap function alone

    value_type& operator[](size_t n) { return representation.array[n]; }
    value_type& ref_count() { return representation.array[-2]; }
    value_type& size() { return representation.array[-1]; }
    const value_type& operator[](size_t n) const { return representation.array[n]; }
    const value_type& ref_count() const { return representation.array[-2]; }
    const value_type& size() const { return representation.array[-1]; }
    bool sign() const { return state == BIG ? representation.array[size() - 1] >> (BITS - 1) : representation.number < 0; }

    void to_big_object();
    void detach();
    void swap(big_integer& tmp);
    void quick_copy(const big_integer& other);
    void reverse_bytes();
    bool convert_to_signed();
    void convert_to_2s(bool sign);
    void trim();
    void reallocate(value_type new_size);
    big_integer from_value_type(value_type t);
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);

#endif // BIG_INTEGER_H
