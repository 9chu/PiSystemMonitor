/**
* @file
* @author chu
* @date 2024/8/11
*/
#pragma once
#include <cassert>
#include <tuple>
#include <variant>
#include <system_error>

template <class T, class E = std::error_code>
class Result;

// <editor-fold desc="Result<T, E>">

namespace detail
{
    /**
     * 移除类型外部的 const volatile 等限定符
     */
    template <class T>
    using RemoveCVRef = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    /**
     * 检查类型 T... 是否等价于 std::errc
     */
    template <class... T>
    using IsErrorCodeEnum = std::is_same<std::tuple<RemoveCVRef<T>...>, std::tuple<std::errc>>;

    template <class T, class... A>
    struct IsConstructible :
        std::is_constructible<T, A...> {};

    template <class A>
    struct IsConstructible<bool, A> :
        std::is_convertible<A, bool> {};

    template <class A>
    struct IsConstructible<bool const, A> :
        std::is_convertible<A, bool> {};

    template <class U, class A>
    struct ReferenceToTemporary :
        std::integral_constant<bool, !std::is_reference_v<A> || !std::is_convertible_v<typename std::remove_reference<A>::type*, U*>>
    {};

    template <class T, class U>
    struct IsValueConvertibleTo :
        std::is_convertible<T, U> {};

    template <class T, class U>
    struct IsValueConvertibleTo<T, U&> :
        std::integral_constant<bool, std::is_lvalue_reference_v<T> &&
            std::is_convertible_v<typename std::remove_reference<T>::type*, U*>> {};

    template <class T>
    struct IsResult : std::false_type {};

    template <class T, class E>
    struct IsResult<Result<T, E>> : std::true_type {};
} // namespace detail

using InPlaceValueType = std::in_place_index_t<0>;
using InPlaceErrorType = std::in_place_index_t<1>;

/**
 * 通用 Result 类
 *
 * 适用于从函数返回结果或者错误码。
 *
 * @tparam T 值
 * @tparam E 错误类型
 */
template <class T, class E>
class Result
{
public:
    using ValueType = T;
    using ErrorType = E;

    static constexpr InPlaceValueType InPlaceValue {};
    static constexpr InPlaceErrorType InPlaceError {};

public:  // 构造函数
    /**
     * 默认构造函数
     * @pre 当且仅当类型 T 可以被默认构造
     */
    constexpr Result() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::is_default_constructible_v<T>
        : m_stValue(InPlaceValue)
    {}

    /**
     * 隐式构造
     * @tparam A 输入参数类型
     */
    template <class A = T>
    constexpr Result(A&& a) noexcept(std::is_nothrow_constructible_v<T, A>)
        requires (std::is_convertible_v<A, T> &&
            // 阻止从 err_t -> error_code 的转换造成二义性
            !(detail::IsErrorCodeEnum<A>::value && std::is_arithmetic_v<T>) &&
            // 阻止 error_code 作为参数
            !std::is_convertible_v<A, E>)
        : m_stValue(InPlaceValue, std::forward<A>(a))
    {}

    /**
     * 隐式构造（错误码）
     * @tparam A 错误类型
     */
    template <class A = E>
    constexpr Result(A&& a) noexcept(std::is_nothrow_constructible_v<E, A>)
        requires (std::is_convertible_v<A, E> && !std::is_convertible_v<A, T>)
        : m_stValue(InPlaceError, std::forward<A>(a))
    {}

    /**
     * 隐式构造（值）
     * @tparam A 参数类型
     */
    template <class... A>
    explicit constexpr Result(A&&... a) noexcept(std::is_nothrow_constructible_v<T, A...>)
        requires (detail::IsConstructible<T, A...>::value &&
            // 阻止从 err_t -> error_code 的转换造成二义性
            !(detail::IsErrorCodeEnum<A...>::value && std::is_arithmetic_v<T>) &&
            !detail::IsConstructible<E, A...>::value &&
            sizeof...(A) >= 1)
        : m_stValue(InPlaceValue, std::forward<A>(a)...)
    {}

    /**
     * 隐式构造（错误码）
     * @tparam A 参数类型
     */
    template <class... A>
    explicit constexpr Result(A&&... a) noexcept(std::is_nothrow_constructible_v<E, A...>)
        requires (!detail::IsConstructible<T, A...>::value && detail::IsConstructible<E, A...>::value && sizeof...(A) >= 1)
        : m_stValue(InPlaceError, std::forward<A>(a)...)
    {}

