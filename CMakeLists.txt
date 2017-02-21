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

add_subdirectory (src/bedrock)

add_executable (office src/main.c)
target_link_libraries (office bedrock)
add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:office>)
