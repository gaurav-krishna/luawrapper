// include/lua_wrapper/LuaEngine.hpp
#pragma once
#include "ILuaEngine.hpp"
#include "LuaException.hpp"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <memory>
#include <unordered_map>

namespace lua_wrapper {

class LuaEngine final : public ILuaEngine {
public:
    explicit LuaEngine();                     // creates a new lua_State
    ~LuaEngine() override;                    // closes the state

    // ────── ILuaEngine implementation ──────
    void execute(const std::string& script) override;
    void executeFile(const std::string& path) override;
    void registerFunction(const std::string& name,
                          LuaCFunction func) override;
    std::vector<std::string> callGlobal(const std::string& funcName,
                                        const std::vector<std::string>& args,
                                        int expectedRetVals) override;

private:
    // Helper to push a C++ std::function as a C closure.
    static int luaFunctionDispatcher(lua_State* L);

    // State ownership – RAII.
    std::unique_ptr<lua_State, decltype(&lua_close)> m_state;

    // Mapping from Lua registry keys → C++ callbacks.
    // The key is the address of the `LuaCFunction` stored on the heap.
    std::unordered_map<void*, LuaCFunction> m_callbacks;
};

} // namespace lua_wrapper