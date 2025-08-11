// include/lua_wrapper/LuaException.hpp
#pragma once
#include <stdexcept>
#include <string>

namespace lua_wrapper {

class LuaException : public std::runtime_error {
public:
    explicit LuaException(const std::string& msg)
        : std::runtime_error(msg) {}
};

} // namespace lua_wrapper