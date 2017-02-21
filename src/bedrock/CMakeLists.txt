include_directories (${CMAKE_CURRENT_LIST_DIR})
include_directories ("${PROJECT_SOURCE_DIR}/deps")
include_directories ("${PROJECT_SOURCE_DIR}/deps/glfw/include")
include_directories ("${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src")

add_library (glad STATIC "${PROJECT_SOURCE_DIR}/deps/glad/glad.c")

file(GLOB LUA_SRCS "${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src/*.c")
add_library (lua STATIC ${LUA_SRCS})

add_library (bedrock STATIC bedrock.c)
add_subdirectory(archivist)
add_subdirectory(gossip)
add_subdirectory(muse)
add_subdirectory(occulus)
add_subdirectory(picasso)
add_dependencies (bedrock glad lua)

target_link_libraries (bedrock glad lua "${PROJECT_SOURCE_DIR}/deps/glfw/build/src/libglfw3.a" X11 Xrandr Xinerama Xxf86vm Xcursor dl GL m pthread rt)
