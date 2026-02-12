#pragma once

/// @file logger/derived/esp_log.hpp
/// @brief Logger backend using ESP-IDF's ESP_LOGx macros.

#include "logger/ability.hpp"
#include "esp_log.h"
#include <string>
#include <vector>

namespace osal {

class Logger : public LoggerAbility<Logger>
{
  friend class LoggerAbility<Logger>;
  friend class DispatchBase<Logger>;

public:
  Logger() { RebuildTag(); }
  ~Logger() = default;

  template <typename... Tags>
  Logger(Tags&&... tags) { SetTagsImpl(std::forward<Tags>(tags)...); }

private:
  // --- Configuration Impl ---

  void SetPrefixImpl(const std::string& prefix)  { prefix_ = prefix; RebuildTag(); }
  void SetSeparatorImpl(const std::string& sep)   { separator_ = sep; RebuildTag(); }
  void SetSuffixImpl(const std::string& suffix)   { suffix_ = suffix; RebuildTag(); }

  template <typename... Tags>
  void SetTagsImpl(Tags&&... tags)
  {
    tags_.clear();
    (tags_.emplace_back(std::forward<Tags>(tags)), ...);
    RebuildTag();
  }

  void SetTagImpl(int index, const std::string& tag)
  {
    if (index >= 0 && index < static_cast<int>(tags_.size())) {
      tags_[index] = tag;
      RebuildTag();
    }
  }

  void ClearTagsImpl() { tags_.clear(); RebuildTag(); }

  // --- Logging Impl (delegates to ESP_LOGx) ---

  template <typename... Args>
  void VerboseImpl(const char* format, Args&&... args)
  { ESP_LOGV(tag_.c_str(), format, std::forward<Args>(args)...); }

  template <typename... Args>
  void DebugImpl(const char* format, Args&&... args)
  { ESP_LOGD(tag_.c_str(), format, std::forward<Args>(args)...); }

  template <typename... Args>
  void InfoImpl(const char* format, Args&&... args)
  { ESP_LOGI(tag_.c_str(), format, std::forward<Args>(args)...); }

  template <typename... Args>
  void WarningImpl(const char* format, Args&&... args)
  { ESP_LOGW(tag_.c_str(), format, std::forward<Args>(args)...); }

  template <typename... Args>
  void ErrorImpl(const char* format, Args&&... args)
  { ESP_LOGE(tag_.c_str(), format, std::forward<Args>(args)...); }

  template <typename... Args>
  void FatalImpl(const char* format, Args&&... args)
  { ESP_LOGE(tag_.c_str(), format, std::forward<Args>(args)...); }

  // --- Internal ---

  void RebuildTag()
  {
    tag_.clear();
    tag_ += prefix_;
    for (size_t i = 0; i < tags_.size(); ++i) {
      tag_ += tags_[i];
      if (i + 1 < tags_.size()) tag_ += separator_;
    }
    tag_ += suffix_;
    if (tag_.empty()) tag_ = "logger";
  }

  std::vector<std::string> tags_;
  std::string tag_       = "logger";
  std::string prefix_    = "| ";
  std::string separator_ = " | ";
  std::string suffix_    = " |";
};

} // namespace osal
