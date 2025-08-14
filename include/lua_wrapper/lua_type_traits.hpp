#pragma once
#include <string>
#include <string_view>
#include <type_traits>
#include <functional>
#include <tuple>
#include <cstddef>
#include <memory>
#include <cstdint>
#include <new>
#include <utility>
#include <compare>
#include <concepts>
#include "LuaException.hpp"

// Make sure basic types have allocator traits
namespace std {
    template<typename T, typename U>
    struct allocator_traits<allocator<T>>;
}

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace lua_wrapper {
namespace detail {

// Type cleanup helper
template<typename T>
using clean_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

// Forward declarations 
template<typename T>
struct function_traits;

// Base trait for function types
template<typename R, typename... Args>
struct base_function_traits {
    using return_type = R;
    using function_type = R(Args...);
    using arguments = std::tuple<Args...>;
    static constexpr size_t arity = sizeof...(Args);
};

// Function pointer
template<typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using signature = R(Args...);
    using return_type = R;
    using args = std::tuple<Args...>;
};

// Member function pointer
template<typename Class, typename R, typename... Args>
struct function_traits<R(Class::*)(Args...)> {
    using signature = R(Args...);
    using return_type = R;
    using args = std::tuple<Args...>;
};

// Const member function pointer
template<typename Class, typename R, typename... Args>
struct function_traits<R(Class::*)(Args...) const> {
    using signature = R(Args...);
    using return_type = R;
    using args = std::tuple<Args...>;
};

// Lambda traits
template<typename F>
struct function_traits {
    using Callable = typename std::remove_reference<F>::type;
    using MemberType = decltype(&Callable::operator());
    using BaseTraits = function_traits<MemberType>;
    using return_type = typename BaseTraits::return_type;
    using args = typename BaseTraits::args;
    using signature = typename BaseTraits::signature;
};

// Function reference traits
template<typename R, typename... Args>
struct function_traits<R(&)(Args...)> {
    using signature = R(Args...);
    using return_type = R;
    using args = std::tuple<Args...>;
};

// std::function traits
template<typename R, typename... Args>
struct function_traits<std::function<R(Args...)>> {
    using signature = R(Args...);
    using return_type = R;
    using args = std::tuple<Args...>;
};

template<typename T, typename = void>
struct extract_function_type;

template<typename ReturnType, typename... Args>
struct extract_function_type<ReturnType(*)(Args...), void> {
    using type = ReturnType(Args...);
};

template<typename ReturnType, typename... Args>
struct extract_function_type<ReturnType(&)(Args...), void> {
    using type = ReturnType(Args...);
};

template<typename ReturnType, typename... Args>
struct extract_function_type<std::function<ReturnType(Args...)>, void> {
    using type = ReturnType(Args...);
};

template<typename T>
struct extract_function_type<T, std::void_t<decltype(&T::operator())>> {
private:
    using member_function_type = decltype(&T::operator());
    using deduced_type = typename extract_function_type<member_function_type, void>::type;
public:
    using type = deduced_type;
};

// Base type traits template
template<typename T>
struct lua_type_traits {
    static_assert(sizeof(T) == 0, "No lua_type_traits specialization for type T");
    static const char* type_name() { return "unknown"; }
};

// Specialization for void
template<>
struct lua_type_traits<void> {
    static void push(lua_State*) {}
    static void get(lua_State*) {}
    static bool is_type(lua_State*, int) { return true; }
    static const char* type_name() { return "void"; }
};

// Specialization for integers
template<>
struct lua_type_traits<int> {
    static void push(lua_State* L, int value) { 
        lua_pushinteger(L, static_cast<lua_Integer>(value)); 
    }
    static int get(lua_State* L, int index) {
        if (!is_type(L, index)) {
            throw lua_wrapper::LuaException("Expected number");
        }
        return static_cast<int>(lua_tointeger(L, index));
    }
    static bool is_type(lua_State* L, int index) {
        return lua_isnumber(L, index);
    }
    static const char* type_name() { return "number"; }
};

// Specialization for double
template<>
struct lua_type_traits<double> {
    static void push(lua_State* L, double value) {
        lua_pushnumber(L, static_cast<lua_Number>(value));
    }
    static double get(lua_State* L, int index) {
        if (!is_type(L, index)) {
            throw lua_wrapper::LuaException("Expected number");
        }
        return static_cast<double>(lua_tonumber(L, index));
    }
    static bool is_type(lua_State* L, int index) {
        return lua_isnumber(L, index);
    }
    static const char* type_name() { return "number"; }
};

// Specialization for bool
template<>
struct lua_type_traits<bool> {
    static void push(lua_State* L, bool value) {
        lua_pushboolean(L, value);
    }
    static bool get(lua_State* L, int index) {
        if (!is_type(L, index)) {
            throw lua_wrapper::LuaException("Expected boolean");
        }
        return lua_toboolean(L, index);
    }
    static bool is_type(lua_State* L, int index) {
        return lua_isboolean(L, index);
    }
    static const char* type_name() { return "boolean"; }
};

// Specialization for strings
template<>
struct lua_type_traits<std::string> {
    static void push(lua_State* L, const std::string& value) {
        auto* str = value.c_str();
        auto len = value.length();
        lua_pushlstring(L, str, len);
    }
    static std::string get(lua_State* L, int index) {
        if (!is_type(L, index)) {
            throw lua_wrapper::LuaException("Expected string");
        }
        std::size_t len = 0;
        const char* str = lua_tolstring(L, index, &len);
        return str ? std::string(str, str + len) : std::string();
    }
    static bool is_type(lua_State* L, int index) {
        return lua_isstring(L, index);
    }
    static const char* type_name() { return "string"; }
};

// Specialization for const char*
template<>
struct lua_type_traits<const char*> {
    static void push(lua_State* L, const char* value) {
        lua_pushstring(L, value);
    }
    static const char* get(lua_State* L, int index) {
        if (!is_type(L, index)) {
            throw lua_wrapper::LuaException("Expected string");
        }
        return lua_tostring(L, index);
    }
    static bool is_type(lua_State* L, int index) {
        return lua_isstring(L, index);
    }
    static const char* type_name() { return "string"; }
};

// Type registration utilities
template<typename T>
struct type_name {
    static const char* get() { return lua_type_traits<T>::type_name(); }
};

} // namespace detail
} // namespace lua_wrapper
