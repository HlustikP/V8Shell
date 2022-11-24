enable_testing()

add_executable(tests unit_tests.cpp)

set_property(TARGET tests PROPERTY CXX_STANDARD 17)

target_include_directories(tests PUBLIC $ENV{V8_INCLUDE} "${PROJECT_SOURCE_DIR}/include")

target_link_directories(tests PUBLIC $ENV{V8_LIB})
target_link_libraries(tests PUBLIC V8Shell Commands GTest::gtest_main)

if(WIN32)
    target_link_libraries(tests PUBLIC V8SWindowsApi winmm.lib dbghelp.lib v8_monolith.lib)
elseif(UNIX)
    target_link_libraries(tests PUBLIC V8SLinuxApi libv8_monolith.a)
endif()

include(GoogleTest)
gtest_discover_tests(tests)
