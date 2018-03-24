#ifndef VARIANT_H
#define VARIANT_H

#include "variant_ext.h"

template <typename T, bool = std::is_literal_type<T>::value>
struct container;

template <typename T>
struct container<T, true> // preserving constexpressness
{
private:
    T data;

public:
    template <typename... As>
    constexpr container(As&&... args)
        : data { std::forward<As>(args)... }
    { };

    constexpr T& get() &
    {
        return data;
    }

    constexpr const T& get() const &
    {
        return data;
    }

    constexpr T&& get() &&
    {
        return std::move(data);
    }

    constexpr const T&& get() const &&
    {
        return std::move(data);
    }
};

template <typename T>
struct container<T, false>
{
private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type data;

public:
    template <typename... As>
    container(As&&... args)
    {
        new(&data) T { std::forward<As>(args)... };
    };

    T& get() &
    {
        return *reinterpret_cast<T*>(&data);
    }

    const T& get() const &
    {
        return *reinterpret_cast<const T*>(&data);
    }

    T&& get() &&
    {
        return std::move(*reinterpret_cast<T*>(&data));
    }

    const T&& get() const &&
    {
        return std::move(*reinterpret_cast<const T*>(&data));
    }
};


template <typename... Ts>
union variadic_union
{
    variadic_union& operator=(const variadic_union& other) = default;
    variadic_union& operator=(variadic_union&& other) = default;
    template <size_t N, typename A>
    void construct(in_place_index_t<N>, A&& t) { };
    void destruct(size_t)
    { }
};

template <typename T, typename... Ts>
union variadic_union<T, Ts...>
{
private:
    container<T> that;
    variadic_union<Ts...> others;

public:
    constexpr variadic_union() { };
    variadic_union(const variadic_union&) = default;
    variadic_union(variadic_union&&) = default;

    variadic_union& operator=(const variadic_union& other) // = default doesn't work (probably compiler bug?)
    {
        that   = other.that;
        others = other.others;

        return *this;
    };
    variadic_union& operator=(variadic_union&& other)
    {
        that   = std::move(other.that);
        others = std::move(other.others);

        return *this;
    }

    template <typename... As>
    constexpr variadic_union(in_place_index_t<0>, As&&... args)
        : that { std::forward<As>(args)... }
    { }

    template <size_t N, typename... As>
    constexpr variadic_union(in_place_index_t<N>, As&&... args)
        : others { in_place_index_t<N - 1>(), std::forward<As>(args)... }
    { }

    constexpr decltype(auto) get(in_place_index_t<0>)
    {
        return that.get();
    }

    template <size_t N>
    constexpr decltype(auto) get(in_place_index_t<N>)
    {
        return others.get(in_place_index_t<N - 1>());
    }

    constexpr decltype(auto) get(in_place_index_t<0>) const
    {
        return that.get();
    }

    template <size_t N>
    constexpr decltype(auto) get(in_place_index_t<N>) const
    {
        return others.get(in_place_index_t<N - 1>());
    }

    template <size_t N, typename A>
    void construct(in_place_index_t<N>, A&& t)
    {
        new(&get(in_place_index_t<N>())) container<std::decay_t<A>>(std::forward<A>(t));
    }

    void destruct(size_t index)
    {
        if (index == 0) {
            that.get().~T();
        }
        else {
            others.destruct(index - 1);
        }
    }
};

template <typename... Ts>
struct union_struct
{
    variadic_union<Ts...> data;

    constexpr union_struct() = default;
    union_struct(const union_struct& other)  = default;
    union_struct(union_struct&& other)      = default;
    union_struct& operator=(const union_struct& other) = default;
    union_struct& operator=(union_struct&& other) = default;

    template <size_t N, typename... As>
    constexpr union_struct(in_place_index_t<N>, As&&... args)
        : data { in_place_index_t<N>(), std::forward<As>(args)... }
    { }

    template <size_t N>
    constexpr decltype(auto) get(in_place_index_t<N>)
    {
        return data.get(in_place_index_t<N>());
    }

    template <size_t N>
    constexpr decltype(auto) get(in_place_index_t<N>) const
    {
        return data.get(in_place_index_t<N>());
    }

    template <size_t N, typename T>
    void construct(in_place_index_t<N>, T&& t)
    {
        data.construct(in_place_index_t<N>(), std::forward<T>(t));
    }

