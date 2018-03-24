#include "gtest/gtest.h"
#include "variant.h"
#include <string>
//#include <variant>
//using namespace std;

// copyable and movable struct
struct cm_my_struct {
	constexpr cm_my_struct() = default;

	cm_my_struct(cm_my_struct const&) {
		logger += "c";
	}

	cm_my_struct(cm_my_struct&&) noexcept {
		logger += "m";
	}

	cm_my_struct& operator=(cm_my_struct&&) noexcept {
		return *this;
	}

	cm_my_struct& operator=(cm_my_struct const&) {
		return *this;
	}

	static std::string logger;
};
std::string cm_my_struct::logger;

// copyable and not movable struct
struct c_my_struct {
	constexpr c_my_struct() = default;

	c_my_struct(c_my_struct const&) {
		logger += "c";
	}

	c_my_struct(c_my_struct&&) = delete;

	static std::string logger;
};
std::string c_my_struct::logger;

// movable and not copyable struct
struct m_my_struct {
	constexpr m_my_struct() = default;

	m_my_struct(m_my_struct const&) = delete;

	m_my_struct(m_my_struct&&) {
		logger += "m";
	}
	friend void swap(m_my_struct&, m_my_struct&) noexcept;
	static std::string logger;
};
std::string m_my_struct::logger;

// not copyable and not movable struct
struct my_struct {
	constexpr my_struct() = default;

	my_struct(my_struct const&) = delete;

	my_struct(my_struct&&) = delete;

	static std::string logger;
};
std::string my_struct::logger;

struct non_default_constructible_struct {
	non_default_constructible_struct() = delete;
};

TEST(testVariant, defaultConstructible) {
	bool b = std::is_default_constructible_v<variant<int, double>>;
	EXPECT_TRUE(b);
	b = std::is_default_constructible_v<variant<non_default_constructible_struct, int>>;
	EXPECT_FALSE(b);
}

struct non_trivially_destructible_struct {
	~non_trivially_destructible_struct() {}
};

TEST(testVariant, triviallyDestructible) {
	variant<int, non_trivially_destructible_struct> non_triv_destr;;
	variant<int, double> triv_destr;
	bool b = std::is_trivially_destructible_v<variant<int, non_trivially_destructible_struct>>;
	EXPECT_FALSE(b);
	b = std::is_trivially_destructible_v<variant<int, double>>;
	EXPECT_TRUE(b);
}

TEST(testVariant, constexprTest) {
	constexpr variant<cm_my_struct> cm;
	constexpr variant<c_my_struct> c;
	constexpr variant<m_my_struct> m;
	constexpr variant<my_struct> ncnm;

	EXPECT_TRUE(std::is_default_constructible_v<variant<cm_my_struct>>);
	EXPECT_TRUE(std::is_default_constructible_v<variant<c_my_struct>>);
	EXPECT_TRUE(std::is_default_constructible_v<variant<m_my_struct>>);
	EXPECT_TRUE(std::is_default_constructible_v<variant<my_struct>>);
	variant<non_default_constructible_struct, int> v(in_place_index_t<1>(), 42);
}

struct st0 {
	st0() {
		logger += "ct";
	}
	~st0() {
		logger += "d";
	}
	static std::string logger;
};
std::string st0::logger;

struct st {
	st() {
		logger += "ct";
	}

	st(int a, int b) {
		logger += "ct(a, b)";
	}

	~st() {
		logger += "d";
	}
	static std::string logger;
};
std::string st::logger;

TEST(testVariant, emplace1) {
	st::logger = st0::logger = "";
	variant<st0, st> v;
	EXPECT_EQ(st::logger, "");
	EXPECT_EQ(st0::logger, "ct");
	v.emplace<st>(10, 20);
	EXPECT_EQ(st::logger, "ct(a, b)");
	EXPECT_EQ(st0::logger, "ctd");
}

TEST(testVariant, emplace3) {
	st::logger = st0::logger = "";
	variant<st0, st> v;
	EXPECT_EQ(st::logger, "");
	EXPECT_EQ(st0::logger, "ct");
	v.emplace<1>(10, 20);
	EXPECT_EQ(st::logger, "ct(a, b)");
	EXPECT_EQ(st0::logger, "ctd");
}

