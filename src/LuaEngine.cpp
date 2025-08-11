// src/LuaEngine.cpp
#include "lua_wrapper/LuaEngine.hpp"
#include <fstream>
#include <sstream>

namespace lua_wrapper {

LuaEngine::LuaEngine()
    : m_state(luaL_newstate(), &lua_close)               // unique_ptr with custom deleter
{
    if (!m_state) {
        throw LuaException("Failed to create lua_State");
    }
    luaL_openlibs(m_state.get());   // load standard libs
}

// ---------------------------------------------------------------------------
// Destructor – RAII ensures the state is closed even on exception.
LuaEngine::~LuaEngine() = default;

// ---------------------------------------------------------------------------
// Execute a script given as a string.
void LuaEngine::execute(const std::string& script) {
    int rc = luaL_loadstring(m_state.get(), script.c_str());
    if (rc != LUA_OK) {
        std::string err = lua_tostring(m_state.get(), -1);
        lua_pop(m_state.get(), 1);
        throw LuaException("Lua load error: " + err);
    }
    rc = lua_pcall(m_state.get(), 0, LUA_MULTRET, 0);
    if (rc != LUA_OK) {
        std::string err = lua_tostring(m_state.get(), -1);
        lua_pop(m_state.get(), 1);
        throw LuaException("Lua runtime error: " + err);
    }
}

// ---------------------------------------------------------------------------
// Load and execute a file.
void LuaEngine::executeFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw LuaException("Unable to open Lua file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    execute(buffer.str());
}

// ---------------------------------------------------------------------------
// Register a C++ callable under a global name.
void LuaEngine::registerFunction(const std::string& name, LuaCFunction func) {
    // Store the callable on the heap; the key is its address.
    auto* heapFunc = new LuaCFunction(std::move(func));
    void* key = static_cast<void*>(heapFunc);
    m_callbacks[key] = *heapFunc; // keep a copy for lifetime tracking

    // Push the key as a lightuserdata upvalue for the dispatcher closure.
    lua_pushlightuserdata(m_state.get(), key);
    // Create a C closure with 1 upvalue → luaFunctionDispatcher.
    lua_pushcclosure(m_state.get(), &LuaEngine::luaFunctionDispatcher, 1);
    // Set global name.
    lua_setglobal(m_state.get(), name.c_str());
}

// ---------------------------------------------------------------------------
// Call a global Lua function (simple string‑to‑string API for the demo).
std::vector<std::string>
LuaEngine::callGlobal(const std::string& funcName,
                      const std::vector<std::string>& args,
                      int expectedRetVals)
{
    lua_getglobal(m_state.get(), funcName.c_str());
    if (!lua_isfunction(m_state.get(), -1)) {
        lua_pop(m_state.get(), 1);
        throw LuaException("Lua global '" + funcName + "' is not a function");
    }

    // push arguments
    for (const auto& a : args) {
        lua_pushlstring(m_state.get(), a.c_str(), a.size());
    }

    int rc = lua_pcall(m_state.get(),
                       static_cast<int>(args.size()),
                       expectedRetVals,
                       0);
    if (rc != LUA_OK) {
        std::string err = lua_tostring(m_state.get(), -1);
        lua_pop(m_state.get(), 1);
        throw LuaException("Lua call error: " + err);
    }

    // collect results
    std::vector<std::string> results;
    for (int i = -expectedRetVals; i < 0; ++i) {
        if (lua_isstring(m_state.get(), i)) {
            size_t len;
            const char* s = lua_tolstring(m_state.get(), i, &len);
            results.emplace_back(s, len);
        } else {
            results.emplace_back("[non‑string]");
        }
    }
    lua_pop(m_state.get(), expectedRetVals);
    return results;
}

// ---------------------------------------------------------------------------
// Dispatcher – called from Lua, forwards to the stored std::function.
int LuaEngine::luaFunctionDispatcher(lua_State* L) {
    // The upvalue (lightuserdata) holds the heap address of the std::function.
    void* key = lua_touserdata(L, lua_upvalueindex(1));
    if (!key) {
        lua_pushstring(L, "Invalid C++ callback key");
        lua_error(L);
        return 0; // unreachable
    }
    // Retrieve the C++ function from the map.
    // Since the wrapper is static we need a way to get to the correct instance.
    // The simplest approach for this demo: store the map in the registry.
    // Registry index key: we use the address of the map itself.
    lua_getfield(L, LUA_REGISTRYINDEX, "lua_wrapper_callbacks");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_pushstring(L, "Callback registry missing");
        lua_error(L);
        return 0;
    }
    lua_pushlightuserdata(L, key);
    lua_gettable(L, -2); // retrieve the std::function* stored as lightuserdata

    if (!lua_islightuserdata(L, -1)) {
        lua_pop(L, 3);
        lua_pushstring(L, "Callback not found");
        lua_error(L);
        return 0;
    }

    auto* fnPtr = static_cast<ILuaEngine::LuaCFunction*>(lua_touserdata(L, -1));
    if (!fnPtr) {
        lua_pop(L, 3);
        lua_pushstring(L, "Callback pointer is null");
        lua_error(L);
        return 0;
    }

    // Execute the user supplied C++ callable.
    int ret = (*fnPtr)(static_cast<void*>(L));

    // cleanup: pop registry table, key, fnPtr
    lua_pop(L, 3);
    return ret;
}

// ---------------------------------------------------------------------------
// When a LuaEngine is constructed we also fill the registry with the map that
// the dispatcher uses. This is done lazily the first time we register a function.
static void ensureCallbackRegistry(lua_State* L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "lua_wrapper_callbacks");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, "lua_wrapper_callbacks");
    } else {
        lua_pop(L, 1);
    }
}

// Hook registration to fill the registry automatically.
void LuaEngine::registerFunction(const std::string& name, LuaCFunction func) {
    ensureCallbackRegistry(m_state.get());

    auto* heapFunc = new LuaCFunction(std::move(func));
    void* key = static_cast<void*>(heapFunc);

    // Store in Lua registry:
    lua_getfield(m_state.get(), LUA_REGISTRYINDEX, "lua_wrapper_callbacks"); // t
    lua_pushlightuserdata_state.get(), key);                               // t key
    lua_pushlightuserdata(m_state.get(), heapFunc);                          // t key value
    lua_settable(m_state.get(), -3);                                          // t
    lua_pop(m_state.get(), 1); // pop registry table

    // push key as upvalue for dispatcher closure
    lua_pushlightuserdata(m_state.get(), key);
    lua_pushcclosure(m_state.get(), &LuaEngine::luaFunctionDispatcher, 1);
    lua_setglobal(m_state.get(), name.c_str());
}

} // namespace lua_wrapper