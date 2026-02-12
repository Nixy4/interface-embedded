cmake_minimum_required(VERSION 3.16)

set(SRCS_DIR "${CMAKE_CURRENT_LIST_DIR}/../src")

# Standalone (non-ESP-IDF) build for interface-embedded.
#
# Logger backend (set one of):
#   LOGGER_BACKEND_ESP_LOG    — requires esp_log.h on include path
#   LOGGER_BACKEND_PRINTF     — fprintf(stderr), works everywhere
#   LOGGER_BACKEND_EASYLOGGER — future
#
# OSAL backend (set one of, optional):
#   OSAL_BACKEND_FREERTOS
#   OSAL_BACKEND_CMSIS_RTOS2
#   OSAL_BACKEND_POSIX
#   OSAL_BACKEND_CPP_STD

add_library(interface-embedded INTERFACE)

target_include_directories(interface-embedded INTERFACE
  "${SRCS_DIR}"
)

target_compile_features(interface-embedded INTERFACE cxx_std_17)

# --- Logger backend ---
if(LOGGER_BACKEND_ESP_LOG)
  target_compile_definitions(interface-embedded INTERFACE LOGGER_BACKEND_ESP_LOG=1)
elseif(LOGGER_BACKEND_PRINTF)
  target_compile_definitions(interface-embedded INTERFACE LOGGER_BACKEND_PRINTF=1)
elseif(LOGGER_BACKEND_EASYLOGGER)
  target_compile_definitions(interface-embedded INTERFACE LOGGER_BACKEND_EASYLOGGER=1)
else()
  # Default to printf if nothing selected
  target_compile_definitions(interface-embedded INTERFACE LOGGER_BACKEND_PRINTF=1)
endif()

# --- OSAL backend (optional) ---
if(OSAL_BACKEND_FREERTOS)
  target_compile_definitions(interface-embedded INTERFACE OSAL_BACKEND_FREERTOS=1)
elseif(OSAL_BACKEND_CMSIS_RTOS2)
  target_compile_definitions(interface-embedded INTERFACE OSAL_BACKEND_CMSIS_RTOS2=1)
elseif(OSAL_BACKEND_POSIX)
  target_compile_definitions(interface-embedded INTERFACE OSAL_BACKEND_POSIX=1)
  find_package(Threads REQUIRED)
  target_link_libraries(interface-embedded INTERFACE Threads::Threads)
elseif(OSAL_BACKEND_CPP_STD)
  target_compile_definitions(interface-embedded INTERFACE OSAL_BACKEND_CPP_STD=1)
  find_package(Threads REQUIRED)
  target_link_libraries(interface-embedded INTERFACE Threads::Threads)
endif()
