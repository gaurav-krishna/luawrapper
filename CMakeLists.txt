# CMakeLists.txt at project root
cmake_minimum_required(VERSION 3.14)
project(LuaWrapper LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD   99)

# Add compiler flags for stricter C++20 mode
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fconcepts-diagnostics-depth=2)
    add_compile_options(-ftemplate-backtrace-limit=0)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-fconcepts-diagnostics-depth=2)
endif()

# ---------------------------------------------------------------------------
# 1️⃣ Build Lua (static library)
# ---------------------------------------------------------------------------
add_library(lualib STATIC
    third_party/lua/lapi.c
    third_party/lua/lauxlib.c
    third_party/lua/lbaselib.c
    third_party/lua/lcode.c
    third_party/lua/lcorolib.c
    third_party/lua/lctype.c
    third_party/lua/ldblib.c
    third_party/lua/ldebug.c
    third_party/lua/ldo.c
    third_party/lua/ldump.c
    third_party/lua/lfunc.c
    third_party/lua/lgc.c
    third_party/lua/linit.c
    third_party/lua/liolib.c
    third_party/lua/llex.c
    third_party/lua/lmathlib.c
    third_party/lua/lmem.c
    third_party/lua/loadlib.c
    third_party/lua/lobject.c
    third_party/lua/lopcodes.c
    third_party/lua/loslib.c
    third_party/lua/lparser.c
    third_party/lua/lstate.c
    third_party/lua/lstring.c
    third_party/lua/lstrlib.c
    third_party/lua/ltable.c
    third_party/lua/ltablib.c
    third_party/lua/ltm.c
    third_party/lua/lua.c
    third_party/lua/lundump.c
    third_party/lua/lutf8lib.c
    third_party/lua/lvm.c
    third_party/lua/lzio.c
)
target_include_directories(lualib PUBLIC third_party/lua)

# ---------------------------------------------------------------------------
# 2️⃣ Set up GTest
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

FetchContent_MakeAvailable(googletest)

enable_testing()

# ---------------------------------------------------------------------------
# 3️⃣ Build the wrapper library
# ---------------------------------------------------------------------------
add_library(lua_wrapper
    src/LuaEngine.cpp
    src/LuaException.cpp
)

target_include_directories(lua_wrapper PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(lua_wrapper PUBLIC lualib)

# ---------------------------------------------------------------------------
# 3️⃣ Unit tests (GoogleTest)
# ---------------------------------------------------------------------------
enable_testing()
find_package(GTest REQUIRED)

add_executable(luatest test/LuaEngineTest.cpp)
target_link_libraries(luatest PRIVATE lua_wrapper GTest::gtest GTest::gtest_main)
add_test(NAME LuaEngineTests COMMAND luatest)