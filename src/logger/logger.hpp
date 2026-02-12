#pragma once

/// @file logger/logger.hpp
/// @brief Conditional dispatch — selects Logger backend independently from OSAL.
///
/// Backend selection:
///   ESP-IDF Kconfig:   CONFIG_INTERFACE_EMBEDDED_LOGGER_ESP_LOG / _PRINTF / _EASYLOGGER
///   Standalone define: LOGGER_BACKEND_ESP_LOG / _PRINTF / _EASYLOGGER

#if defined(CONFIG_INTERFACE_EMBEDDED_LOGGER_ESP_LOG) || defined(LOGGER_BACKEND_ESP_LOG)
  #include "logger/derived/esp_log.hpp"
#elif defined(CONFIG_INTERFACE_EMBEDDED_LOGGER_PRINTF) || defined(LOGGER_BACKEND_PRINTF)
  #include "logger/derived/printf.hpp"
// #elif defined(CONFIG_INTERFACE_EMBEDDED_LOGGER_EASYLOGGER) || defined(LOGGER_BACKEND_EASYLOGGER)
//   #include "logger/derived/easylogger.hpp"
#else
  // Default: printf backend (works everywhere)
  #include "logger/derived/printf.hpp"
#endif
