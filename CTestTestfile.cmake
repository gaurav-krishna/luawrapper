# CMake generated Testfile for 
# Source directory: /workspaces/luawrapper
# Build directory: /workspaces/luawrapper
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(LuaEngineTests "/workspaces/luawrapper/luatest")
set_tests_properties(LuaEngineTests PROPERTIES  _BACKTRACE_TRIPLES "/workspaces/luawrapper/CMakeLists.txt;94;add_test;/workspaces/luawrapper/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
