#ifndef BIND_H
#define BIND_H

#include <tuple>

template <int N>
struct placeholder
{ };

constexpr placeholder<1> _1;
constexpr placeholder<2> _2;
constexpr placeholder<3> _3;

template <bool once, typename F, typename... As>
struct bind_t;

template <typename A, typename... Ts>
struct type_count
{
	static constexpr size_t value = 0;
};

template <typename A, typename T_0, typename... Ts>
struct type_count<A, T_0, Ts...>
{
	static constexpr size_t value = (type_count<A, Ts...>::value)
								  +  std::is_same<A, T_0>::value;
};

template <typename A, bool once, typename F, typename... As, typename... Ts>
struct type_count<A, bind_t<once, F, As...>, Ts...>
{
	static constexpr size_t value = type_count<A, As...>::value
								  + type_count<A, Ts...>::value;
};

template <typename T, bool>
struct ref_check
{ };

template <typename T>
struct ref_check<T, true>
{
	using type = T&&;
};

template <typename T>
struct ref_check<T, false>
{
	using type = T&;
};

template <typename T, int i, typename... Ts>
using unique_guard_t =
	typename ref_check<T, type_count<const placeholder<i + 1>&, Ts...>::value == 1>::type;

template <bool once, typename A>
struct G
{
    G(A&& a)
        : a(std::forward<A>(a))
    { }

    template <typename... Bs>
    decltype(auto) operator()(Bs&&...)
    {
        return static_cast<typename ref_check<std::decay_t<A>, once>::type>(a);
    }

    std::decay_t<A> a;
};

template <bool once>
struct G<once, const placeholder<1>&>
{
    G(const placeholder<1>&)
    { }

    template <typename B1, typename... Bs>
    decltype(auto) operator()(B1&& b1, Bs&&...)
    {
        return std::forward<B1>(b1);
    }
};

template <bool once, int N>
struct G<once, const placeholder<N>&>
{
    G(const placeholder<N>&)
    { }

    template <typename B, typename... Bs>
    decltype(auto) operator()(B&&, Bs&&... bs)
    {
        G<once, const placeholder<N - 1>&> next((placeholder<N - 1>()));
        return next(std::forward<Bs>(bs)...);
    }
};

template <bool once, bool d_once, typename F, typename... As>
struct G<once, bind_t<d_once, F, As...>>
{
    G(bind_t<d_once, F, As...>&& f)
        : f(std::forward<bind_t<d_once, F, As...>>(f))
    { }

    template <typename... Bs>
    decltype(auto) operator()(Bs&&... bs)
    {
        return f(std::forward<Bs>(bs)...);
    }

    bind_t<d_once, F, As...> f;
};

template <bool once, typename F, typename... As>
struct bind_t
{
    bind_t(F&& f, As&&... as)
        : f(std::forward<F>(f))
        , gs(std::forward<As>(as)...)
    { }

    template <typename... Bs>
    decltype(auto) operator()(Bs&&... bs)
    {
        return call(
			std::make_integer_sequence<int, sizeof...(As)>(),
			std::make_integer_sequence<int, sizeof...(Bs)>(),
			std::forward<Bs>(bs)...
			);
    }

private:
    template <int... ks, int... is, typename... Bs>
    decltype(auto) call(
		std::integer_sequence<int, ks...>,
		std::integer_sequence<int, is...>, // iterating and checking for placeholder uniqueness
		Bs&&... bs
		)
    {
        return f(std::get<ks>(gs)(unique_guard_t<Bs, is, As...>(bs)...)...);
    }

	std::decay_t<F> f;
    std::tuple<G<once, As>...> gs;
};

#define BIND_FUNC(name, bb) 													\
template <typename F, typename... As>											\
decltype(auto) name(F&& f, As&&... as)											\
{																				\
    return bind_t<bb, F, As...>(std::forward<F>(f), std::forward<As>(as)...);	\
}																				\

BIND_FUNC(bind, false)
BIND_FUNC(call_once_bind, true)
#undef BIND_FUNC

#endif	// BIND_H