    void destruct(size_t index)
    {
        data.destruct(index);
    }
};

template <bool trivally_destructible, typename... Ts>
struct destructible_union;

template <typename... Ts>
struct destructible_union<true, Ts...>  : union_struct<Ts...>
{
    size_t index;

    constexpr destructible_union() = default;
    destructible_union(const destructible_union& other) = default;
    destructible_union(destructible_union&& other)      = default;
    destructible_union& operator=(const destructible_union& other) = default;
    destructible_union& operator=(destructible_union&& other) = default;

    template <size_t N, typename... As>
    constexpr destructible_union(in_place_index_t<N>, As&&... args)
        : union_struct<Ts...> { in_place_index_t<N>(), std::forward<As>(args)... }
        , index { N }
    { }

    template <size_t N>
    constexpr decltype(auto) get(in_place_index_t<N>)
    {
        if (N != index) {
            throw bad_variant_access();
        }

        return union_struct<Ts...>::get(in_place_index_t<N>());
    }

    template <size_t N>
    constexpr decltype(auto) get(in_place_index_t<N>) const
    {
        if (N != index) {
            throw bad_variant_access();
        }

        return union_struct<Ts...>::get(in_place_index_t<N>());
    }

    void set_index(size_t index) noexcept
    {
        this->index = index;
    }

    constexpr size_t get_index() const
    {
        return index;
    }

    void reset_index() noexcept
    {
        index = variant_npos;
    }
};

template <typename... Ts>
struct destructible_union<false, Ts...> : union_struct<Ts...>
{
    size_t index;

    constexpr destructible_union() = default;
    destructible_union(const destructible_union& other) = default;
    destructible_union(destructible_union&& other)      = default;
    destructible_union& operator=(const destructible_union& other) = default;
    destructible_union& operator=(destructible_union&& other) = default;

    template <size_t N, typename... As>
    constexpr destructible_union(in_place_index_t<N>, As&&... args)
        : union_struct<Ts...> { in_place_index_t<N>(), std::forward<As>(args)... }
        , index { N }
    { }

    constexpr size_t get_index() const
    {
        return index;
    }

    template <size_t N>
    constexpr decltype(auto) get(in_place_index_t<N>)
    {
        if (N != index) {
            throw bad_variant_access();
        }

        return union_struct<Ts...>::get(in_place_index_t<N>());
    }

    template <size_t N>
    constexpr decltype(auto) get(in_place_index_t<N>) const
    {
        if (N != index) {
            throw bad_variant_access();
        }

        return union_struct<Ts...>::get(in_place_index_t<N>());
    }

    void set_index(size_t index) noexcept
    {
        this->index = index;
    }

    void reset_index() noexcept
    {
        index = variant_npos;
    }

    ~destructible_union()
    {
        this->destruct(index);
    }
};

template <typename... Ts>
using DestructibleUnion = destructible_union<
    (std::is_trivially_destructible_v<Ts> && ...),
    Ts...
    >;

template <typename... Ts>
struct simple_variant : DestructibleUnion<Ts...>
{
    constexpr simple_variant()
        : DestructibleUnion<Ts...> { in_place_index_t<0>() }
    { }

    template <size_t N, typename... As>
    constexpr simple_variant(in_place_index_t<N>, As&&... args)
        : DestructibleUnion<Ts...> { in_place_index_t<N>(), std::forward<As>(args)... }
    { }

    simple_variant(const simple_variant& other) noexcept
    {
        this->set_index(other.index());
        visit([this](auto&& arg) {
            constexpr size_t j = get_index<std::decay_t<decltype(arg)>, Ts...>();

            this->construct(in_place_index_t<j>(), arg);
        }, other);
    }
    simple_variant(simple_variant&& other) noexcept
    {
        this->set_index(other.index());
        visit([this](auto&& arg) {
            constexpr size_t j = get_index<std::decay_t<decltype(arg)>, Ts...>();

            this->construct(in_place_index_t<j>(), std::move(arg));
        }, other);
    }

    ~simple_variant() = default;