    /**
     * 加上标签进行构造（值）
     * @tparam A 参数类型
     * @param a 参数
     */
    template <class... A>
    constexpr Result(InPlaceValueType, A&&... a) noexcept(std::is_nothrow_constructible_v<T, A...>)
        requires (std::is_constructible_v<T, A...>)
        : m_stValue(InPlaceValue, std::forward<A>(a)...)
    {}

    /**
     * 加上标签进行构造（错误）
     * @tparam A 参数类型
     */
    template <class... A>
    constexpr Result(InPlaceErrorType, A&&... a) noexcept(std::is_nothrow_constructible_v<E, A...>)
        requires (std::is_constructible_v<E, A...>)
        : m_stValue(InPlaceError, std::forward<A>(a)...)
    {}

    /**
     * 从一个错误类型 Result<T, E> 转换到当前类型
     * @tparam T2 另一个错误存储的值类型
     * @tparam E2 另一个错误存储的错误类型
     * @param r2 另一个错误实例
     */
    template <class T2, class E2>
    constexpr Result(const Result<T2, E2>& r2) noexcept(
        std::is_nothrow_constructible_v<T, T2 const&> &&
        std::is_nothrow_constructible_v<E, E2> &&
        std::is_nothrow_default_constructible_v<E2> &&
        std::is_nothrow_copy_constructible_v<E2>)
        requires (std::is_convertible_v<T2, T> && std::is_convertible_v<E2, E> &&
            !std::is_convertible_v<Result<T2, E2> const&, T>)
        : m_stValue(InPlaceError, r2.GetError())
    {
        if (r2)
            m_stValue.template emplace<0>(*r2);
    }

    template <class T2, class E2>
    constexpr Result(Result<T2, E2>&& r2) noexcept(
        std::is_nothrow_constructible_v<T, T2&&> &&
        std::is_nothrow_constructible_v<E, E2> &&
        std::is_nothrow_default_constructible_v<E2> &&
        std::is_nothrow_copy_constructible_v<E2>)
        requires (std::is_convertible_v<T2, T> && std::is_convertible_v<E2, E> &&
            !std::is_convertible_v<Result<T2, E2>&&, T>)
        : m_stValue(InPlaceError, r2.GetError())
    {
        if (r2)
            m_stValue.template emplace<0>(std::move(*r2));
    }

public:
    constexpr explicit operator bool() const noexcept
    {
        return m_stValue.index() == 0;
    }

    constexpr T* operator->() noexcept
    {
        return std::get_if<0>(&m_stValue);
    }

    constexpr T const* operator->() const noexcept
    {
        return std::get_if<0>(&m_stValue);
    }

    constexpr T& operator*() & noexcept
    {
        T* p = operator->();
        assert(p != 0);
        return *p;
    }

    constexpr T const& operator*() const& noexcept
    {
        T const* p = operator->();
        assert(p != 0);
        return *p;
    }

    template <class U = T>
    constexpr T operator*() && noexcept(std::is_nothrow_move_constructible_v<T>)
        requires (std::is_move_constructible_v<U>)
    {
        return std::move(**this);
    }

    template <class U = T>
    constexpr T&& operator*() && noexcept
        requires (!std::is_move_constructible_v<U>)
    {
        return std::move(**this);
    }

    template <class U = T>
    constexpr T operator*() const && noexcept requires(std::is_move_constructible_v<U>) = delete;

    template <class U = T>
    constexpr T const&& operator*() const && noexcept
        requires (!std::is_move_constructible_v<U>)
    {
        return std::move(**this);
    }

    friend constexpr bool operator==(const Result& r1, const Result& r2) noexcept(noexcept(r1.m_stValue == r2.m_stValue))
    {
        return r1.m_stValue == r2.m_stValue;
    }

    friend constexpr bool operator!=(const Result& r1, const Result& r2) noexcept(noexcept(!(r1 == r2)))
    {
        return !(r1 == r2);
    }

public:
    /**
     * 是否有值
     */
    constexpr bool HasValue() const noexcept
    {
        return m_stValue.index() == 0;
    }

