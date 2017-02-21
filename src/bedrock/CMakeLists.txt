include_directories (${CMAKE_CURRENT_LIST_DIR})

add_library (bedrock STATIC bedrock.c)
add_subdirectory(archivist)
add_subdirectory(gossip)
add_subdirectory(muse)
add_subdirectory(occulus)
add_subdirectory(picasso)
add_dependencies (bedrock glad lua)

if (USE_OCCULUS)
  target_compile_definitions (bedrock PRIVATE MEM_DEBUG)
endif (USE_OCCULUS)

target_link_libraries (bedrock glad lua "${PROJECT_SOURCE_DIR}/deps/glfw/build/src/libglfw3.a" X11 Xrandr Xinerama Xxf86vm Xcursor dl GL m pthread rt)
