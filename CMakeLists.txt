cmake_minimum_required (VERSION 3.8)

# Enforce new versioning mechanics
if (${CMAKE_VERSION} VERSION_GREATER 3.25.0) 
    cmake_policy(SET CMP<0048> NEW)
endif()

project (V8Shell VERSION 0.3.0)
include (CPack)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 17)

include(FetchContent)
set(gtest_force_shared_crt)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

FetchContent_MakeAvailable(googletest)

if(MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
    )
endif()

add_compile_definitions(V8_COMPRESS_POINTERS V8_ENABLE_SANDBOX)

add_subdirectory(src)

if(WIN32)
    add_subdirectory(tests)
endif()
