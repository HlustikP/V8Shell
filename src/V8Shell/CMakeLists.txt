add_library(V8Shell STATIC V8Shell.cpp)

set_property(TARGET V8Shell PROPERTY CXX_STANDARD 17)

target_include_directories(V8Shell PUBLIC $ENV{V8_INCLUDE} ${CMAKE_PROJECT_DIR}V8Shell)

target_link_directories(V8Shell PUBLIC $ENV{V8_LIB})
target_link_libraries(V8Shell PUBLIC v8_monolith.lib winmm.lib dbghelp.lib)
