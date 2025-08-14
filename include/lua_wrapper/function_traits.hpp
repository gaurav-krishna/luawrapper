#pragma once

#include <tuple>
#include <type_traits>
#include <cstddef>
#include <functional>
#include <utility>
#include <compare>
#include <concepts>

namespace lua_wrapper {

using std::size_t;

// Primary template for function_traits
template<typename F>
struct function_traits;

// Specialization for function pointers
template<typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using return_type = R;
    static constexpr size_t arity = sizeof...(Args);

    template<size_t I>
    using arg = typename std::tuple_element<I, std::tuple<Args...>>::type;

    using tuple_type = std::tuple<Args...>;
};

// Specialization for member function pointers
template<typename C, typename R, typename... Args>
struct function_traits<R(C::*)(Args...)> {
    using return_type = R;
    static constexpr size_t arity = sizeof...(Args);

    template<size_t I>
    using arg = typename std::tuple_element<I, std::tuple<Args...>>::type;

    using tuple_type = std::tuple<Args...>;
};

// Specialization for const member function pointers
template<typename C, typename R, typename... Args>
struct function_traits<R(C::*)(Args...) const> {
    using return_type = R;
    static constexpr size_t arity = sizeof...(Args);

    template<size_t I>
    using arg = typename std::tuple_element<I, std::tuple<Args...>>::type;

    using tuple_type = std::tuple<Args...>;
};

// Specialization for lambda expressions and functors
template<typename F>
struct function_traits : public function_traits<decltype(&F::operator())> {};

} // namespace lua_wrapper
