#include <iostream>
#include "big_integer.h"

big_integer();
big_integer(big_integer const& other);
big_integer(int a);
explicit big_integer(std::string const& str);
~big_integer();

big_integer& operator=(big_integer const& other);

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

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

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
