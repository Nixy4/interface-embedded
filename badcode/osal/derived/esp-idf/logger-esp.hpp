#pragma once

#include "logger-base.hpp"
#include "esp_log.h"

namespace osal {

class Logger: public LoggerBase<Logger>
{
  friend LoggerBase<Logger>;
public:
  Logger() = default;
  ~Logger() = default;

protected:
  void InitializeImpl()
  {
    // ESP_LOGI("Logger", "Logger initialized");
  }

  void SetLevelImpl(LogLevel level)
  {
    esp_log_level_t esp_level;
    switch (level)
    {
    case LogLevel::VERBOSE:
      esp_level = ESP_LOG_VERBOSE;
      break;
    case LogLevel::DEBUG:
      esp_level = ESP_LOG_DEBUG;
      break;
    case LogLevel::INFO:
      esp_level = ESP_LOG_INFO;
      break;
    case LogLevel::WARNING:
      esp_level = ESP_LOG_WARN;
      break;
    case LogLevel::ERROR:
      esp_level = ESP_LOG_ERROR;
      break;
    case LogLevel::FATAL:
      esp_level = ESP_LOG_ERROR;
      break;
    default:
      esp_level = ESP_LOG_INFO;
      break;
    }
    esp_log_level_set("*", esp_level);
  }

  LogLevel GetLevelImpl()
  {
    switch (esp_log_level_get("*"))
    {
    case ESP_LOG_VERBOSE:
      return LogLevel::VERBOSE;
    case ESP_LOG_DEBUG:
      return LogLevel::DEBUG;
    case ESP_LOG_INFO:
      return LogLevel::INFO;
    case ESP_LOG_WARN:
      return LogLevel::WARNING;
    case ESP_LOG_ERROR:
      return LogLevel::ERROR;
    default:
      return LogLevel::INFO;
    }
  }

  template <typename... Args>
  void VerboseImpl(const char* format, Args&&... args)
  {
    ESP_LOGV(GetTags().c_str(), format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void DebugImpl(const char* format, Args&&... args)
  {
    ESP_LOGD(GetTags().c_str(), format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void InfoImpl(const char* format, Args&&... args)
  {
    ESP_LOGI(GetTags().c_str(), format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void WarningImpl(const char* format, Args&&... args)
  {
    ESP_LOGW(GetTags().c_str(), format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void ErrorImpl(const char* format, Args&&... args)
  {
    ESP_LOGE(GetTags().c_str(), format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void FatalImpl(const char* format, Args&&... args)
  {
    ESP_LOGE(GetTags().c_str(), format, std::forward<Args>(args)...);
  }
};

} // namespace osal
