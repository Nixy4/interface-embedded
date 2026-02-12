#pragma once

/// @file logger/derived/printf.hpp
/// @brief Logger backend using fprintf(stderr).
/// Works on any platform with <cstdio> — POSIX, C++ Std, bare-metal, CMSIS, etc.

#include "logger/ability.hpp"
#include <cstdio>
#include <string>
#include <vector>

namespace ifce::log {

class Logger : public LoggerAbility<Logger>
{
  friend class LoggerAbility<Logger>;
  friend class ifce::DispatchBase<Logger>;

public:
  Logger() { RebuildTag(); }
  ~Logger() = default;

  template <typename... Tags>
  Logger(Tags&&... tags) { SetTagsImpl(std::forward<Tags>(tags)...); }

private:
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

  template <typename... Args>
  void VerboseImpl(const char* format, Args&&... args)
  { PrintLog("V", format, std::forward<Args>(args)...); }

  template <typename... Args>
  void DebugImpl(const char* format, Args&&... args)
  { PrintLog("D", format, std::forward<Args>(args)...); }

  template <typename... Args>
  void InfoImpl(const char* format, Args&&... args)
  { PrintLog("I", format, std::forward<Args>(args)...); }

  template <typename... Args>
  void WarningImpl(const char* format, Args&&... args)
  { PrintLog("W", format, std::forward<Args>(args)...); }

  template <typename... Args>
  void ErrorImpl(const char* format, Args&&... args)
  { PrintLog("E", format, std::forward<Args>(args)...); }

  template <typename... Args>
  void FatalImpl(const char* format, Args&&... args)
  { PrintLog("F", format, std::forward<Args>(args)...); }

  template <typename... Args>
  void PrintLog(const char* level, const char* format, Args&&... args)
  {
    std::fprintf(stderr, "[%s] %s ", level, tag_.c_str());
    std::fprintf(stderr, format, std::forward<Args>(args)...);
    std::fprintf(stderr, "\n");
  }

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

} // namespace ifce::log