    /**
     * 是否有错误
     */
    constexpr bool HasError() const noexcept
    {
        return m_stValue.index() == 1;
    }

    /**
     * 获取值
     */
    constexpr T& GetValue() & noexcept
    {
        assert(HasValue());
        return *std::get_if<0>(&m_stValue);
    }

    constexpr const T& GetValue() const& noexcept
    {
        assert(HasValue());
        return *std::get_if<0>(&m_stValue);
    }

    template <class U = T>
    constexpr T GetValue() && noexcept(std::is_nothrow_move_constructible_v<T>)
        requires (std::is_move_constructible_v<U>)
    {
        return std::move(GetValue());
    }

    template <class U = T>
    constexpr T&& GetValue() && noexcept
        requires (!std::is_move_constructible_v<U>)
    {
        return std::move(GetValue());
    }

    template <class U = T>
    constexpr T GetValue() const&& noexcept
        requires (std::is_move_constructible_v<U>) = delete;

    template <class U = T>
    constexpr T const&& GetValue() const&& noexcept
        requires (!std::is_move_constructible_v<U>)
    {
        return std::move(GetValue());
    }

    /**
     * 获取错误
     */
    constexpr E GetError() const& noexcept(std::is_nothrow_default_constructible_v<E> && std::is_nothrow_copy_constructible_v<E>)
    {
        return HasError() ? *std::get_if<1>(&m_stValue) : E{};
    }

    constexpr E GetError() && noexcept(std::is_nothrow_default_constructible_v<E> && std::is_nothrow_move_constructible_v<E>)
    {
        return HasError() ? std::move(*std::get_if<1>(&m_stValue)) : E{};
    }

    /**
     * 原位构造值
     * @tparam A 参数列表
     * @param a 参数
     * @return 构造结果的引用
     */
    template <class... A>
    constexpr T& Emplace(A&&... a) noexcept(std::is_nothrow_constructible_v<T, decltype(std::forward<A>(a))...>)
    {
        return m_stValue.template emplace<0>(std::forward<A>(a)...);
    }

    /**
     * 交换值
     * @param r 被交换对象
     */
    constexpr void Swap(Result& r) noexcept(noexcept(m_stValue.swap(r.m_stValue)))
    {
        m_stValue.swap(r.m_stValue);
    }

    // for STL std::swap
    friend constexpr void swap(Result& r1, Result& r2) noexcept(noexcept(r1.Swap(r2)))
    {
        r1.Swap(r2);
    }

private:
    std::variant<T, E> m_stValue;
};

// Result<void> 特化
template <class E>
class Result<void, E>
{
public:
    using ValueType = void;
    using ErrorType = E;

    static constexpr InPlaceValueType InPlaceValue {};
    static constexpr InPlaceErrorType InPlaceError {};

public:
    constexpr Result() noexcept
        : m_stValue(InPlaceValue)
    {}

    template <class A>
    explicit constexpr Result(A&& a) noexcept(std::is_nothrow_constructible_v<E, A>)
        requires (std::is_constructible_v<E, A> && !std::is_convertible_v<A, E>)
        : m_stValue(InPlaceError, std::forward<A>(a))
    {}

    template <class A>
    constexpr Result(A&& a) noexcept(std::is_nothrow_constructible_v<E, A>)
        requires (std::is_convertible_v<A, E>)
        : m_stValue(InPlaceError, std::forward<A>(a))
    {}

    template <class... A>
    constexpr Result(A&&... a) noexcept(std::is_nothrow_constructible_v<E, A...>)
        requires (std::is_constructible_v<E, A...> && sizeof...(A) >= 2)
        : m_stValue(InPlaceError, std::forward<A>(a)...)
    {}

    constexpr Result(InPlaceValueType) noexcept
        : m_stValue(InPlaceValue)
    {}

    template <class... A>
    constexpr Result(InPlaceErrorType, A&&... a) noexcept(std::is_nothrow_constructible_v<E, A...>)
        requires (std::is_constructible_v<E, A...>)
        : m_stValue(InPlaceError, std::forward<A>(a)... )
    {}

