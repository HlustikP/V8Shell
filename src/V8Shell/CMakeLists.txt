add_library(V8Shell STATIC V8Shell.cpp)

set_property(TARGET V8Shell PROPERTY CXX_STANDARD 17)

target_include_directories(V8Shell PUBLIC $ENV{V8_INCLUDE} ${CMAKE_PROJECT_DIR}V8Shell)

set(V8_LIB "")
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(V8_LIB $ENV{V8_LIB})
else()
    set(V8_LIB $ENV{V8_DEBUG})
endif()

target_link_directories(V8Shell PUBLIC V8_LIB)

target_link_libraries(V8Shell PUBLIC Commands)

if(WIN32)
    target_link_libraries(V8Shell PUBLIC V8SWindowsApi v8_monolith.lib winmm.lib dbghelp.lib)
elseif(UNIX)
    target_link_libraries(V8Shell PUBLIC V8SLinuxApi libv8_monolith.a)
endif()
