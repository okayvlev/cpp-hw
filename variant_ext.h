#ifndef VARIANT_EXT_H
#define VARIANT_EXT_H

#include "variant.h"
#include "util.h"


struct monostate
{ };

struct in_place_t
{
    explicit in_place_t() = default;
};

constexpr in_place_t in_place { };

template <class T>
struct in_place_type_t
{
    explicit in_place_type_t() = default;
};

template <class T>
constexpr in_place_type_t<T> in_place_type { };

template <size_t I>
struct in_place_index_t
{
    explicit in_place_index_t() = default;
};

template <size_t I>
constexpr in_place_index_t<I> in_place_index { };


constexpr bool operator< (monostate, monostate) noexcept { return false; }
constexpr bool operator> (monostate, monostate) noexcept { return false; }
constexpr bool operator<=(monostate, monostate) noexcept { return true;  }
constexpr bool operator>=(monostate, monostate) noexcept { return true;  }
constexpr bool operator==(monostate, monostate) noexcept { return true;  }
constexpr bool operator!=(monostate, monostate) noexcept { return false; }


struct bad_variant_access : public std::exception
{
    bad_variant_access() noexcept = default;
    ~bad_variant_access()         = default;

    const char* what() const noexcept override
    {
        return "bad_variant_access";
    }
};


template <size_t I, class T>
struct variant_alternative;

template <size_t I, class T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

template <size_t I, typename... Ts>
struct variant_alternative<I, variant<Ts...>>
{
    using type = get_type_t<I, Ts...>;
};

template <size_t I, class T> class variant_alternative<I, const T>
{
    using type = std::add_const_t<variant_alternative<I, T>>;
};

template <size_t I, class T> class variant_alternative<I, volatile T>
{
    using type = std::add_volatile_t<variant_alternative<I, T>>;
};

template <size_t I, class T> class variant_alternative<I, const volatile T>
{
    using type = std::add_cv_t<variant_alternative<I, T>>;
};


template <class T>
struct variant_size;

template <typename... Ts>
struct variant_size<variant<Ts...>>
    : std::integral_constant<size_t, sizeof...(Ts)>
{ };

template <class T>
class variant_size<const T> : variant_size<T>
{ };

template <class T>
class variant_size<volatile T> : variant_size<T>
{ };

template <class T>
class variant_size<const volatile T> : variant_size<T>
{ };

template <class T>
constexpr size_t variant_size_v = variant_size<T>::value;


template <class T, class... Ts, typename = std::enable_if_t<is_unique<T, Ts...>()>>
constexpr bool holds_alternative(const variant<Ts...>& v) noexcept
{
    return v.valueless_by_exception() ? false : v.index() == get_index<T, Ts...>();
}

template <size_t I, class... Ts>
constexpr variant_alternative_t<I, variant<Ts...>>&
get(variant<Ts...>& v)
{
    return v.get(in_place_index_t<I>());
}

template <size_t I, class... Ts>
constexpr variant_alternative_t<I, variant<Ts...>>&&
get(variant<Ts...>&& v)
{
    return std::move(v).get(in_place_index_t<I>());
}

template <size_t I, class... Ts>
constexpr variant_alternative_t<I, variant<Ts...>> const&
get(const variant<Ts...>& v)
{
    return v.get(in_place_index_t<I>());
}

template <size_t I, class... Ts>
constexpr variant_alternative_t<I, variant<Ts...>> const&&
get(const variant<Ts...>&& v)
{
    return std::move(v).get(in_place_index_t<I>());
}

template <class T, class... Ts>
constexpr T& get(variant<Ts...>& v)
{
    return get<get_index<T, Ts...>()>(std::forward<variant<Ts...>&>(v));
}

template <class T, class... Ts>
constexpr T&& get(variant<Ts...>&& v)
{
    return get<get_index<T, Ts...>()>(std::forward<variant<Ts...>&&>(v));
}

template <class T, class... Ts>
constexpr const T& get(const variant<Ts...>& v)
{
    return get<get_index<T, Ts...>()>(std::forward<const variant<Ts...>&>(v));
}

template <class T, class... Ts>
constexpr const T&& get(const variant<Ts...>&& v)
{
    return get<get_index<T, Ts...>()>(std::forward<const variant<Ts...>&&>(v));
}

template <class T, class... Ts>
constexpr std::add_pointer_t<T> get_if(variant<Ts...>* pv) noexcept
{
    return (pv != nullptr && get_index<T, Ts...>() == pv->index())
            ? &get<T>(*pv)
            : nullptr;
}

