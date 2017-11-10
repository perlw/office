add_subdirectory (ui)

set (
  SOURCES
  ${SOURCES}
  "${CMAKE_CURRENT_LIST_DIR}/mod.c"
  "${CMAKE_CURRENT_LIST_DIR}/debug.c"
  "${CMAKE_CURRENT_LIST_DIR}/input.c"
  "${CMAKE_CURRENT_LIST_DIR}/sound.c"
  "${CMAKE_CURRENT_LIST_DIR}/test.c"
  "${CMAKE_CURRENT_LIST_DIR}/window.c"
  PARENT_SCOPE
)
