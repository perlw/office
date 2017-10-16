cmake_minimum_required (VERSION 3.0)
project (office C)

set (CMAKE_INSTALL_RPATH "./")
set (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/deps")
include_directories ("${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src")

file(GLOB LUA_SRCS "${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src/*.c")
add_library (lua STATIC ${LUA_SRCS})

option (KRONOS_DEBUG "Debug kronos operations" OFF)
option (OCCULUS_LOG_MEM "Log memory allocations to file" OFF)
option (RECTIFY_DEBUG "Debug rectify operations" OFF)
option (LUA_BRIDGE_DEBUG "Use lua bridge debug output" OFF)

add_subdirectory (src/bedrock)

set (
  SOURCES
  "src/main.c"
  "src/assets.c"
  "src/config.c"
  "src/screen.c"
)
add_subdirectory (src/ascii)
add_subdirectory (src/systems)
add_subdirectory (src/scenes)
add_executable (office ${SOURCES})

if (UNIX)
  target_compile_options (office PRIVATE -std=gnu99 -march=native -Wall -Wno-incompatible-pointer-types)
endif (UNIX)
if (WIN32)
  if (MSVC)
    target_compile_options (office PRIVATE /EHsc /MP8 /D_CRT_SECURE_NO_WARNINGS /w44244 /WX)
    set_target_properties (office PROPERTIES LINK_FLAGS "/machine:x64 /nodefaultlib:libcmtd /nodefaultlib:libcmt")
  endif (MSVC)
endif (WIN32)

if (KRONOS_DEBUG)
  target_compile_definitions (office PRIVATE KRONOS_DEBUG)
endif (KRONOS_DEBUG)
if (OCCULUS_LOG_MEM)
  target_compile_definitions (office PRIVATE OCCULUS_LOG_MEM)
endif (OCCULUS_LOG_MEM)
if (RECTIFY_DEBUG)
  target_compile_definitions (office PRIVATE RECTIFY_DEBUG)
endif (RECTIFY_DEBUG)
if (LUA_BRIDGE_DEBUG)
  target_compile_definitions (office PRIVATE LUA_BRIDGE_DEBUG)
endif (LUA_BRIDGE_DEBUG)

target_link_libraries (office bedrock lua)
add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets "${CMAKE_BINARY_DIR}/bin")
add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/lua-src "${CMAKE_BINARY_DIR}/bin")
if (UNIX)
  add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/deps/fmod/linux/libfmod.so.9 "${CMAKE_BINARY_DIR}/bin")
endif (UNIX)
if (WIN32)
  add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/deps/fmod/windows/fmod64.dll "${CMAKE_BINARY_DIR}/bin")
endif (WIN32)
add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:office> "${CMAKE_BINARY_DIR}/bin")