    simple_variant& operator=(const simple_variant& rhs)
    {
        if (valueless_by_exception() && rhs.valueless_by_exception())
            return *this;
        if (rhs.valueless_by_exception() && !valueless_by_exception()) {
            this->destruct(index());
            this->reset_index();
            return *this;
        }
        if (index() == rhs.index()) {
            DestructibleUnion<Ts...>::operator=(rhs);
            return *this;
        }
        if (visit([] (auto& arg) -> bool {
                return (std::is_nothrow_copy_constructible_v<decltype(arg)> &&
                       std::is_nothrow_move_constructible_v<decltype(arg)>);
            }, rhs)) {
                simple_variant<Ts...> tmp { rhs };
                this->destruct(index());
                try {
                    new(this) simple_variant<Ts...> { std::move(tmp) };
                }
                catch (...) {
                    this->reset_index();
                }
                return *this;
        }
        else {
            this->operator=(simple_variant(rhs));
            return *this;
        }


        return *this;
    }

    simple_variant& operator=(simple_variant&& rhs) noexcept((
        (std::is_nothrow_move_constructible_v<Ts>  &&
            std::is_nothrow_move_assignable_v<Ts>) && ...)
        )
    {
        if (valueless_by_exception() && rhs.valueless_by_exception())
            return *this;
        if (rhs.valueless_by_exception() && !valueless_by_exception()) {
            this->destruct(index());
            this->reset_index();
            return *this;
        }
        if (index() == rhs.index()) {
            DestructibleUnion<Ts...>::operator=(std::move(rhs));
            return *this;
        }

        simple_variant<Ts...> tmp { std::move(rhs) };
        this->destruct(index());
        try {
            new(this) simple_variant<Ts...> { std::move(tmp) };
        }
        catch (...) {
            this->reset_index();
        }

        return *this;
    }

    constexpr size_t index() const noexcept
    {
        return this->get_index();
    }

    constexpr bool valueless_by_exception() const noexcept
    {
        return index() == variant_npos;
    }

};

