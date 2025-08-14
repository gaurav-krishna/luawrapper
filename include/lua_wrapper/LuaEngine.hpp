#pragma once
#include <cstddef>
#include <functional>
#include <memory>
#include <unordered_map>
#include <type_traits>
#include <vector>
#include <string>
#include <new>
#include <cstring>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}


#include "ILuaEngine.hpp"
#include "ILuaEngine.hpp"
#include "LuaException.hpp"
#include "function_wrapper.hpp"
#include "lua_type_traits.hpp"

namespace lua_wrapper {

class LuaEngine final : public ILuaEngine {
public:
    explicit LuaEngine();  // creates a new lua_State
    ~LuaEngine() override = default; // unique_ptr takes care of cleanup

    // Implementation of pure virtual functions
    void executeFile(const std::string& path) override;
    bool doLoadString(const std::string& script) override;
    bool doCallFunction(int nargs, int nresults) override;
    void doGetGlobal(const std::string& name) override;
    lua_State* getState() const override { return m_state.get(); }
    void registerFunctionImplRaw(const std::string& name, void* funcPtr,
                                const std::type_info* retType,
                                const std::vector<const std::type_info*>& argTypes) override;

protected:
    struct lua_state_deleter {
        void operator()(lua_State* L) const { 
            if (L) lua_close(L); 
        }
    };

private:
    std::unique_ptr<lua_State, lua_state_deleter> m_state;
};

} // namespace lua_wrapper