TEST(testVariant, swap) {
	variant<int, double> v(in_place_index_t<1>(), 3.14), w(in_place_index_t<0>(), 42);
	swap(v, w);
	EXPECT_EQ(get<0>(v), 42);
	EXPECT_EQ(get<1>(w), 3.14);
	variant<int, double> v1(in_place_index_t<1>(), 3.14), w1(in_place_index_t<1>(), 1.234);
	swap(w1, v1);
	EXPECT_EQ(get<1>(v1), 1.234);
	EXPECT_EQ(get<1>(w1), 3.14);
}

TEST(testVariant, exceptionSafety) {
	variant<cm_my_struct> v, w;
	v = std::move(w);
}

TEST(testVariant, holdsAlternative) {
	variant<int, std::string> v = std::string("abc");
	EXPECT_FALSE(holds_alternative<int>(v));
	EXPECT_TRUE(holds_alternative<std::string>(v));
}

TEST(testVariant, get_if) {
	variant<int, float> v(12);
	EXPECT_EQ(*get_if<int>(&v), 12);
}

TEST(testVariant, checkEquals) {
	variant<int, double> v(52), w(3.14);
	EXPECT_FALSE(v == w);
	w = 52;
	EXPECT_TRUE(v == w);
}

TEST(testVariant, checkNotEquals) {
	variant<int, double> v(52), w(3.14);
	EXPECT_TRUE(v != w);
	w = 52;
	EXPECT_FALSE(v != w);
}

struct testNotEq {
	friend bool operator==(testNotEq const&, testNotEq const&) {
		return false;
	}

	friend bool operator!=(testNotEq const&, testNotEq const&) {
		return false;
	}
};

TEST(testVariant, checkCheatingNotEquals) {
	variant<testNotEq> v, w;
	EXPECT_FALSE(v == w);
	EXPECT_FALSE(v != w);
}

TEST(testVariant, checkLess) {
	variant<int, double> v(3), w(2.14);
	EXPECT_TRUE(v < w);
	v = 3.0;
	EXPECT_FALSE(v < w);
}

TEST(testVariant, checkGreater) {
	variant<int, double> v(3), w(2.14);
	EXPECT_FALSE(v > w);
	v = 3.0;
	EXPECT_TRUE(v > w);
}

struct fo {
	std::string operator()(int a, int b) const {
		std::stringstream ss;
		ss << a << " + " << b;
		return ss.str();
	}
	std::string operator()(std::string a, std::string b) const {
		std::stringstream ss;
		ss << a << " + " << b;
		return ss.str();
	}
	std::string operator()(std::string a, int b) const {
		std::stringstream ss;
		ss << a << " + " << b;
		return ss.str();
	}
	std::string operator()(int a, std::string b) const {
		std::stringstream ss;
		ss << a << " + " << b;
		return ss.str();
	}
};

struct fo2 {
	template <typename T1, typename T2>
	std::string operator()(T1 a, T2 b) const {
		std::stringstream ss;
		ss << a << " + " << b;
		return ss.str();
	}
};

TEST(testVariant, ctor4) {
	variant<std::string, bool> v("abc");
	EXPECT_EQ(v.index(), 1u);
	EXPECT_TRUE(get<1>(v));
}

TEST(testVariant, ctor5) {
	variant<bool, std::string> v(in_place_type_t<std::string>(), "hello world");
	EXPECT_EQ(v.index(), 1u);
	EXPECT_EQ(get<1>(v), "hello world");
}

struct S {
	S(int i) : i(i) {}
	int i;
};

TEST(testVariant, testMonostate) {
	variant<monostate, S> var;
	EXPECT_EQ(var.index(), 0u);
	var = 12;
	EXPECT_EQ(var.index(), 1u);
	EXPECT_EQ(get<S>(var).i, 12);
}

void simpleFoo(cm_my_struct&) {

}

TEST(testVariant, simpleVisit) {
	variant<cm_my_struct> v;
	cm_my_struct::logger = "";
	visit(simpleFoo, v);
	EXPECT_EQ(cm_my_struct::logger, "");
}

TEST(testVariant, visit) {
	auto lambda = [](auto a, auto b) -> std::string {
		std::stringstream ss;
		ss << a << " + " << b;
		return ss.str();
	};
	variant<int, std::string> v(3), w(std::string("hello"));
	EXPECT_EQ(visit(fo2(), v, w), "3 + hello");
	EXPECT_EQ(visit(lambda, v, w), "3 + hello");
	EXPECT_EQ(visit(fo(), v, w), "3 + hello");
	v = std::string("world");
	EXPECT_EQ(visit(fo(), v, w), "world + hello");
	w = 42;
	EXPECT_EQ(visit(fo(), v, w), "world + 42");
	EXPECT_EQ(visit(lambda, v, w), "world + 42");
	variant<int, double> vv(3.14);
	EXPECT_EQ(visit(fo2(), v, vv), "world + 3.14");
	EXPECT_EQ(visit(lambda, v, vv), "world + 3.14");
	v = 100;
	EXPECT_EQ(visit(fo(), v, w), "100 + 42");
	EXPECT_EQ(visit(lambda, v, w), "100 + 42");
}

