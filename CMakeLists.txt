cmake_minimum_required (VERSION 3.8)

project ("V8Shell")

if(MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
    )
endif()

add_compile_definitions(V8_COMPRESS_POINTERS V8_ENABLE_SANDBOX)

add_subdirectory ("src")
