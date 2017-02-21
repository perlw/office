cmake_minimum_required (VERSION 3.0)
project (office C)

# Force clang
SET (CMAKE_C_COMPILER             "/usr/bin/clang")
SET (CMAKE_C_FLAGS                "-std=gnu99 -Wall -Wno-missing-braces")
SET (CMAKE_C_FLAGS_DEBUG          "-g")
SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
SET (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

SET (CMAKE_AR      "/usr/bin/llvm-ar-3.8")
SET (CMAKE_LINKER  "/usr/bin/llvm-ld-3.8")
SET (CMAKE_NM      "/usr/bin/llvm-nm-3.8")
SET (CMAKE_OBJDUMP "/usr/bin/llvm-objdump-3.8")
SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib-3.8")

include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/deps")
include_directories ("${PROJECT_SOURCE_DIR}/deps/glfw/include")
include_directories ("${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src")

add_library (glad STATIC "${PROJECT_SOURCE_DIR}/deps/glad/glad.c")

file(GLOB LUA_SRCS "${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src/*.c")
add_library (lua STATIC ${LUA_SRCS})

option (USE_OCCULUS "Use memory debugger" OFF)

add_subdirectory (src/bedrock)

add_executable (office src/main.c)
if (USE_OCCULUS)
  target_compile_definitions (office PRIVATE MEM_DEBUG)
endif (USE_OCCULUS)
target_link_libraries (office bedrock)
add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:office>)
