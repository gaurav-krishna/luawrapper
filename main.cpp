#include "lua_wrapper/LuaEngine.hpp"
#include <iostream>

int main() {
    lua_wrapper::LuaEngine engine;

    // Register a C++ function that prints a message.
    engine.registerFunction("print_msg",
        [](std::string msg) -> void {
            std::cout << "[Lua] " << msg << std::endl;
        });

    // Run a tiny script that calls the C++ function.
    const char* script = R"(
        print_msg("Hello from Lua!")
        x = 5 * 2
        return x
    )";

    auto result = engine.execute<int>(script);
    std::cout << "x = " << result << '\n';
    return 0;
}