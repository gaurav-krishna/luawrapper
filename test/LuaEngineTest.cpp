// test/LuaEngineTest.cpp
#include "lua_wrapper/ILuaEngine.hpp"
#include "lua_wrapper/LuaEngine.hpp"
#include "lua_wrapper/LuaException.hpp"

#include <gtest/gtest.h>

using namespace lua_wrapper;

class LuaEngineTest : public ::testing::Test {
protected:
    std::unique_ptr<ILuaEngine> engine;

    void SetUp() override {
        engine = std::make_unique<LuaEngine>();
    }
};

TEST_F(LuaEngineTest, ExecuteSimpleScript) {
    const std::string script = R"(
        a = 10
        b = 20
        c = a + b
    )";
    EXPECT_NO_THROW(engine->execute(script));

    // Retrieve `c` from Lua to verify the script ran.
    auto result = engine->callGlobal("tostring", {"c"}, 1);
    ASSERT_FALSE(result.empty());
    EXPECT_EQ(result[0], "30");
}

TEST_F(LuaEngineTest, RegisterAndCallCppFunction) {
    // Register a simple addition function.
    engine->registerFunction("cpp_add",
        [](void* rawState) -> int {
            lua_State* L = static_cast<lua_State*>(rawState);
            // Expect exactly two numbers on the stack.
            double a = luaL_checknumber(L, 1);
            double b = luaL_checknumber(L, 2);
            lua_pushnumber(L, a + b);
            return 1;                // one result
        });

    const std::string script = R"(
        result = cpp_add(7, 8)
    )";
    engine->execute(script);

    // Verify that the Lua variable `result` holds 15.
    auto ret = engine->callGlobal("tostring", {"result"}, 1);
    ASSERT_FALSE(ret.empty());
    EXPECT_EQ(ret[0], "15");
}

TEST_F(LuaEngineTest, ErrorPropagation) {
    const std::string badScript = "this_is_not_valid_lua_code!";
    EXPECT_THROW(engine->execute(badScript), LuaException);
}

TEST_F(LuaEngineTest, CallbackReceivesCorrectArguments) {
    // Store values that the lambda will capture.
    int receivedA = 0, receivedB = 0;
    engine->registerFunction("record_args",
        [&](void* rawState) -> int {
            lua_State* L = static_cast<lua_State*>(rawState);
            receivedA = static_cast<int>(luaL_checkinteger(L, 1));
            receivedB = static_cast<int>(luaL_checkinteger(L, 2));
            lua_pushboolean(L, 1); // return true
            return 1;
        });

    const stdstring script = R"(
        ok = record_args(42, 99)
    )";
    engine->execute(script);
    EXPECT_TRUE(ok);
    EXPECT_EQ(receivedA, 42);
    EXPECT_EQ(receivedB, 99);
}