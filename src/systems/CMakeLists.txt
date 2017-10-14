set (
  SOURCES
  ${SOURCES}
  "${CMAKE_CURRENT_LIST_DIR}/systems.c"
  "${CMAKE_CURRENT_LIST_DIR}/debug.c"
  #"${CMAKE_CURRENT_LIST_DIR}/game.c"
  #"${CMAKE_CURRENT_LIST_DIR}/input.c"
  "${CMAKE_CURRENT_LIST_DIR}/lua_bridge.c"
  "${CMAKE_CURRENT_LIST_DIR}/sound.c"
  "${CMAKE_CURRENT_LIST_DIR}/ui.c"
  "${CMAKE_CURRENT_LIST_DIR}/window.c"
  PARENT_SCOPE
)