template <class T, class... Ts>
constexpr std::add_pointer_t<const T> get_if(const variant<Ts...>* pv) noexcept
{
    return (pv != nullptr && get_index<T, Ts...>() == pv->index())
            ? &get<T>(*pv)
            : nullptr;
}

template <class... Ts>
constexpr bool operator==(const variant<Ts...>& v, const variant<Ts...>& w)
{
    if (v.index() != w.index()) {
        return false;
    }

    if (v.valueless_by_exception()) {
        return true;
    }

    return visit([](const auto& arg_1, const auto& arg_2) constexpr -> bool {
        if constexpr(std::is_same_v<decltype(arg_1), decltype(arg_2)>)
            return (arg_1 == arg_2);
        else
            return false;
        }, v, w);
}

template <class... Ts>
constexpr bool operator!=(const variant<Ts...>& v, const variant<Ts...>& w)
{
    return !(v == w);
}

template <class... Ts>
constexpr bool operator< (const variant<Ts...>& v, const variant<Ts...>& w)
{
    if (v.index() != w.index()) {
        return false;
    }

    if (v.valueless_by_exception()) {
        return true;
    }

    if (v.index() != w.index()) {
        return v.index() < w.index();
    }

    return visit([](const auto& arg_1, const auto& arg_2) constexpr -> bool {
        if constexpr(std::is_same_v<decltype(arg_1), decltype(arg_2)>)
            return (arg_1 < arg_2);
        else
            return false;
        }, v, w);
}

template <class... Ts>
constexpr bool operator> (const variant<Ts...>& v, const variant<Ts...>& w)
{
    if (v.index() != w.index()) {
        return false;
    }

    if (v.valueless_by_exception()) {
        return true;
    }

    if (v.index() != w.index()) {
        return v.index() > w.index();
    }

    return visit([](const auto& arg_1, const auto& arg_2) constexpr -> bool {
        if constexpr(std::is_same_v<decltype(arg_1), decltype(arg_2)>)
            return (arg_1 > arg_2);
        else
            return false;
        }, v, w);
}

template <class... Ts>
constexpr bool operator<=(const variant<Ts...>& v, const variant<Ts...>& w)
{
    return !(v > w);
}

template <class... Ts>
constexpr bool operator>=(const variant<Ts...>& v, const variant<Ts...>& w)
{
    return !(v < w);
}

template <
    class... Ts,
    typename = std::enable_if_t<
        (std::is_move_constructible_v<Ts> && ...) &&
        (std::is_swappable_v<Ts> && ...)
        >
    >
void swap(variant<Ts...>& lhs, variant<Ts...>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}


template <typename F, typename... Vs, size_t... Is>
constexpr auto make_matrix(std::index_sequence<Is...>) {
    struct callee_impl
    {
        static constexpr decltype(auto) callee(F&& f, Vs&&... vars)
        {
            return std::invoke(
                std::forward<F>(f),
                get<Is>(std::forward<Vs>(vars))...
                );
        }
    };

    return callee_impl::callee;
}

template <
    typename    F,
    typename... Vs,
    size_t...   Prevs,  // current "path" of indices
    size_t...   Curs,   // seq of current indices
    typename... Others  // seq of indices for each next dimension
    >
constexpr decltype(auto) make_matrix(
    std::index_sequence<Prevs...>,
    std::index_sequence<Curs...>,
    Others... others
    )
{
    return make_array(
        make_matrix<F, Vs...>(std::index_sequence<Prevs..., Curs>(), others...)...
        );
}

template <typename F, typename... Vs>
constexpr decltype(auto) make_vt_matrix()
{
    return make_matrix<F, Vs...>(
        std::index_sequence<>(),
        std::make_index_sequence<variant_size_v<std::decay_t<Vs>>>()...
        );
}

template <typename T>
constexpr const T& get_matrix(const T& t)
{
    return t;
}

template <typename T, typename... Is>
constexpr decltype(auto) get_matrix(const T& matrix, size_t cur, Is... is)
{
    return get_matrix(matrix[cur], is...);
}

template <class F, class... Vs>
constexpr decltype(auto) visit(F&& f, Vs&&... vars)
{
    constexpr auto vt_matrix = make_vt_matrix<F&&, Vs&&...>();

    return (get_matrix(vt_matrix, vars.index()...))(
        std::forward<F>(f),
        std::forward<Vs>(vars)...
    );
}

#endif