template <typename T_0, typename... Ts>
struct variant
    : enable_default_constructor<
        std::is_default_constructible<T_0>::value,
        noexcept(std::is_nothrow_default_constructible<T_0>::value)
        >
    , enable_copy_constructor<
        std::is_copy_constructible_v<T_0>     &&
            (std::is_copy_constructible_v<Ts> && ...)
        >
    , enable_move_constructor<
        std::is_move_constructible_v<T_0>     &&
            (std::is_move_constructible_v<Ts> && ...)
        >
    , enable_copy_assignment<
        std::is_copy_constructible_v<T_0>     &&
            (std::is_copy_constructible_v<Ts> && ...) &&
            std::is_copy_assignable_v<T_0>    &&
            (std::is_copy_assignable_v<Ts>    && ...)
        >
    , enable_move_assignment<
        std::is_move_constructible_v<T_0>     &&
            (std::is_move_constructible_v<Ts> && ...) &&
            std::is_move_assignable_v<T_0>    &&
            (std::is_move_assignable_v<Ts>    && ...)
        >
    , simple_variant<T_0, Ts...>
{
private:
    using default_constructor_t = enable_default_constructor<
        std::is_default_constructible<T_0>::value,
        noexcept(std::is_nothrow_default_constructible<T_0>::value)
        >;

public:
    /* -------------------[Constructors and Destructor]------------------- */
    constexpr variant() = default;
    variant(const variant& other) = default;
    variant(variant&& other) noexcept(
        std::is_nothrow_move_constructible_v<T_0> &&
        (std::is_nothrow_move_constructible_v<Ts> && ...)
        ) = default;

    template <
        typename T,
        typename U = best_match<T&&, T_0, Ts...>,
        typename   = std::enable_if_t<std::is_constructible<U, T>::value, U>
        >
    constexpr variant(T&& t) noexcept(std::is_nothrow_constructible<U, T>())
        : simple_variant<T_0, Ts...> (
            in_place_index_t<get_index<U, T_0, Ts...>()>(),
            std::forward<T>(t)
            )
    { }

    template <class T, class... As, typename = std::enable_if_t<std::is_constructible_v<T, As...>>>
    constexpr explicit variant(in_place_type_t<T>, As&&... args)
        : default_constructor_t { 1 }
        , simple_variant<T_0, Ts...> (
            in_place_index_t<get_index<T, T_0, Ts...>()>(),
            std::forward<As>(args)...
            )
    { }

    template <size_t I, class... As>
    constexpr explicit variant(in_place_index_t<I>, As&&... args)
        : default_constructor_t { 1 }
        , simple_variant<T_0, Ts...> {
            in_place_index_t<I>(),
            std::forward<As>(args)...
            }
    { }

    template <typename T, typename U, typename... As>
    constexpr explicit variant(in_place_type_t<T>,
                               std::initializer_list<U> il, As&&... args)
       : default_constructor_t { 1 }
       , simple_variant<T_0, Ts...> {
           in_place_index_t<get_index<T, T_0, Ts...>()>(),
           il,
           std::forward<As>(args)...
           }
    { }

    template <std::size_t I, typename U, typename... As>
    constexpr explicit variant(in_place_index_t<I>,
                               std::initializer_list<U> il, As&&... args)
       : default_constructor_t { 1 }
       , simple_variant<T_0, Ts...> {
           in_place_index_t<I>(),
           il,
           std::forward<As>(args)...
           }
    { }

    ~variant() = default;
    /* -------------------[Constructors and Destructor]------------------- */

    /* -------------------[Assignment operators]------------------- */
    variant& operator=(const variant& rhs) = default;
    variant& operator=(variant&& rhs)      = default;

    template <
        typename T,
        typename U = best_match<T&&, T_0, Ts...>,
        typename   = std::enable_if_t<
            std::is_assignable<U&, T>  ::value &&
            std::is_constructible<U, T>::value,
            U
            >
        >
    variant& operator=(T&& t) noexcept(
        std::is_nothrow_assignable_v<U&, T> &&
        std::is_nothrow_constructible_v<U, T>)
    {
        constexpr size_t j = get_index<U, T_0, Ts...>();
        if (j == index()) {
            get<j>(*this) = std::forward<T>(t);
        }
        else {
            if (std::is_nothrow_constructible_v<U, T> ||
                !std::is_nothrow_move_constructible_v<U>) {
                    this->emplace<j>(std::forward<T>(t));
            }
            else {
                this->operator=(variant(std::forward<T>(t)));
            }

        }
        return *this;
    }
    /* -------------------[Assignment operators]-------------------*/

    /* -------------------[Observers]------------------- */

    using simple_variant<T_0, Ts...>::index;
    using simple_variant<T_0, Ts...>::valueless_by_exception;

    /* -------------------[Observers]------------------- */

    /* -------------------[Modifiers]------------------- */
    template <class T, class... As>
    T& emplace(As&&... args)
    {
        return emplace<get_index<T, T_0, Ts...>()>(std::forward<As>(args)...);
    }

    template <size_t I, class... As>
    variant_alternative_t<I, variant>& emplace(As&&... args)
    {
        try {
            if (!valueless_by_exception()) {
                this->destruct(index());
            }
            new(this) variant(in_place_index_t<I>(), std::forward<As>(args)...);
        }
        catch (...) {
            this->reset_index();
        }

        return get<I>(*this);
    }

    void swap(variant& rhs) noexcept(
        (std::is_nothrow_move_constructible_v<Ts> && ...) &&
        (std::is_nothrow_swappable_v<Ts> && ...)
        )
    {
        if (valueless_by_exception() && rhs.valueless_by_exception())
            return;

        if (index() == rhs.index()) {
            visit([](auto& arg_1, auto& arg_2) constexpr {
                if constexpr(std::is_same_v<decltype(arg_1), decltype(arg_2)>) {
                    using std::swap;
                    swap(arg_1, arg_2);
                }
            }, *this, rhs);
        }
        else {
            variant tmp { std::move(*this) };
            operator=(std::move(rhs));
            rhs = std::move(tmp);
        }
    }

    /* -------------------[Modifiers]------------------- */

    /* -------------------[Friends]------------------- */
    template <size_t I, class... As>
    friend constexpr variant_alternative_t<I, variant<As...>>&
    get(variant<As...>& v);

    template <size_t I, class... As>
    friend constexpr variant_alternative_t<I, variant<As...>>&&
    get(variant<As...>&& v);

    template <size_t I, class... As>
    friend constexpr variant_alternative_t<I, variant<As...>> const&
    get(const variant<As...>& v);

    template <size_t I, class... As>
    friend constexpr variant_alternative_t<I, variant<As...>> const&&
    get(const variant<As...>&& v);
    /* -------------------[Friends]------------------- */

};

#endif  // VARIANT_H