TEST(testVariant, badVariantAccess) {
	variant<int, float> v;
	v = 12;
	std::string err;
	try {
		get<float>(v);
	} catch (const bad_variant_access& e) {
		err = e.what();
	}
	EXPECT_TRUE(err == "bad_variant_access" || err == "bad variant access");
}

TEST(testVariant, mainExampleFromCppRef) {
	variant<int, float> v, w;
	v = 12;
	EXPECT_EQ(get<int>(v), 12);
	w = get<int>(v);
	EXPECT_EQ(get<0>(w), 12);
	v = 13;
	w = get<0>(v);
	w = v;
	EXPECT_EQ(get<int>(w), 13);

	bool b = false;
	try {
		get<float>(w);
	} catch (const bad_variant_access&) {
		b = true;
	}
	EXPECT_TRUE(b);

	variant<std::string> x("abc");
	EXPECT_EQ(get<0>(x), "abc");
	x = "def";
	EXPECT_EQ(get<0>(x), "def");

	variant<std::string, bool> y("abc");
	EXPECT_TRUE(holds_alternative<bool>(y));
	y = std::string("xyz");
	EXPECT_TRUE(holds_alternative<std::string>(y));
}

struct stype {
    int x {};
    double g {};
    std::vector<int> v {};
    std::string s {};

    stype(int x, double g, std::string s) : x(x), g(g), v{x, static_cast<int>(g)}, s(s) {}

    template<typename T>
    stype(std::initializer_list<T> lst) : x(static_cast<int>(lst.size())) {}
};

struct stype_constexpr {
    int y;

    constexpr stype_constexpr(int y) : y(y) {}

    ~stype_constexpr() = default;
};

#define COMPLEX_TYPES_WITH_USER_DEFINED int,double,std::string,stype
#define COMPLEX_TYPES int,double,std::string
#define CONSTEXPR_TYPES int,double
#define CONSTEXPR_TYPES_WITH_USER_DEFINED int,double,stype_constexpr

void c(variant<int>&& , variant<int>&& ) {
    std::cout << "ASDASD" << std::endl;
}

TEST(Construction, from_type) {
    std::string test1;
    variant<COMPLEX_TYPES> v(std::string("asdasd"));
    test1 += std::to_string(v.index());
    variant<COMPLEX_TYPES> vv(1.0);
    test1 += std::to_string(vv.index());
    variant<COMPLEX_TYPES> vvv(std::move(v));
    test1 += std::to_string(vvv.index());
    EXPECT_EQ(test1, "212");
   // std::cerr << test1 << std::endl; // ans = 212
}


TEST(Construction, copy_move) {
    std::string test2;
    constexpr variant<CONSTEXPR_TYPES> q(1.1);
    test2 += std::to_string(q.index());
    variant<CONSTEXPR_TYPES> qq(q);
    test2 += std::to_string(qq.index());
    variant<CONSTEXPR_TYPES> qqq(std::move(q));
    test2 += std::to_string(qqq.index());
    test2 += std::to_string(q.index());
    EXPECT_EQ(test2, "1111");
//    std::cout << test2 << std::endl; // ans = 1111
}


TEST(Constuction, in_place_index) {
    std::string test3;
    variant<COMPLEX_TYPES_WITH_USER_DEFINED> g(in_place_index<3>, 1, 11.4, std::string("Heeellllooooo"));
    test3 += std::to_string(g.index());
    EXPECT_EQ(test3, "3");
//    std::cout << test3 << std::endl;   // ans = 3
}

TEST(Construction, in_place_index_1) {
    std::string test4;
    constexpr variant<CONSTEXPR_TYPES_WITH_USER_DEFINED> gg(in_place_index<2>, 11);
    test4 += std::to_string(gg.index());
    EXPECT_EQ(test4, "2");
//    std::cout << test4 << std::endl; // ans = 2
}

