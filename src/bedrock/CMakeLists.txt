include_directories (${CMAKE_CURRENT_LIST_DIR})
include_directories ("${PROJECT_SOURCE_DIR}/src/bedrock/archivist")
include_directories ("${PROJECT_SOURCE_DIR}/src/bedrock/gossip")
include_directories ("${PROJECT_SOURCE_DIR}/src/bedrock/muse")
include_directories ("${PROJECT_SOURCE_DIR}/src/bedrock/occulus")
include_directories ("${PROJECT_SOURCE_DIR}/src/bedrock/picasso")
include_directories ("${PROJECT_SOURCE_DIR}/deps")
include_directories ("${PROJECT_SOURCE_DIR}/deps/glfw/include")
include_directories ("${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src")

add_library (glad STATIC "${PROJECT_SOURCE_DIR}/deps/glad/glad.c")

file(GLOB LUA_SRCS "${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src/*.c")
add_library (lua STATIC ${LUA_SRCS})

add_library (bedrock STATIC bedrock.c archivist/a_main.c gossip/g_main.c muse/m_main.c occulus/o_main.c picasso/p_main.c picasso/p_shader.c picasso/p_program.c)
add_dependencies (bedrock glad lua)

target_link_libraries (bedrock glad lua "${PROJECT_SOURCE_DIR}/deps/glfw/build/src/libglfw3.a" X11 Xrandr Xinerama Xxf86vm Xcursor dl GL m pthread rt)
