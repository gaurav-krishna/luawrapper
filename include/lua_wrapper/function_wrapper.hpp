#pragma once

#include <tuple>
#include <type_traits>
#include <functional>
#include <cstddef>
#include <utility>
#include <new>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "LuaException.hpp"
#include "lua_type_traits.hpp"

namespace lua_wrapper {
namespace detail {

template<typename Ret, typename Args>
struct function_wrapper;

template<typename Ret, typename... Args>
struct function_wrapper<Ret, std::tuple<Args...>> {
    using return_type = Ret;
    using arguments = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);

    template<typename F>
    static int wrap(lua_State* L, F&& func) {
        try {
            if (lua_gettop(L) != sizeof...(Args)) {
                throw lua_wrapper::LuaException("Wrong number of arguments");
            }

            if constexpr (std::is_same_v<Ret, void>) {
                call_helper<F>(L, std::forward<F>(func), std::make_index_sequence<sizeof...(Args)>{});
                return 0;
            } else {
                Ret result = call_helper<F>(L, std::forward<F>(func), std::make_index_sequence<sizeof...(Args)>{});
                detail::lua_type_traits<Ret>::push(L, std::move(result));
                return 1;
            }
        } catch (const std::exception& e) {
            luaL_error(L, e.what());
            return 0;
        }
    }

private:
    template<typename F, std::size_t... Is>
    static auto call_helper(lua_State* L, F&& func, std::index_sequence<Is...>) {
        return func(detail::lua_type_traits<std::remove_reference_t<Args>>::get(L, Is + 1)...);
    }
};

} // namespace detail
} // namespace lua_wrapper