TEST(Construction, in_place_type) {
    std::string test5;
    variant<COMPLEX_TYPES_WITH_USER_DEFINED> w(in_place_type<stype>, 1, 11.4, std::string("Heeellllooooo"));
    test5 += std::to_string(w.index());
    EXPECT_EQ(test5, "3");
//    std::cout << test5 << std::endl;   // ans = 3
}

TEST(Construction, in_place_type_1) {
    std::string test6;
    constexpr variant<CONSTEXPR_TYPES_WITH_USER_DEFINED> ww(in_place_type<stype_constexpr>, 11);
    test6 += std::to_string(ww.index());
    EXPECT_EQ(test6, "2");
//    std::cout << test6 << std::endl; // ans = 2
}

TEST(Construction, in_place_type_2) {
    std::string test7;
    constexpr variant<CONSTEXPR_TYPES> r(in_place_type<int>, 11);
    test7 += std::to_string(r.index());
    constexpr variant<CONSTEXPR_TYPES> rr(in_place_type<double>, 1.1);
    test7 += std::to_string(rr.index());
    EXPECT_EQ(test7, "01");
//    std::cout << test7 << std::endl; // ans 01
}

TEST(Construction, in_place_type_li) {
    std::string test8;
    variant<COMPLEX_TYPES_WITH_USER_DEFINED> www(in_place_type<stype>, {1, 1, 1});
    test8 += std::to_string(www.index());
    EXPECT_EQ(test8, "3");
//    std::cout << test8 << std::endl;   // ans = 3
}

TEST(Construction, in_place_index_li) {
    std::string test9;
    variant<COMPLEX_TYPES_WITH_USER_DEFINED> wwww(in_place_index<3>, {1, 1, 1});
    test9 += std::to_string(wwww.index());
    EXPECT_EQ(test9, "3");
//    std::cout << test9 << std::endl;   // ans = 3
}


TEST(Assignment, copy) {
    std::string test10;
    variant<COMPLEX_TYPES> c(123);
    test10 += std::to_string(c.index());
    variant<COMPLEX_TYPES> cc(std::string("asd"));
    test10 += std::to_string(cc.index());
    c = cc;
    test10 += std::to_string(c.index());
    EXPECT_EQ(test10, "022");
//    std::cout << test10 << std::endl;   // ans = 022
}



TEST(Assignment, copy_1) {
    std::string test11;
    variant<CONSTEXPR_TYPES> x(123);
    test11 += std::to_string(x.index());
    variant<CONSTEXPR_TYPES> xx(2.0);
    test11 += std::to_string(xx.index());
    x = xx;
    test11 += std::to_string(x.index());
    EXPECT_EQ(test11, "011");
//    std::cout << test11 << std::endl;   // ans = 011
}

TEST(Assignment, move) {
    std::string test12;
    variant<COMPLEX_TYPES> a(123);
    test12 += std::to_string(a.index());
    a = variant<COMPLEX_TYPES>(std::string("asd"));
    test12 += std::to_string(a.index());
    EXPECT_EQ(test12, "02");
//    std::cout << test12 << std::endl;   // ans = 02
}

TEST(Assignment, move_1) {
    std::string test13;
    variant<CONSTEXPR_TYPES> b(123);
    test13 += std::to_string(b.index());
    b = variant<CONSTEXPR_TYPES> (2.0);;
    test13 += std::to_string(b.index());
    EXPECT_EQ(test13, "01");
//    std::cout << test13 << std::endl;   // ans = 01
}

TEST(Assignment, move_copy_type) {
    std::string test14;
    int a14 = 13;
    double b14 = 14;
    variant<COMPLEX_TYPES> e(a14);
    test14 += std::to_string(e.index());
    e = b14;
    test14 += std::to_string(e.index());
    e = std::string("asd");
    test14 += std::to_string(e.index());
    EXPECT_EQ(test14, "012");
//    std::cout << test14 << std::endl;   // ans = 012
}

TEST(Assignment, move_literal) {
    std::string test15;
    variant<CONSTEXPR_TYPES> ee(123);
    test15 += std::to_string(ee.index());
    ee = 12.1;
    test15 += std::to_string(ee.index());
    ee = 1;
    test15 += std::to_string(ee.index());
    EXPECT_EQ(test15, "010");
//    std::cout << test15<< std::endl;   // ans = 010
}

