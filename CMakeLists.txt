cmake_minimum_required (VERSION 3.0)
project (office C)

include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/deps")
include_directories ("${PROJECT_SOURCE_DIR}/deps/glfw/include")
include_directories ("${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src")

add_subdirectory (src/bedrock)

add_executable (office src/main.c)
target_link_libraries (office bedrock)
