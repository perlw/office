cmake_minimum_required (VERSION 3.0)
project (office C)

include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/deps")

option (USE_OCCULUS "Use memory debugger" OFF)
add_subdirectory (src/bedrock)

add_executable (office src/main.c)

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

target_link_libraries (office bedrock)
add_custom_command (TARGET office POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:office>)
