cmake_minimum_required (VERSION 3.8)

project (V8Shell)

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
add_subdirectory(tests)
