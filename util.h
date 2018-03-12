#ifndef UTIL_H
#define UTIL_H

#include <functional>

constexpr size_t variant_npos = -1;

template <typename T_0, typename... Ts>
struct variant;

template <typename... Ts>
struct overload;

template <size_t... Ts>
struct get_max;

template <size_t I, typename... Ts>
struct get_type;

template <size_t I, typename... Ts>
using get_type_t = typename get_type<I, Ts...>::type;


template <>
struct overload<>
{
    void operator()() const;
};

template <typename T, typename... Ts>
struct overload<T, Ts...> : overload<Ts...>
{
    using overload<Ts...>::operator();
    T operator()(T) const;
};


template <size_t T, size_t... Ts>
struct get_max<T, Ts...>
{
    static constexpr size_t value = std::max(T, get_max<Ts...>::value);
};

template <size_t T>
struct get_max<T>
{
    static constexpr size_t value = T;
};

template <typename... Ts>
constexpr size_t get_max_size()
{
    return get_max<sizeof(Ts)...>::value;
}

template <typename... Ts>
constexpr size_t get_max_align()
{
    return get_max<alignof(Ts)...>::value;
}

template <typename T, typename... Ts>
constexpr size_t get_count()
{
    size_t count = 0;
    constexpr bool matches[] = { std::is_same<T, Ts>::value... };

    for (bool match : matches) {
        if (match) {
            ++count;
        }
    }

    return count;
}

template <typename T, typename... Ts>
constexpr size_t get_index()
{
    constexpr bool matches[] = { std::is_same<T, Ts>::value... };

    for (size_t i = 0; i < sizeof(matches); ++i) {
        if (matches[i]) {
            return i;
        }
    }

    return variant_npos;
}

template <typename T, typename... Ts>
struct get_type<0, T, Ts...>
{
    using type = T;
};

template <size_t I, typename T, typename... Ts>
struct get_type<I, T, Ts...>
{
    using type = typename get_type<I - 1, Ts...>::type;
};


template <typename T, typename... Ts>
constexpr bool is_unique()
{
    return get_count<T, Ts...>() == 1;
}


template <typename T, typename... Ts>
using best_match_once = std::enable_if_t<is_unique<T, Ts...>(), T>;

template <typename T, typename... Ts>   // TODO decltype?
using best_match = best_match_once<std::result_of_t<overload<Ts...>(T)>, Ts...>;

template <bool, bool NOEXCEPT>
struct enable_default_constructor;

template <bool NOEXCEPT>
struct enable_default_constructor<true, NOEXCEPT>
{
    constexpr enable_default_constructor(bool) { }; // mirroring deleted constructor
    constexpr enable_default_constructor() noexcept(NOEXCEPT) = default;
    constexpr enable_default_constructor(const enable_default_constructor& other)          = default;
    constexpr enable_default_constructor(enable_default_constructor&& other)               = default;
    constexpr enable_default_constructor& operator=(const enable_default_constructor& rhs) = default;
    constexpr enable_default_constructor& operator=(enable_default_constructor&& rhs)      = default;
};

template <bool NOEXCEPT>
struct enable_default_constructor<false, NOEXCEPT>
{
    constexpr enable_default_constructor(bool) { }; // need something to call instead
    constexpr enable_default_constructor() noexcept(NOEXCEPT) = delete;
    constexpr enable_default_constructor(const enable_default_constructor& other)          = default;
    constexpr enable_default_constructor(enable_default_constructor&& other)               = default;
    constexpr enable_default_constructor& operator=(const enable_default_constructor& rhs) = default;
    constexpr enable_default_constructor& operator=(enable_default_constructor&& rhs)      = default;
};

template <bool>
struct enable_copy_constructor;

template <>
struct enable_copy_constructor<true>
{
    constexpr enable_copy_constructor() = default;
    constexpr enable_copy_constructor(const enable_copy_constructor& other)          = default;
    constexpr enable_copy_constructor(enable_copy_constructor&& other)               = default;
    constexpr enable_copy_constructor& operator=(const enable_copy_constructor& rhs) = default;
    constexpr enable_copy_constructor& operator=(enable_copy_constructor&& rhs)      = default;
};

