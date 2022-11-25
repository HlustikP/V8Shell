cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (V8ShellMain 
	"main.cpp"
)

if(WIN32)
    set_target_properties(V8ShellMain PROPERTIES OUTPUT_NAME "V8Shell")
endif()

add_subdirectory(Commands)
add_subdirectory(V8Shell)

set_property(TARGET V8ShellMain PROPERTY CXX_STANDARD 17)

target_include_directories(V8ShellMain PUBLIC $ENV{V8_INCLUDE} "${PROJECT_SOURCE_DIR}/include")

target_link_directories(V8ShellMain PUBLIC $ENV{V8_LIB})
target_link_libraries(V8ShellMain PUBLIC Commands V8Shell)

if(WIN32)
    target_link_libraries(V8ShellMain PUBLIC V8SWindowsApi v8_monolith.lib winmm.lib dbghelp.lib)
elseif(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(V8ShellMain PUBLIC V8SLinuxApi libv8_monolith.a ${CMAKE_DL_LIBS} Threads::Threads)
endif()
