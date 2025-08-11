#include "lua_wrapper/LuaEngine.hpp"
#include <iostream>

int main() {
    lua_wrapper::LuaEngine engine;

    // Register a C++ function that prints a message.
    engine.registerFunction("print_msg",
        [](void* raw) -> int {
            lua_State* L = static_cast<lua_State*>(raw);
            const char* msg = luaL_checkstring(L, 1);
            std::cout << "[Lua] " << msg << std::endl;
            return 0; // no return values
        });

    // Run a tiny script that calls the C++ function.
    const char* script = R"(
        print_msg("Hello from Lua!")
        x = 5 * 2
    )";

    engine.execute(script);
    auto result = engine.callGlobal("tostring", {"x"}, 1);
    std::cout << "x = " << result[0] << '\n';
}