template <>
struct enable_copy_constructor<false>
{
    constexpr enable_copy_constructor() = default;
    constexpr enable_copy_constructor(const enable_copy_constructor& other)          = delete;
    constexpr enable_copy_constructor(enable_copy_constructor&& other)               = default;
    constexpr enable_copy_constructor& operator=(const enable_copy_constructor& rhs) = default;
    constexpr enable_copy_constructor& operator=(enable_copy_constructor&& rhs)      = default;
};

template <bool>
struct enable_move_constructor;

template <>
struct enable_move_constructor<true>
{
    constexpr enable_move_constructor() = default;
    constexpr enable_move_constructor(const enable_move_constructor& other)          = default;
    constexpr enable_move_constructor(enable_move_constructor&& other)               = default;
    constexpr enable_move_constructor& operator=(const enable_move_constructor& rhs) = default;
    constexpr enable_move_constructor& operator=(enable_move_constructor&& rhs)      = default;
};

template <>
struct enable_move_constructor<false>
{
    constexpr enable_move_constructor() = default;
    constexpr enable_move_constructor(const enable_move_constructor& other)          = default;
    constexpr enable_move_constructor(enable_move_constructor&& other)               = delete;
    constexpr enable_move_constructor& operator=(const enable_move_constructor& rhs) = default;
    constexpr enable_move_constructor& operator=(enable_move_constructor&& rhs)      = default;
};

template <bool>
struct enable_copy_assignment;

template <>
struct enable_copy_assignment<true>
{
    constexpr enable_copy_assignment() = default;
    constexpr enable_copy_assignment(const enable_copy_assignment& other)          = default;
    constexpr enable_copy_assignment(enable_copy_assignment&& other)               = default;
    constexpr enable_copy_assignment& operator=(const enable_copy_assignment& rhs) = default;
    constexpr enable_copy_assignment& operator=(enable_copy_assignment&& rhs)      = default;
};

template <>
struct enable_copy_assignment<false>
{
    constexpr enable_copy_assignment() = default;
    constexpr enable_copy_assignment(const enable_copy_assignment& other)          = default;
    constexpr enable_copy_assignment(enable_copy_assignment&& other)               = default;
    constexpr enable_copy_assignment& operator=(const enable_copy_assignment& rhs) = delete;
    constexpr enable_copy_assignment& operator=(enable_copy_assignment&& rhs)      = default;
};

template <bool>
struct enable_move_assignment;

template <>
struct enable_move_assignment<true>
{
    constexpr enable_move_assignment() = default;
    constexpr enable_move_assignment(const enable_move_assignment& other)          = default;
    constexpr enable_move_assignment(enable_move_assignment&& other)               = default;
    constexpr enable_move_assignment& operator=(const enable_move_assignment& rhs) = default;
    constexpr enable_move_assignment& operator=(enable_move_assignment&& rhs)      = default;
};

template <>
struct enable_move_assignment<false>
{
    constexpr enable_move_assignment() = default;
    constexpr enable_move_assignment(const enable_move_assignment& other)          = default;
    constexpr enable_move_assignment(enable_move_assignment&& other)               = default;
    constexpr enable_move_assignment& operator=(const enable_move_assignment& rhs) = default;
    constexpr enable_move_assignment& operator=(enable_move_assignment&& rhs)      = delete;
};

template <typename T, typename... As>
constexpr decltype(auto) make_array_impl(As&&... args)
{
    return std::array<T, sizeof...(As)> { std::forward<As>(args)... };
}

template <typename A_0, typename... As>
constexpr decltype(auto) make_array(A_0&& arg_0, As&&... args)
{
    return make_array_impl<A_0, A_0, As...>(std::forward<A_0>(arg_0), std::forward<As>(args)...);
}


#endif  // UTIL_H
