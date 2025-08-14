#pragma once

#include <utility>
#include <cstddef>
#include <type_traits>
#include <memory>
#include <new>
#include <string>
#include <compare>
#include <concepts>
#include <algorithm>

namespace lua_wrapper {

using std::size_t;

// Primary template for compile-time integer sequence
template<size_t N, typename F>
constexpr void template_for_impl(F&& f) {
    if constexpr (N > 0) {
        template_for_impl<N - 1>(f);
        f(std::integral_constant<size_t, N - 1>{});
    }
}

// Helper function for compile-time iteration
template<size_t N, typename F>
constexpr void template_for(F&& f) {
    template_for_impl<N>(std::forward<F>(f));
}

} // namespace lua_wrapper
