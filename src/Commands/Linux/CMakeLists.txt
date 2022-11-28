add_library(V8SLinuxApi STATIC V8SLinuxApi.cpp)

set_property(TARGET V8SLinuxApi PROPERTY CXX_STANDARD 17)

target_include_directories(V8SLinuxApi PUBLIC "${PROJECT_SOURCE_DIR}/include")
