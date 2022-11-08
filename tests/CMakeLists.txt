enable_testing()

add_executable(tests unit_tests.cpp)

set_property(TARGET tests PROPERTY CXX_STANDARD 17)

target_include_directories(tests PUBLIC $ENV{V8_INCLUDE} "${PROJECT_SOURCE_DIR}/include")

target_link_directories(V8Shell PUBLIC $ENV{V8_LIB})
target_link_libraries(tests Commands GTest::gtest_main v8_monolith.lib winmm.lib dbghelp.lib)

include(GoogleTest)
gtest_discover_tests(tests)
