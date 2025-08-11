// include/lua_wrapper/ILuaEngine.hpp
#pragma once
#include <functional>
#include <string>
#include <vector>

namespace lua_wrapper {

/// @brief Abstract interface – see DIP.
class ILuaEngine {
public:
    using LuaCFunction = std::function<int(void*)>;

    virtual ~ILuaEngine() = default;

    /// @brief Execute a Lua chunk (script source). Throws on error.
    virtual void execute(const std::string& script) = 0;

    /// @brief Load a script from a file. Throws on I/O or Lua error.
    virtual void executeFile(const std::string& path) = 0;

    /// @brief Register a C++ callable that can be invoked from Lua.
    ///        The callable receives the raw lua_State* (you may push/pop
    ///        values yourself) and must return the number of results.
    virtual void registerFunction(const std::string& name,
                                  LuaCFunction func) = 0;

    /// @brief Convenience helper – call a global Lua function with N
    ///        arguments and fetch M results.
    virtual std::vector<std::string>
    callGlobal(const std::string& funcName,
               const std::vector<std::string>& args,
               int expectedRetVals) = 0;
};

} // namespace lua_wrapper