cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (V8Shell 
	"${CMAKE_PROJECT_DIR}V8Shell.cpp"
	"${CMAKE_PROJECT_DIR}V8Shell.h"
	"${CMAKE_PROJECT_DIR}Commands/Commands.cpp"
)

set_property(TARGET V8Shell PROPERTY CXX_STANDARD 17)

target_include_directories(V8Shell PUBLIC $ENV{V8_INCLUDE} ${CMAKE_PROJECT_DIR}Commands)

target_link_directories(V8Shell PUBLIC $ENV{V8_LIB})
target_link_libraries(V8Shell PUBLIC v8_monolith.lib winmm.lib dbghelp.lib)
