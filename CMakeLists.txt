cmake_minimum_required (VERSION 3.0)
project (office C)

include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/src/bedrock")
include_directories ("${PROJECT_SOURCE_DIR}/src/bedrock/archivist")
include_directories ("${PROJECT_SOURCE_DIR}/src/bedrock/gossip")
include_directories ("${PROJECT_SOURCE_DIR}/src/bedrock/muse")
include_directories ("${PROJECT_SOURCE_DIR}/src/bedrock/occulus")
include_directories ("${PROJECT_SOURCE_DIR}/src/bedrock/picasso")
include_directories ("${PROJECT_SOURCE_DIR}/deps")
include_directories ("${PROJECT_SOURCE_DIR}/deps/glfw/include")
include_directories ("${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src")

add_library (glad STATIC deps/glad/glad.c)
add_library (lua STATIC deps/lua-5.3.3/src/lapi.c deps/lua-5.3.3/src/lcode.c deps/lua-5.3.3/src/lctype.c deps/lua-5.3.3/src/ldebug.c deps/lua-5.3.3/src/ldo.c deps/lua-5.3.3/src/ldump.c deps/lua-5.3.3/src/lfunc.c deps/lua-5.3.3/src/lgc.c deps/lua-5.3.3/src/llex.c deps/lua-5.3.3/src/lmem.c deps/lua-5.3.3/src/lobject.c deps/lua-5.3.3/src/lopcodes.c deps/lua-5.3.3/src/lparser.c deps/lua-5.3.3/src/lstate.c deps/lua-5.3.3/src/lstring.c deps/lua-5.3.3/src/ltable.c deps/lua-5.3.3/src/ltm.c deps/lua-5.3.3/src/lundump.c deps/lua-5.3.3/src/lvm.c deps/lua-5.3.3/src/lzio.c deps/lua-5.3.3/src/lauxlib.c deps/lua-5.3.3/src/lbaselib.c deps/lua-5.3.3/src/lbitlib.c deps/lua-5.3.3/src/lcorolib.c deps/lua-5.3.3/src/ldblib.c deps/lua-5.3.3/src/liolib.c deps/lua-5.3.3/src/lmathlib.c deps/lua-5.3.3/src/loslib.c deps/lua-5.3.3/src/lstrlib.c deps/lua-5.3.3/src/ltablib.c deps/lua-5.3.3/src/lutf8lib.c deps/lua-5.3.3/src/loadlib.c deps/lua-5.3.3/src/linit.c)

add_library (bedrock STATIC src/bedrock/bedrock.c src/bedrock/archivist/a_main.c src/bedrock/gossip/g_main.c src/bedrock/muse/m_main.c src/bedrock/occulus/o_main.c src/bedrock/picasso/p_main.c src/bedrock/picasso/p_shader.c src/bedrock/picasso/p_program.c)
add_dependencies (bedrock glad lua)
target_link_libraries (bedrock glad lua "${PROJECT_SOURCE_DIR}/deps/glfw/build/src/libglfw3.a" X11 Xrandr Xinerama Xxf86vm Xcursor dl GL m pthread rt)

add_executable (office src/main.c)
target_link_libraries (office bedrock)
