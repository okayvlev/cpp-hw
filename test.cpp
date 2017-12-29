/*
	Testset designed by Kirill Kozlov("https://github.com/kirill146/")
	and modified for personal testing
*/

#include <vector>
#include "gtest/gtest.h"
#include "persistent_set.h"
#include "smart_pointers/linked_ptr.h"
#include "smart_pointers/shared_ptr.h"
#include "persistent_set.h"

using namespace std;

template <typename T>
vector<T> vec(persistent_set<T> const& s) {
	vector<T> res;
	for (auto it = s.begin(); it != s.end(); ++it) {
		res.push_back(*it);
	}
	return res;
}

TEST(test_set, insert) {
	persistent_set<int> s;
	s.insert(32);
	s.insert(33);
	EXPECT_EQ(vector<int>({32, 33}), vec(s));
}
TEST(test_set, generic1) {
	persistent_set<int> s;
	s.insert(3);
	// s.out();
	s.insert(5);
	// s.out();
	s.insert(4);
	// s.out();
	s.insert(1);
	// s.out();
	if (s.find(3) != s.end())
	s.erase(s.find(3));
	// s.out();
	EXPECT_EQ(vector<int>({ 1, 4, 5 }), vec(s));

}

TEST(test_set, insert_10_elements) {
	persistent_set<int> s;
	for (int i = 0; i < 5; i++) {
		s.insert(i);
	}
	for (int j = 9; j >= 5; j--) {
		s.insert(j);
	}
	EXPECT_EQ(vector<int>({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }), vec(s));
}

TEST(test_set, copy) {
	persistent_set<int> s;
	s.insert(1);
	persistent_set<int> p(s);
	p.insert(2);
	EXPECT_EQ(vector<int>({ 1 }), vec(s));
	EXPECT_EQ(vector<int>({ 1, 2 }), vec(p));
}

TEST(test_set, copy2) {
	persistent_set<int> s;
	s.insert(12);
	persistent_set<int> p(s);
	s.insert(45);
	EXPECT_EQ(vector<int>({ 12, 45 }), vec(s));
	EXPECT_EQ(vector<int>({ 12 }), vec(p));
	p.insert(0);
	EXPECT_EQ(vector<int>({ 12, 45 }), vec(s));
	EXPECT_EQ(vector<int>({ 0, 12 }), vec(p));
}

TEST(test_set, insert_and_copy) {
	persistent_set<int> s;
	s.insert(4);
	s.insert(5);
	s.insert(7);
	s.insert(6);
	s.insert(1);
	s.insert(2);
	s.insert(8);
	s.insert(3);
	s.insert(0);
	s.insert(9);
	EXPECT_EQ(vector<int>({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }), vec(s));
	persistent_set<int> p(s);
	s.insert(100);
	EXPECT_EQ(vector<int>({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 100 }), vec(s));
	EXPECT_EQ(vector<int>({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }), vec(p));

	persistent_set<int>::iterator it = s.find(100);
	persistent_set<int>::iterator it2 = s.find(9);
	swap(it, it2);
	EXPECT_EQ(it2, s.find(100));
}

TEST(test_set, iterators) {
	persistent_set<int> s;
	s.insert(4);
	s.insert(5);
	s.insert(7);
	s.insert(6);
	s.insert(1);
	s.insert(2);
	s.insert(8);
	s.insert(3);
	s.insert(0);
	s.insert(9);
	EXPECT_EQ(vector<int>({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }), vec(s));
	persistent_set<int> p(s);
	s.insert(100);
	EXPECT_EQ(vector<int>({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 100 }), vec(s));
	EXPECT_EQ(vector<int>({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }), vec(p));

	persistent_set<int>::iterator it = s.find(100);
	persistent_set<int>::iterator it2 = s.find(9);
	persistent_set<int>::iterator tmp;
	tmp = it;
	it = it2;
	it2 = tmp;
	// swap(it, it2);
	EXPECT_EQ(it, s.find(9));
	EXPECT_EQ(it2, s.find(100));
}

TEST(test_set, erase0) {
	persistent_set<int> s;
	s.insert(4);
	s.erase(s.begin());
	EXPECT_EQ(vector<int>({}), vec(s));
}