    template <class E2>
    constexpr Result(const Result<void, E2>& r2) noexcept(std::is_nothrow_constructible_v<E, E2> &&
        std::is_nothrow_default_constructible_v<E2> &&
        std::is_nothrow_copy_constructible_v<E2>)
        requires (std::is_convertible_v<E2, E>)
        : m_stValue(InPlaceError, r2.GetError())
    {
        if (r2)
            Emplace();
    }

public:
    constexpr explicit operator bool() const noexcept
    {
        return m_stValue.index() == 0;
    }

    constexpr void* operator->() noexcept
    {
        return std::get_if<0>(&m_stValue);
    }

    constexpr void const* operator->() const noexcept
    {
        return std::get_if<0>(&m_stValue);
    }

    constexpr void operator*() const noexcept
    {
        assert(HasValue());
    }

    friend constexpr bool operator==(const Result& r1, const Result& r2) noexcept(noexcept(r1.m_stValue == r2.m_stValue))
    {
        return r1.m_stValue == r2.m_stValue;
    }

    friend constexpr bool operator!=(const Result& r1, const Result& r2) noexcept(noexcept(!(r1 == r2)))
    {
        return !(r1 == r2);
    }

public:
    constexpr bool HasValue() const noexcept
    {
        return m_stValue.index() == 0;
    }

    constexpr bool HasError() const noexcept
    {
        return m_stValue.index() == 1;
    }

    constexpr void GetValue() const noexcept
    {
        assert(HasValue());
    }

    constexpr E GetError() const& noexcept(std::is_nothrow_default_constructible_v<E> && std::is_nothrow_copy_constructible_v<E>)
    {
        return HasError() ? *std::get_if<1>(&m_stValue) : E{};
    }

    constexpr E GetError() && noexcept(std::is_nothrow_default_constructible_v<E> && std::is_nothrow_move_constructible_v<E>)
    {
        return HasError() ? std::move(*std::get_if<1>(&m_stValue)) : E{};
    }

    constexpr void Emplace() noexcept
    {
        m_stValue.template emplace<0>();
    }

    constexpr void Swap(Result& r) noexcept(noexcept(m_stValue.swap(r.m_stValue)))
    {
        m_stValue.swap(r.m_stValue);
    }

    // for STL std::swap
    friend constexpr void swap(Result& r1, Result& r2) noexcept(noexcept(r1.Swap(r2)))
    {
        r1.Swap(r2);
    }

private:
    std::variant<std::monostate, E> m_stValue;
};

// Result<T&> 特化
template <class U, class E>
class Result<U&, E>
{
public:
    using ValueType = U&;
    using ErrorType = E;

    static constexpr InPlaceValueType InPlaceValue {};
    static constexpr InPlaceErrorType InPlaceError {};

public:
    template <class A>
    constexpr Result(A&& a) noexcept(std::is_nothrow_constructible_v<U&, A>)
        requires (std::is_convertible_v<A, U&> &&
            !detail::ReferenceToTemporary<U, A>::value &&
            !std::is_convertible_v<A, E>)
        : m_stValue(InPlaceValue, &static_cast<U&>(std::forward<A>(a)))
    {}

    template <class A = E>
    constexpr Result(A&& a) noexcept(std::is_nothrow_constructible_v<E, A>)
        requires (std::is_convertible_v<A, E> && !std::is_convertible_v<A, U&>)
        : m_stValue(InPlaceError, std::forward<A>(a))
    {}

    template <class A>
    explicit constexpr Result(A&& a) noexcept(std::is_nothrow_constructible_v<U&, A>)
        requires (detail::IsConstructible<U&, A>::value &&
            !std::is_convertible_v<A, U&> &&
            !detail::ReferenceToTemporary<U, A>::value &&
            !detail::IsConstructible<E, A>::value)
        : m_stValue(InPlaceValue, &static_cast<U&>(std::forward<A>(a)))
    {}

    template <class... A>
    explicit constexpr Result(A&&... a) noexcept(std::is_nothrow_constructible_v<E, A...>)
        requires (!detail::IsConstructible<U&, A...>::value &&
            detail::IsConstructible<E, A...>::value &&
            sizeof...(A) >= 1)
        : m_stValue(InPlaceError, std::forward<A>(a)...)
    {}

    template <class A>
    constexpr Result(InPlaceValueType, A&& a) noexcept(std::is_nothrow_constructible_v<U&, A>)
        requires (std::is_constructible_v<U&, A> && !detail::ReferenceToTemporary<U, A>::value)
        : m_stValue(InPlaceValue, &static_cast<U&>( std::forward<A>(a)))
    {}

