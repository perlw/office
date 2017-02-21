cmake_minimum_required (VERSION 3.0)
project (office C)

include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/deps")

# GLFW
option (BUILD_SHARED_LIBS "Build shared libraries" OFF)
option (GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option (GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option (GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option (GLFW_INSTALL "Generate installation target" OFF)
if (MSVC)
  option (USE_MSVC_RUNTIME_LIBRARY_DLL "Use MSVC runtime library DLL" OFF)
endif (MSVC)
include_directories ("${PROJECT_SOURCE_DIR}/deps/glfw/include")
add_subdirectory ("${PROJECT_SOURCE_DIR}/deps/glfw")

if (UNIX AND NOT APPLE)
  # Force clang
  set (CMAKE_C_COMPILER             "/usr/bin/clang")
  set (CMAKE_C_FLAGS                "-std=gnu99")
  set (CMAKE_C_FLAGS_DEBUG          "-g")
  set (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
  set (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
  set (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

  set (CMAKE_AR      "/usr/bin/llvm-ar-3.8")
  set (CMAKE_LINKER  "/usr/bin/llvm-ld-3.8")
  set (CMAKE_NM      "/usr/bin/llvm-nm-3.8")
  set (CMAKE_OBJDUMP "/usr/bin/llvm-objdump-3.8")
  set (CMAKE_RANLIB  "/usr/bin/llvm-ranlib-3.8")
endif (UNIX AND NOT APPLE)
#GLFW

option (USE_OCCULUS "Use memory debugger" OFF)
add_subdirectory (src/bedrock)

add_executable (office src/main.c)

if (UNIX)
  target_compile_options(office PRIVATE -Wall -Wno-missing-braces)
endif (UNIX)
if (WIN32)
  if (MSVC)
    target_compile_options(office PRIVATE /EHsc /MP8)
    set (LINKER_FLAGS /machine:x64 /nodefaultlib:libcmtd)
  endif (MSVC)
endif (WIN32)

if (USE_OCCULUS)
  target_compile_definitions (office PRIVATE MEM_DEBUG)
endif (USE_OCCULUS)

target_link_libraries (office bedrock ${LINKER_FLAGS})
add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:office>)
