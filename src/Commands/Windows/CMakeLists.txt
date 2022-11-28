add_library(V8SWindowsApi STATIC V8SWindowsApi.cpp)

set_property(TARGET V8SWindowsApi PROPERTY CXX_STANDARD 17)

target_include_directories(V8SWindowsApi PUBLIC "${PROJECT_SOURCE_DIR}/include")

target_link_directories(V8SWindowsApi PUBLIC $ENV{V8_LIB})
target_link_libraries(V8SWindowsApi PUBLIC winmm.lib dbghelp.lib)