TEST(Holds_alternative, type) {
    std::string test16;
    variant<COMPLEX_TYPES> t;
    std::string sss = "abacaba";
    t = sss;
    test16 += std::to_string(holds_alternative<std::string>(t));
    // test16 += std::to_string(holds_alternative<stype>(t));
    test16 += std::to_string(holds_alternative<int>(t));
    t = 1.0;
    // test16 += std::to_string(holds_alternative<std::vector<int>>(t));
    test16 += std::to_string(holds_alternative<double>(t));
    EXPECT_EQ(test16, "101");
//    std::cout << test16 << std::endl;  // ans = 10001
}

TEST(Get, index) {
    std::string test17;
    variant<COMPLEX_TYPES> u(11);
    int u1 = get<0>(u);
    test17 += std::to_string(u1) + " ";
    u = std::string("hello");
    test17 += get<2>(u) + " ";
    variant<COMPLEX_TYPES> uu(variant<COMPLEX_TYPES>(2.0));
    test17 += std::to_string(uu.index()) + " " + std::to_string(get<1>(uu)) + " ";
    u = uu;
    test17 += std::to_string(get<1>(u));
    test17 += get<2>(variant<COMPLEX_TYPES> (in_place_index<2>, " bye"));
    EXPECT_EQ(test17, "11 hello 1 2.000000 2.000000 bye");
//    std::cout << test17 << std::endl;  // ans = 11 hello 1 2.0 2.0 bye
}

TEST(Get, type) {
    std::string test18;
    variant<COMPLEX_TYPES> y(11);
    int y1 = get<int>(y);
    test18 += std::to_string(y1) + " ";
    y = std::string("hello");
    test18 += get<std::string>(y) + " ";
    variant<COMPLEX_TYPES> yy(variant<COMPLEX_TYPES>(2.0));
    test18 += std::to_string(yy.index()) + " " + std::to_string(get<double>(yy)) + " ";
    y = yy;
    test18 += std::to_string(get<double>(y));
    test18 += get<std::string>(variant<COMPLEX_TYPES> (in_place_type<std::string>, " bye"));
    EXPECT_EQ(test18, "11 hello 1 2.000000 2.000000 bye");
//    std::cout << test18 << std::endl;  // ans = 11 hello 1 2.0 2.0 bye
}

TEST(Get, exception) {
    std::string test19;
    constexpr variant<CONSTEXPR_TYPES> i;
    try {
        test19 += std::to_string(get<double>(i));
    } catch(bad_variant_access& e) {
        test19 += "BAD";
    }
    EXPECT_EQ(test19, "BAD");
//    std::cout << test19 << std::endl; // ans = BAD
}



TEST(Visit, one_arg) {
    std::string test20;
	// variant<CONSTEXPR_TYPES> t(in_place_index<0>, 11);
	visit([&test20](auto&& a) {
        test20 += std::to_string(1 + a);
    }, variant<CONSTEXPR_TYPES>(in_place_index<0>, 11));
    EXPECT_EQ(test20, "12");
//    std::cout << test20 << std::endl; // ans = 12
}


TEST(Visit, two_arg) {
    variant<CONSTEXPR_TYPES> ll(11);
    variant<CONSTEXPR_TYPES> l(11);

    std::string test21;
    visit([&test21](auto&& a, auto&& b) {
        test21 += std::to_string(b + a);
	    },
	    l,
	    ll
	    );
    EXPECT_EQ(test21, "22");
//    std::cout << test21 << std::endl; // ans 22
}

TEST(Get_if, trivial) {
    std::string test22;
    variant<int, float> f{12};
    if(auto pval = get_if<int>(&f)) {
      test22 += std::string("variant value: ");
      test22 += std::to_string(*pval);
    } else
      test22 += "failed to get value!";
    EXPECT_EQ(test22, "variant value: 12");
//    std::cout << test22 << std::endl;
}

TEST(Swap, trivial) {
    std::string test24;
    variant<CONSTEXPR_TYPES> f1{1};
    variant<CONSTEXPR_TYPES> f2{2};
    test24 += std::to_string(get<0>(f1)) + std::to_string(get<0>(f2)) + "\n";
    swap(f1, f2);
    test24 += std::to_string(get<0>(f1)) + std::to_string(get<0>(f2));
    EXPECT_EQ(test24, "12\n21");
//    std::cout << test24 << std::endl;
}

TEST(Emplace, trivial) {
    std::string test25;
    variant<COMPLEX_TYPES_WITH_USER_DEFINED> f3;
    test25 += std::to_string(get<0>(f3));
    f3.emplace<stype>(9, 0.0, std::string("asd"));
    test25 += get<stype>(f3).s;
    EXPECT_EQ(test25, "0asd");
//    std::cout << test25 << std::endl;
}