    template <class... A>
    constexpr Result(InPlaceErrorType, A&&... a) noexcept(std::is_nothrow_constructible_v<E, A...>)
        requires (std::is_constructible_v<E, A...>)
        : m_stValue(InPlaceError, std::forward<A>(a)...)
    {}

    template <class U2, class E2>
    constexpr Result(const Result<U2&, E2>& r2) noexcept(std::is_nothrow_constructible_v<U&, U2&> &&
        std::is_nothrow_constructible_v<E, E2> &&
        std::is_nothrow_default_constructible_v<E2> &&
        std::is_nothrow_copy_constructible_v<E2>)
        requires (std::is_convertible_v<U2&, U&> &&
            !detail::ReferenceToTemporary<U, U2&>::value &&
            std::is_convertible_v<E2, E> &&
            !std::is_convertible_v<const Result<U2&, E2>&, U&>)
        : m_stValue(InPlaceError, r2.error())
    {
        if (r2)
            Emplace(*r2);
    }

public:
    constexpr explicit operator bool() const noexcept
    {
        return m_stValue.index() == 0;
    }

    constexpr U* operator->() const noexcept
    {
        return HasValue() ? *std::get_if<0>(m_stValue) : 0;
    }

    constexpr U& operator*() const noexcept
    {
        U* p = operator->();
        assert(p != 0);
        return *p;
    }

    friend constexpr bool operator==(const Result& r1, const Result& r2)
        noexcept(noexcept(r1 && r2 ? *r1 == *r2 : r1.m_stValue == r2.m_stValue))
    {
        return r1 && r2 ? *r1 == *r2 : r1.m_stValue == r2.m_stValue;
    }

    friend constexpr bool operator!=(const Result& r1, const Result& r2)
        noexcept(noexcept(!(r1 == r2)))
    {
        return !(r1 == r2);
    }

public:
    constexpr bool HasValue() const noexcept
    {
        return m_stValue.index() == 0;
    }

    constexpr bool HasError() const noexcept
    {
        return m_stValue.index() == 1;
    }

    constexpr U& GetValue() const noexcept
    {
        assert(HasValue());
        return *std::get_if<0>(&m_stValue);
    }

    constexpr E GetError() const& noexcept(std::is_nothrow_default_constructible_v<E> && std::is_nothrow_copy_constructible_v<E>)
    {
        return HasError() ? *std::get_if<1>(&m_stValue) : E{};
    }

    constexpr E GetError() && noexcept(std::is_nothrow_default_constructible_v<E> && std::is_nothrow_move_constructible_v<E>)
    {
        return HasError() ? std::move(*std::get_if<1>(&m_stValue)) : E{};
    }

    template <class A>
    constexpr U& emplace(A&& a) noexcept(std::is_nothrow_constructible_v<U&, A>)
        requires (detail::IsConstructible<U&, A>::value && !detail::ReferenceToTemporary<U, A>::value)
    {
        return *m_stValue.template emplace<0>(&static_cast<U&>(a));
    }

    constexpr void Swap(Result& r) noexcept(noexcept(m_stValue.swap(r.m_stValue)))
    {
        m_stValue.swap(r.m_stValue);
    }

    // for STL std::swap
    friend constexpr void swap(Result& r1, Result& r2) noexcept(noexcept(r1.Swap(r2)))
    {
        r1.Swap(r2);
    }

private:
    std::variant<U*, E> m_stValue;
};

// </editor-fold>
// <editor-fold desc="operator |">

/**
 * operator |
 *
 * 比较 Result<T> 和默认值 U。
 * 如果 Result<T> 有值，则返回值，否则返回默认值 U。
 * @tparam T 存储值的类型
 * @tparam E 错误码的类型
 * @tparam U 默认值的类型
 * @param r 存储值
 * @param u 默认值
 * @return 结果
 */
template <class T, class E, class U>
T operator|(const Result<T, E>& r, U&& u)
    requires (detail::IsValueConvertibleTo<U, T>::value)
{
    if (r)
        return *r;
    else
        return std::forward<U>(u);
}

