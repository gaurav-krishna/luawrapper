// test/LuaEngineTest.cpp
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include "lua_wrapper/ILuaEngine.hpp"
#include "lua_wrapper/LuaEngine.hpp"
#include "lua_wrapper/LuaException.hpp"

namespace lua_wrapper {
namespace testing {

class LuaEngineTest : public ::testing::Test {
protected:
    std::unique_ptr<lua_wrapper::ILuaEngine> engine;

    void SetUp() override {
        engine = std::make_unique<lua_wrapper::LuaEngine>();
    }
};
TEST_F(LuaEngineTest, ExecuteSimpleScript) {
    const ::std::string script = R"(
        local x = 10
        local y = 20
        local z = x + y
        return z
    )";

    auto result = engine->execute<int>(script);
    EXPECT_EQ(result, 30);

    // Test different return types
    EXPECT_EQ(engine->execute<double>("return 3.14"), 3.14);
    EXPECT_EQ(engine->execute<::std::string>("return 'hello'"), "hello");
    EXPECT_EQ(engine->execute<bool>("return true"), true);
}

TEST_F(LuaEngineTest, RegisterAndCallCppFunction) {
    // Register a lambda function
    engine->registerFunction("cpp_add", 
        [](double a, double b) -> double {
            return a + b;
        });

    const ::std::string script = R"(
        result = cpp_add(7.5, 8.5)
        return result
    )";
    
    auto result = engine->execute<double>(script);
    EXPECT_DOUBLE_EQ(result, 16.0);

    // Test direct call from C++
    double sum = engine->callGlobal<double>("cpp_add", 10.5, 20.5);
    EXPECT_DOUBLE_EQ(sum, 31.0);
}

TEST_F(LuaEngineTest, ErrorPropagation) {
    const ::std::string badScript = "this_is_not_valid_lua_code!";
    EXPECT_THROW(engine->execute<void>(badScript), LuaException);
}

TEST_F(LuaEngineTest, CallbackReceivesCorrectArguments) {
    // Store values that the lambda will capture.
    int receivedA = 0, receivedB = 0;
    auto captureFunc = [&](int a, int b) -> bool {
        receivedA = a;
        receivedB = b;
        return true;
    };
    
    // Use new style registration without explicit template parameters
    engine->registerFunction("record_args", captureFunc);

    const ::std::string script = R"(
        result = record_args(42, 99)
        return result
    )";

    bool ret = engine->execute<bool>(script);

    EXPECT_TRUE(ret);
    EXPECT_EQ(receivedA, 42);
    EXPECT_EQ(receivedB, 99);
}

}} // namespace lua_wrapper::testing