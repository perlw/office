set (
  SOURCES
  ${SOURCES}
  "${CMAKE_CURRENT_LIST_DIR}/systems.c"
  "${CMAKE_CURRENT_LIST_DIR}/system_debug.c"
  "${CMAKE_CURRENT_LIST_DIR}/system_game.c"
  "${CMAKE_CURRENT_LIST_DIR}/system_input.c"
  "${CMAKE_CURRENT_LIST_DIR}/system_lua_bridge.c"
  "${CMAKE_CURRENT_LIST_DIR}/system_sound.c"
  "${CMAKE_CURRENT_LIST_DIR}/system_ui.c"
  PARENT_SCOPE
)
