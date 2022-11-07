    enable_testing()

    add_executable(tests unit_tests.cpp)

    set_property(TARGET tests PROPERTY CXX_STANDARD 17)

    target_include_directories(tests PUBLIC $ENV{V8_INCLUDE})

    target_link_directories(V8Shell PUBLIC $ENV{V8_LIB})
    target_link_libraries(tests V8Shell GTest::gtest_main v8_monolith.lib winmm.lib dbghelp.lib
    "${CMAKE_CURRENT_SOURCE_DIR}/../build/x64-release/src/CMakeFiles/V8Shell.dir/Commands/Commands.cpp.obj")

    include(GoogleTest)
    gtest_discover_tests(tests)
