add_library(Commands STATIC Commands.cpp)

set_property(TARGET Commands PROPERTY CXX_STANDARD 17)

if(WIN32)
    add_subdirectory(Windows)
elseif(UNIX)
    add_subdirectory(Linux)
endif()

set(V8_LIB "")
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(V8_LIB $ENV{V8_LIB})
else()
    set(V8_LIB $ENV{V8_DEBUG})
endif()

target_include_directories(Commands PUBLIC $ENV{V8_INCLUDE} "${PROJECT_SOURCE_DIR}/include")
target_link_directories(Commands PUBLIC "${V8_LIB}")

if(WIN32)
    target_link_libraries(Commands PUBLIC V8SWindowsApi winmm.lib dbghelp.lib v8_monolith.lib)
elseif(UNIX)
    target_link_libraries(Commands PUBLIC V8SLinuxApi libv8_monolith.a)
endif()