template <class T, class E, class U>
T operator|(Result<T, E>&& r, U&& u)
    requires (detail::IsValueConvertibleTo<U, T>::value)
{
    if (r)
        return *std::move(r);
    else
        return std::forward<U>(u);
}

/**
 * operator |
 *
 * 比较 Result<T> 和默认值函数 F。
 * 如果 Result<T> 有值，则返回值，否则返回默认值函数 F 的返回值。
 * @tparam T 存储值的类型
 * @tparam E 错误码的类型
 * @tparam F 默认函数类型
 * @param r 存储值
 * @param f 默认值函数
 * @return 结果
 */
template <class T, class E, class F, class U = decltype(std::declval<F>()())>
T operator|(const Result<T, E>& r, F&& f)
    requires (detail::IsValueConvertibleTo<U, T>::value)
{
    if (r)
        return *r;
    else
        return std::forward<F>(f)();
}

template <class T, class E, class F, class U = decltype(std::declval<F>()())>
T operator|(Result<T, E>&& r, F&& f)
    requires (detail::IsValueConvertibleTo<U, T>::value)
{
    if (r)
        return *std::move(r);
    else
        return std::forward<F>(f)();
}

template <class T, class E, class F, class U = decltype(std::declval<F>()())>
U operator|(const Result<T, E>& r, F&& f)
    requires (detail::IsResult<U>::value && detail::IsValueConvertibleTo<T, typename U::ValueType>::value)
{
    if (r)
        return *r;
    else
        return std::forward<F>(f)();
}

template <class T, class E, class F, class U = decltype(std::declval<F>()())>
U operator|(Result<T, E>&& r, F&& f)
    requires (detail::IsResult<U>::value && detail::IsValueConvertibleTo<T, typename U::ValueType>::value)
{
    if (r)
        return *std::move(r);
    else
        return std::forward<F>(f)();
}

template <class E, class F, class U = decltype(std::declval<F>()())>
U operator|(const Result<void, E>& r, F&& f)
    requires (detail::IsResult<U>::value && std::is_void_v<typename U::ValueType>)
{
    if (r)
        return {};
    else
        return std::forward<F>(f)();
}

template <class E, class F, class U = decltype(std::declval<F>()())>
U operator|(Result<void, E>&& r, F&& f)
    requires (detail::IsResult<U>::value && std::is_void_v<typename U::ValueType>)
{
    if (r)
        return {};
    else
        return std::forward<F>(f)();
}

// </editor-fold>
// <editor-fold desc="operator |=">

template <class T, class E, class U>
Result<T, E>& operator|=(Result<T, E>& r, U&& u)
    requires (detail::IsValueConvertibleTo<U, T>::value)
{
    if (!r)
        r = std::forward<U>(u);
    return r;
}

template <class T, class E, class F, class U = decltype(std::declval<F>()())>
Result<T, E>& operator|=(Result<T, E>& r, F&& f)
    requires (detail::IsValueConvertibleTo<U, T>::value)
{
    if (!r)
        r = std::forward<F>(f)();
    return r;
}

template <class T, class E, class F, class U = decltype(std::declval<F>()())>
Result<T, E>& operator|=(Result<T, E>& r, F&& f)
    requires (detail::IsResult<U>::value && detail::IsValueConvertibleTo<typename U::ValueType, T>::value &&
        std::is_convertible_v<typename U::ErrorType, E>)
{
    if (!r)
        r = std::forward<F>(f)();
    return r;
}

// </editor-fold>
// <editor-fold desc="operator &">

/**
 * operator &
 *
 * 如果 Result<T> 存在错误，返回错误。
 * 否则，计算表达式 F 的值并返回。
 * @tparam T 存储值类型
 * @tparam E 错误值类型
 * @tparam F 表达式函数类型
 * @param r Result 对象
 * @param f 表达式
 * @return 计算结果
 */
template <class T, class E, class F, class U = decltype(std::declval<F>()(std::declval<T const&>()))>
Result<U, E> operator&(const Result<T, E>& r, F&& f)
    requires (!detail::IsResult<U>::value && !std::is_void_v<U>)
{
    if (r.HasError())
        return r.GetError();
    else
        return std::forward<F>(f)(*r);
}

