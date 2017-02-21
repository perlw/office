include_directories (${CMAKE_CURRENT_LIST_DIR})
include_directories ("${PROJECT_SOURCE_DIR}/deps")
include_directories ("${PROJECT_SOURCE_DIR}/deps/glad")
include_directories ("${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src")

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
add_subdirectory ("${PROJECT_SOURCE_DIR}/deps/glfw" "${PROJECT_SOURCE_DIR}/deps/glfw")
#GLFW

add_library (glad STATIC "${PROJECT_SOURCE_DIR}/deps/glad/glad.c")
file(GLOB LUA_SRCS "${PROJECT_SOURCE_DIR}/deps/lua-5.3.3/src/*.c")
add_library (lua STATIC ${LUA_SRCS})

add_library (bedrock STATIC bedrock.c)
add_subdirectory (archivist)
add_subdirectory (gossip)
add_subdirectory (muse)
add_subdirectory (occulus)
add_subdirectory (picasso)
add_dependencies (bedrock glad lua glfw)

if (USE_OCCULUS)
  target_compile_definitions (bedrock PRIVATE MEM_DEBUG)
endif (USE_OCCULUS)

list (APPEND DEPS_LIBS glad lua glfw)
if (UNIX)
  if (APPLE)
    # Dummy
  else (APPLE)
    list (APPEND DEPS_LIBS X11 Xrandr Xinerama Xxf86vm Xcursor dl GL m pthread rt)
		target_compile_options (bedrock PRIVATE -std=gnu99 -march=native -Wall)
  endif (APPLE)
endif (UNIX)

if (WIN32)
  if (MSVC)
    list (APPEND DEPS_LIBS opengl32.lib gdi32.lib user32.lib shell32.lib)
		target_compile_options (bedrock PRIVATE /EHsc /MP8)
    set_target_properties (bedrock PROPERTIES LINK_FLAGS "/machine:x64 /nodefaultlib:libcmtd")
  endif (MSVC)
endif (WIN32)

target_link_libraries (bedrock ${DEPS_LIBS})
