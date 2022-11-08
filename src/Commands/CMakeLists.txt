add_library(Commands STATIC Commands.cpp)

set_property(TARGET Commands PROPERTY CXX_STANDARD 17)

target_include_directories(Commands PUBLIC $ENV{V8_INCLUDE} ${CMAKE_PROJECT_DIR}Commands)

target_link_directories(Commands PUBLIC $ENV{V8_LIB})
target_link_libraries(Commands PUBLIC v8_monolith.lib winmm.lib dbghelp.lib)