TEST(test_set, erase) {
	persistent_set<int> s;
	EXPECT_EQ(vector<int>({ }), vec(s));
	s.insert(4);
	EXPECT_EQ(vector<int>({ 4 }), vec(s));
	s.insert(5);
	EXPECT_EQ(vector<int>({ 4, 5 }), vec(s));
	s.insert(7);
	EXPECT_EQ(vector<int>({ 4, 5, 7 }), vec(s));
	s.insert(6);
	EXPECT_EQ(vector<int>({ 4, 5, 6, 7 }), vec(s));
	s.insert(1);
	EXPECT_EQ(vector<int>({ 1, 4, 5, 6, 7 }), vec(s));
	s.insert(2);
	EXPECT_EQ(vector<int>({ 1, 2, 4, 5, 6, 7 }), vec(s));
	s.insert(8);
	EXPECT_EQ(vector<int>({ 1, 2, 4, 5, 6, 7, 8 }), vec(s));
	s.insert(3);
	EXPECT_EQ(vector<int>({ 1, 2, 3, 4, 5, 6, 7, 8 }), vec(s));
	s.insert(0);
	EXPECT_EQ(vector<int>({ 0, 1, 2, 3, 4, 5, 6, 7, 8 }), vec(s));
	s.insert(9);
	EXPECT_EQ(vector<int>({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }), vec(s));
	s.erase(s.begin());
	EXPECT_EQ(vector<int>({ 1, 2, 3, 4, 5, 6, 7, 8, 9 }), vec(s));
	s.erase(s.find(5));
	EXPECT_EQ(vector<int>({ 1, 2, 3, 4, 6, 7, 8, 9 }), vec(s));
	s.erase(s.find(3));
	EXPECT_EQ(vector<int>({ 1, 2, 4, 6, 7, 8, 9 }), vec(s));
	// s.out();
	s.erase(s.find(7));
	EXPECT_EQ(vector<int>({ 1, 2, 4, 6, 8, 9 }), vec(s));
	s.erase(s.find(1));
	EXPECT_EQ(vector<int>({ 2, 4, 6, 8, 9 }), vec(s));
	s.erase(s.find(8));
	EXPECT_EQ(vector<int>({ 2, 4, 6, 9 }), vec(s));
	s.erase(s.find(9));
	EXPECT_EQ(vector<int>({2, 4, 6}), vec(s));
	s.erase(s.find(4));
	EXPECT_EQ(vector<int>({ 2, 6 }), vec(s));
	s.erase(s.begin());
	EXPECT_EQ(vector<int>({ 6 }), vec(s));
	s.erase(s.begin());
	EXPECT_EQ(vector<int>({ }), vec(s));
	s.insert(100);
	EXPECT_EQ(vector<int>({ 100 }), vec(s));
	s.erase(s.find(100));
	EXPECT_EQ(vector<int>({}), vec(s));
}

