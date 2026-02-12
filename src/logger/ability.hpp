#pragma once

/// @file logger/ability.hpp
/// @brief LoggerAbility<Derived> — CRTP mixin for logging.
///
/// Independent from OSAL. Uses common/dispatch.hpp for CRTP helpers.
/// Backends (esp_log, printf, easylogger, ...) are selected independently
/// from the OSAL backend.

#include "common/dispatch.hpp"
#include <cstdio>
#include <string>
#include <vector>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace osal {

template <typename Derived>
class LoggerAbility : protected DispatchBase<Derived>
{
  friend Derived;
  using Base = DispatchBase<Derived>;

protected:
  /// LogDispatch: invoke callable on Derived*, or printf fallback
  template <typename Fn, typename... Args>
  void LogDispatch(const char* label, Fn&& fn, const char* format, Args&&... args)
  {
    if constexpr (std::is_invocable_v<Fn, Derived*, const char*, Args...>)
      std::forward<Fn>(fn)(static_cast<Derived*>(this), format, std::forward<Args>(args)...);
    else {
      std::printf("%s: ", label);
      std::printf(format, std::forward<Args>(args)...);
      std::printf("\n");
    }
  }

public:
  LoggerAbility()  = default;
  ~LoggerAbility() = default;

  // --- Configuration ---

  void SetPrefix(const std::string& prefix)
  {
    Base::Dispatch([](auto* s, const auto& v)
      -> decltype((void)s->SetPrefixImpl(v)) { s->SetPrefixImpl(v); }, prefix);
  }

  void SetSeparator(const std::string& separator)
  {
    Base::Dispatch([](auto* s, const auto& v)
      -> decltype((void)s->SetSeparatorImpl(v)) { s->SetSeparatorImpl(v); }, separator);
  }

  void SetSuffix(const std::string& suffix)
  {
    Base::Dispatch([](auto* s, const auto& v)
      -> decltype((void)s->SetSuffixImpl(v)) { s->SetSuffixImpl(v); }, suffix);
  }

  template <typename... Tags>
  void SetTags(Tags&&... tags)
  {
    Base::Dispatch([](auto* s, auto&&... t)
      -> decltype((void)s->SetTagsImpl(std::forward<decltype(t)>(t)...)) {
        s->SetTagsImpl(std::forward<decltype(t)>(t)...);
    }, std::forward<Tags>(tags)...);
  }

  void SetTag(int index, const std::string& tag)
  {
    Base::Dispatch([](auto* s, int i, const auto& t)
      -> decltype((void)s->SetTagImpl(i, t)) { s->SetTagImpl(i, t); }, index, tag);
  }

  void ClearTags()
  {
    Base::Dispatch([](auto* s)
      -> decltype((void)s->ClearTagsImpl()) { s->ClearTagsImpl(); });
  }

  // --- Logging ---

  template <typename... Args>
  void Verbose(const char* format, Args&&... args)
  {
    LogDispatch("VERBOSE", [](auto* s, const char* f, auto&&... a)
      -> decltype((void)s->VerboseImpl(f, std::forward<decltype(a)>(a)...)) {
        s->VerboseImpl(f, std::forward<decltype(a)>(a)...);
    }, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void Debug(const char* format, Args&&... args)
  {
    LogDispatch("DEBUG", [](auto* s, const char* f, auto&&... a)
      -> decltype((void)s->DebugImpl(f, std::forward<decltype(a)>(a)...)) {
        s->DebugImpl(f, std::forward<decltype(a)>(a)...);
    }, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void Info(const char* format, Args&&... args)
  {
    LogDispatch("INFO", [](auto* s, const char* f, auto&&... a)
      -> decltype((void)s->InfoImpl(f, std::forward<decltype(a)>(a)...)) {
        s->InfoImpl(f, std::forward<decltype(a)>(a)...);
    }, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void Warning(const char* format, Args&&... args)
  {
    LogDispatch("WARNING", [](auto* s, const char* f, auto&&... a)
      -> decltype((void)s->WarningImpl(f, std::forward<decltype(a)>(a)...)) {
        s->WarningImpl(f, std::forward<decltype(a)>(a)...);
    }, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void Error(const char* format, Args&&... args)
  {
    LogDispatch("ERROR", [](auto* s, const char* f, auto&&... a)
      -> decltype((void)s->ErrorImpl(f, std::forward<decltype(a)>(a)...)) {
        s->ErrorImpl(f, std::forward<decltype(a)>(a)...);
    }, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void Fatal(const char* format, Args&&... args)
  {
    LogDispatch("FATAL", [](auto* s, const char* f, auto&&... a)
      -> decltype((void)s->FatalImpl(f, std::forward<decltype(a)>(a)...)) {
        s->FatalImpl(f, std::forward<decltype(a)>(a)...);
    }, format, std::forward<Args>(args)...);
  }
};

} // namespace osal
