set(SRCS_DIR ${CMAKE_CURRENT_LIST_DIR}/../src/)
set(SRCS
    # "${SRCS_DIR}/ability/logger.cpp"
    # "${SRCS_DIR}/ability/task.cpp"
    # "${SRCS_DIR}/ability/queue.cpp"
    # "${SRCS_DIR}/ability/event.cpp"
    )

idf_component_register(
  SRCS "${SRCS}"
  INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/../src")