TEST(test_set, erase2) {
	persistent_set<int> s;
	s.insert(4);
	s.insert(5);
	s.insert(7);
	s.insert(6);
	s.insert(1);
	s.insert(2);
	s.insert(8);
	s.insert(3);
	s.insert(0);
	s.insert(9);
	EXPECT_EQ(vector<int>({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }), vec(s));
	persistent_set<int> p;
	for (int i = 0; i < 10; i++) {
		p = s;
		auto it = p.begin();
		for (int j = 0; j < i; j++) {
			it++;
		}
		p.erase(it);
		vector<int> ans({ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
		ans.erase(ans.begin() + i);
		EXPECT_EQ(ans, vec(p));
	}
}


std::set<void*> cnt;

struct Counter {
 int x;
 Counter(int x = 0) : x(x) {
  cnt.insert(this);
 }
 Counter(Counter const& other): x(other.x) {
  cnt.insert(this);
 }
 void swap(Counter& other) {
  std::swap(x, other.x);
 }
 Counter& operator=(Counter const& other) {
  Counter tmp(other);
  swap(tmp);
  return *this;
 }
 bool operator<(Counter const& other) const {
  return x < other.x;
 }
 bool operator>(Counter const& other) const {
  return x > other.x;
 }
 bool operator==(Counter const& other) const {
  return x == other.x;
 }
 friend ostream& operator<<(ostream& o, Counter const& c) {
  o << c.x;
  return o;
 }
 ~Counter() {
  cnt.erase(this);
 }
};

TEST(test_set, counter_no_cpy) {
 if (true) {
  persistent_set<Counter> s;
  for (int i = 0; i < 25; ++i)
   s.insert(Counter(i));
  for (int i = 0; i < 5; ++i)
   s.insert(*s.begin());
  for (int i = 1; i <= 5; ++i) {
   auto it = s.end();
   for (int j = 0; j <= i; ++j) {
	//    cerr << "1\n";
    --it;
	// cerr << "1\n";
}
   s.insert(*it);
  }
  // cerr << "1\n";
  s.erase(s.begin());
  for (int i = 0; i < 10; ++i) {
   auto it = s.begin();
   for (int j = 0; j < i; ++j)
    ++it;
   s.erase(it);
  }
 }
 cout << "objects count : " << cnt.size() << '\n';
 EXPECT_TRUE(cnt.empty());
 cnt.clear();
}

TEST(test_set, counter_cpy) {
 if (true) {
  persistent_set<Counter> s;
  for (int i = 0; i < 25; ++i)
   s.insert(Counter(i));
  persistent_set<Counter> p(s);
  for (int i = 0; i < 5; ++i)
   p.insert(*p.begin());
  for (int i = 1; i <= 5; ++i) {
   auto it = p.end();
   for (int j = 0; j <= i; ++j)
    --it;
   p.insert(*it);
  }
  p.erase(p.begin());
  for (int i = 0; i < 10; ++i) {
   auto it = p.begin();
   for (int j = 0; j < i; ++j)
    ++it;
   p.erase(it);
  }
 }
 cout << "objects count : " << cnt.size() << '\n';
 EXPECT_TRUE(cnt.empty());

}


struct smth {
	int var;
	smth(int var) : var(var) {}
	smth() = delete;
	smth(smth const& rhs) {
		var = rhs.var;
	}
	smth(smth&& rhs) noexcept {
		var = rhs.var;
	}
	bool operator<(smth const& rhs) const {
		return var < rhs.var;
	}
	bool operator>(smth const& rhs) const {
		return var > rhs.var;
	}
	bool operator==(smth const& rhs) const {
		return var == rhs.var;
	}
};

TEST(test_set, smth) {
	persistent_set<smth> s;
	s.insert(smth(2));
	s.insert(smth(1));
	s.erase(s.find(2));
	s.insert(smth(5));
	EXPECT_EQ(vector<smth>({1, 5}), vec(s));
	persistent_set<smth> p(s);
	p.insert(smth(0));
	EXPECT_EQ(vector<smth>({ 1, 5 }), vec(s));
	EXPECT_EQ(vector<smth>({ 0, 1, 5 }), vec(p));
	persistent_set<smth> q(s);
	s.insert(smth(8));
	EXPECT_EQ(vector<smth>({ 1, 5 }), vec(q));
	q = s;
	EXPECT_EQ(vector<smth>({ 1, 5, 8 }), vec(s));
	EXPECT_EQ(vector<smth>({ 1, 5, 8 }), vec(q));
	s.erase(s.find(8));
	EXPECT_EQ(vector<smth>({ 1, 5 }), vec(s));
	EXPECT_EQ(vector<smth>({ 1, 5, 8 }), vec(q));
}

template < typename Set >
std::vector < std::vector < int > > test(std::string const& msg) {
	int tt = clock();
	srand(100);
	std::vector < Set > ss(256);
	for (int i = 0; i < 1000000; ++i) {
		int r = rand() & 7;
		if (r == 0) {
			int j = rand() & 255;
			ss[j] = ss[rand() & 255];
		} else if (r <= 3) {
			int j = rand() & 255;
			ss[j].insert(rand() & 16383);
		} else {
			int j = rand() & 255;
			auto it = ss[j].find(rand() & 16383);
			if (it != ss[j].end())
				ss[j].erase(it);
		}
	}
	std::vector < std::vector < int > > a(256);
	for (int i = 0; i < 256; ++i)
		for (int x : ss[i])
			a[i].push_back(x);
	std::cout << msg << " " << 1.*(clock() - tt) / CLOCKS_PER_SEC << "\n";
	return a;
}

template < typename Set >
std::vector < std::vector < int > > test2(std::string const& msg) {
	int tt = clock();
	int seed = time(0);
	srand(seed);
	std::cerr << "seed = " << seed << "\n";
	int num_of_sets = 1;
	int max_value = 15;
	int distr_k = 7;
	std::vector < Set > ss(num_of_sets);
	for (int i = 0; i < 87; ++i) {
		int r = rand() & distr_k;
		if (r == 0) {
			int j = rand() % num_of_sets;
			int l = rand() % num_of_sets;
			// cerr << "copy " << l << " to " << j << "\n";
			// ss[j] = ss[l];
		} else if (r <= 3) {
			int j = rand() % num_of_sets;
			int v = rand() % max_value;
			cerr << "insert " << v << " in " << j << "\n";
			ss[j].insert(v);
		} else {
			int v = rand() % max_value;
			int j = rand() % num_of_sets;
			cerr << "erase " << v << " from " << j << "\n";
			auto it = ss[j].find(v);
			if (it != ss[j].end())
				ss[j].erase(it);
		}
	}
	// ss[0].out();
	std::vector < std::vector < int > > a(num_of_sets);
	for (int i = 0; i < num_of_sets; ++i)
		for (int x : ss[i])
			a[i].push_back(x);
	std::cout << msg << " " << 1.*(clock() - tt) / CLOCKS_PER_SEC << "\n";
	return a;
}


TEST(test_set, random) {
	auto r1 = test<persistent_set<int, shared_ptr>>("shared_ptr");
	auto r2 = test<persistent_set<int, linked_ptr>>("linked_ptr");
	EXPECT_EQ(r1, r2);
	auto r3 = test<set<int>>("std::set  ");
	EXPECT_EQ(r2, r3);
}