template <class T, class E, class F, class U = decltype(std::declval<F>()(std::declval<T>()))>
Result<U, E> operator&(Result<T, E>&& r, F&& f)
    requires (!detail::IsResult<U>::value && !std::is_void_v<U>)
{
    if (r.HasError())
        return r.GetError();
    else
        return std::forward<F>(f)(*std::move(r));
}

template <class T, class E, class F, class U = decltype(std::declval<F>()(std::declval<T const&>()))>
Result<U, E> operator&(const Result<T, E>& r, F&& f)
    requires (std::is_void_v<U>)
{
    if (r.HasError())
        return r.GetError();
    else
        std::forward<F>(f)(*r);
    return {};
}

template <class T, class E, class F, class U = decltype(std::declval<F>()(std::declval<T>()))>
Result<U, E> operator&(Result<T, E>&& r, F&& f)
    requires (std::is_void_v<U>)
{
    if (r.HasError())
        return r.GetError();
    else
        std::forward<F>(f)(*std::move(r));
    return {};
}

template <class E, class F, class U = decltype(std::declval<F>()())>
Result<U, E> operator&(const Result<void, E>& r, F&& f)
    requires (!detail::IsResult<U>::value && !std::is_void_v<U>)
{
    if (r.has_error())
        return r.GetError();
    else
        return std::forward<F>(f)();
}

template <class E, class F, class U = decltype(std::declval<F>()())>
Result<U, E> operator&(const Result<void, E>& r, F&& f)
    requires (std::is_void_v<U>)
{
    if (r.HasError())
        return r.GetError();
    else
        std::forward<F>(f)();
    return {};
}

template <class T, class E, class F, class U = decltype(std::declval<F>()(std::declval<T const&>()))>
U operator&(const Result<T, E>& r, F&& f)
    requires (detail::IsResult<U>::value && std::is_convertible_v<E, typename U::ErrorType>)
{
    if (r.HasError())
        return r.GetError();
    else
        return std::forward<F>(f)(*r);
}

template <class T, class E, class F, class U = decltype(std::declval<F>()(std::declval<T>()))>
U operator&(Result<T, E>&& r, F&& f)
    requires (detail::IsResult<U>::value && std::is_convertible_v<E, typename U::ErrorType>)
{
    if (r.HasError())
        return r.GetError();
    else
        return std::forward<F>(f)(*std::move(r));
}

template <class E, class F, class U = decltype(std::declval<F>()())>
U operator&(const Result<void, E>& r, F&& f)
    requires (detail::IsResult<U>::value && std::is_convertible_v<E, typename U::ErrorType>)
{
    if (r.HasError())
        return r.GetError();
    else
        return std::forward<F>(f)();
}

// </editor-fold>
// <editor-fold desc="operator &=">

template <class T, class E, class F, class U = decltype(std::declval<F>()(std::declval<T>()))>
Result<T, E>& operator&=(Result<T, E>& r, F&& f)
    requires (!detail::IsResult<U>::value && detail::IsValueConvertibleTo<U, T>::value)
{
    if (r)
        r = std::forward<F>(f)(*std::move(r));
    return r;
}

template <class E, class F, class U = decltype(std::declval<F>()())>
Result<void, E>& operator&=(Result<void, E>& r, F&& f)
    requires (!detail::IsResult<U>::value)
{
    if (r)
        std::forward<F>(f)();
    return r;
}

template <class T, class E, class F, class U = decltype(std::declval<F>()(std::declval<T>()))>
Result<T, E>& operator&=(Result<T, E>& r, F&& f)
    requires (detail::IsResult<U>::value && detail::IsValueConvertibleTo<typename U::ValueType, T>::value &&
        std::is_convertible_v<typename U::ErrorType, E>)
{
    if (r)
        r = std::forward<F>(f)(*std::move(r));
    return r;
}

template <class E, class F, class U = decltype(std::declval<F>()())>
Result<void, E>& operator&=(Result<void, E>& r, F&& f)
    requires (detail::IsResult<U>::value && std::is_void_v<typename U::ValueType> && std::is_convertible_v<typename U::ErrorType, E>)
{
    if (r)
        r = std::forward<F>(f)();
    return r;
}

// </editor-fold>

#define CHECK_RESULT(EXPR)       \
do {                            \
    auto&& ret_ = EXPR;         \
    if (!ret_) [[unlikely]]     \
        return ret_.GetError(); \
} while (false)
