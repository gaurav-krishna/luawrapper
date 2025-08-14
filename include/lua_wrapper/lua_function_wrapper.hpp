#pragma once
#include <functional>
#include "LuaException.hpp"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

namespace lua_wrapper {
namespace detail {

// Convert C++ values to Lua values
template<typename T>
struct to_lua;

// Push numeric types to Lua stack
template<>
struct to_lua<int> {
    static void push(lua_State* L, int value) { lua_pushinteger(L, value); }
};

template<>
struct to_lua<double> {
    static void push(lua_State* L, double value) { lua_pushnumber(L, value); }
};

template<>
struct to_lua<float> {
    static void push(lua_State* L, float value) { lua_pushnumber(L, value); }
};

// Push string types to Lua stack
template<>
struct to_lua<std::string> {
    static void push(lua_State* L, const std::string& value) {
        lua_pushlstring(L, value.data(), value.size());
    }
};

template<>
struct to_lua<const char*> {
    static void push(lua_State* L, const char* value) {
        lua_pushstring(L, value);
    }
};

// Push boolean to Lua stack
template<>
struct to_lua<bool> {
    static void push(lua_State* L, bool value) { lua_pushboolean(L, value); }
};

// Convert Lua values to C++ values
template<typename T>
struct from_lua;

template<>
struct from_lua<int> {
    static int get(lua_State* L, int index) {
        if (!lua_isnumber(L, index)) {
            throw LuaException("Expected number");
        }
        return lua_tointeger(L, index);
    }
};

template<>
struct from_lua<double> {
    static double get(lua_State* L, int index) {
        if (!lua_isnumber(L, index)) {
            throw LuaException("Expected number");
        }
        return lua_tonumber(L, index);
    }
};

template<>
struct from_lua<float> {
    static float get(lua_State* L, int index) {
        if (!lua_isnumber(L, index)) {
            throw LuaException("Expected number");
        }
        return static_cast<float>(lua_tonumber(L, index));
    }
};

template<>
struct from_lua<std::string> {
    static std::string get(lua_State* L, int index) {
        if (!lua_isstring(L, index)) {
            throw LuaException("Expected string");
        }
        size_t len;
        const char* str = lua_tolstring(L, index, &len);
        return std::string(str, len);
    }
};

template<>
struct from_lua<bool> {
    static bool get(lua_State* L, int index) {
        if (!lua_isboolean(L, index)) {
            throw LuaException("Expected boolean");
        }
        return lua_toboolean(L, index);
    }
};

// Helper to create Lua function wrappers for C++ functions
template<typename Ret, typename... Args>
struct function_wrapper {
    // Wrapper for C++ function returning non-void
    template<typename F>
    static int wrap(lua_State* L, F&& f) {
        try {
            if (lua_gettop(L) != sizeof...(Args)) {
                throw LuaException("Wrong number of arguments");
            }

            Ret result = call_helper(L, std::forward<F>(f), std::make_index_sequence<sizeof...(Args)>{});
            to_lua<Ret>::push(L, result);
            return 1;

        } catch (const std::exception& e) {
            luaL_error(L, e.what());
            return 0;
        }
    }

private:
    template<typename F, std::size_t... I>
    static Ret call_helper(lua_State* L, F&& f, std::index_sequence<I...>) {
        return f(from_lua<Args>::get(L, I + 1)...);
    }
};

// Specialization for void return type
template<typename... Args>
struct function_wrapper<void, Args...> {
    template<typename F>
    static int wrap(lua_State* L, F&& f) {
        try {
            if (lua_gettop(L) != sizeof...(Args)) {
                throw LuaException("Wrong number of arguments");
            }

            call_helper(L, std::forward<F>(f), std::make_index_sequence<sizeof...(Args)>{});
            return 0;

        } catch (const std::exception& e) {
            luaL_error(L, e.what());
            return 0;
        }
    }

private:
    template<typename F, std::size_t... I>
    static void call_helper(lua_State* L, F&& f, std::index_sequence<I...>) {
        f(from_lua<Args>::get(L, I + 1)...);
    }
};

} // namespace detail
} // namespace lua_wrapper
