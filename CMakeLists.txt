cmake_minimum_required(VERSION 3.16)

if(DEFINED IDF_BUILD_ARTIFACT OR DEFINED ESP_PLATFORM)
  # 当前处于 ESP-IDF 构建环境中
  message(STATUS "Building under ESP-IDF")
  include(${CMAKE_CURRENT_LIST_DIR}/cmake/esp-idf.cmake)
endif()