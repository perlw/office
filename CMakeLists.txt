cmake_minimum_required (VERSION 3.0)
project (office C)

set (CMAKE_INSTALL_RPATH "./")
set (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/deps")
include_directories ("${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src")

file(GLOB LUA_SRCS "${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src/*.c")
add_library (lua STATIC ${LUA_SRCS})

option (USE_OCCULUS "Use memory debugger" OFF)

add_subdirectory (src/bedrock)

set (
  SOURCES
  "src/main.c"
  "src/assets.c"
  "src/config.c"
  "src/input.c"
  "src/sound.c"
  "src/scenes.c"
  "src/scene_test.c"
  "src/scene_game.c"
  "src/scene_sound-test.c"
  "src/scene_world-edit.c"
)
add_subdirectory (src/debug)
add_subdirectory (src/ascii)
add_subdirectory (src/ui)
add_executable (office ${SOURCES})

if (UNIX)
  target_compile_options (bedrock PRIVATE -std=gnu99 -march=native -Wall)
endif (UNIX)
if (WIN32)
  if (MSVC)
    target_compile_options (office PRIVATE /EHsc /MP8)
    set_target_properties (office PROPERTIES LINK_FLAGS "/machine:x64 /nodefaultlib:libcmtd")
  endif (MSVC)
endif (WIN32)

if (USE_OCCULUS)
  target_compile_definitions (office PRIVATE MEM_DEBUG)
endif (USE_OCCULUS)

target_link_libraries (office bedrock lua)
add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets "${CMAKE_BINARY_DIR}/bin")
if (UNIX)
  add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/deps/fmod/linux/libfmod.so.9 "${CMAKE_BINARY_DIR}/bin")
endif (UNIX)
if (WIN32)
  add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/deps/fmod/windows/fmod64.dll "${CMAKE_BINARY_DIR}/bin")
endif (WIN32)
add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:office> "${CMAKE_BINARY_DIR}/bin")
