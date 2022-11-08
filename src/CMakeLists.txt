cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (V8Shell 
	"${CMAKE_PROJECT_DIR}V8Shell.cpp"
	"${CMAKE_PROJECT_DIR}V8Shell.h"
)

add_subdirectory(Commands)

set_property(TARGET V8Shell PROPERTY CXX_STANDARD 17)

target_include_directories(V8Shell PUBLIC $ENV{V8_INCLUDE} "${PROJECT_SOURCE_DIR}/include")

target_link_directories(V8Shell PUBLIC $ENV{V8_LIB})
target_link_libraries(V8Shell PUBLIC Commands v8_monolith.lib winmm.lib dbghelp.lib)
