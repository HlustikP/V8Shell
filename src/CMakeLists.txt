cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (V8Shell "V8Shell.cpp" "V8Shell.h")

set_property(TARGET V8Shell PROPERTY CXX_STANDARD 17)

target_include_directories(V8Shell PUBLIC $ENV{V8_INCLUDE})

target_link_directories(V8Shell PUBLIC $ENV{V8_LIB})
target_link_libraries(V8Shell PUBLIC v8_monolith.lib winmm.lib dbghelp.lib)
