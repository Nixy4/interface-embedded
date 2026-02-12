#pragma once
#include <vector>
#include <string>
#include <utility>

// // Helper macro for format checking in derived classes
// #ifdef __GNUC__
//   #define LOGGER_FORMAT_CHECK(fmt_idx, args_idx) __attribute__((format(printf, fmt_idx, args_idx)))
// #else
//   #define LOGGER_FORMAT_CHECK(fmt_idx, args_idx)
// #endif

namespace osal {

enum class LogLevel
{
  VERBOSE,
  DEBUG,
  INFO,
  WARNING,
  ERROR,
  FATAL
};

template <typename Derived>
class LoggerBase
{
  friend Derived;
protected:
  std::vector<std::string> tags_;
  size_t width_;
  std::string prefix_;
  std::string suffix_;
  std::string separator_;
  std::string combination_;

  LoggerBase()
  {
    prefix_      = "| ";
    suffix_      = " |";
    separator_   = " | ";
    width_       = 10;
    combination_ = "No Tags";
  }
  ~LoggerBase() = default;
  
  // Disable copy
  LoggerBase(const LoggerBase&) = delete;
  LoggerBase& operator=(const LoggerBase&) = delete;
  
  // Enable move
  LoggerBase(LoggerBase&&) = default;
  LoggerBase& operator=(LoggerBase&&) = default;

public:

  void Initialize()
  {
    static_cast<Derived *>(this)->InitializeImpl();
  }
  
  template <typename... Args>
  void SetTags(Args&&... args)
  {
    tags_.clear();
    (tags_.emplace_back(std::forward<Args>(args)), ...);
    UpdateCombination();
  }

  std::string GetTags()
  {
    return combination_;
  }

private:
  void UpdateCombination()
  {
    // 生成组合标签字符串
    combination_.clear();
    combination_ = prefix_;
    if(tags_.empty())
    {
      combination_ += "No Tags";
      combination_ += suffix_;
      return;
    }
    for (size_t i = 0; i < tags_.size(); ++i)
    {
      combination_ += tags_[i];
      if (tags_[i].length() < width_)
      {
        combination_ += std::string(width_ - tags_[i].length(), ' ');
      }
      if (i != tags_.size() - 1)
      {
        combination_ += separator_;
      }
    }
    combination_ += suffix_;
  }

  public:
  // 需要实现的接口
  void SetLevel(LogLevel level)
  {
    static_cast<Derived *>(this)->SetLevelImpl(level);
  }
  
  LogLevel GetLevel()
  {
    return static_cast<Derived *>(this)->GetLevelImpl();
  }

private:

  // SFINAE helper to detect if VerboseImpl exists
  template <typename T, typename... Args>
  auto VerboseHelper(int, const char* format, Args&&... args) 
    -> decltype(std::declval<T>().VerboseImpl(format, std::forward<Args>(args)...), void())
  {
    static_cast<Derived*>(this)->VerboseImpl(format, std::forward<Args>(args)...);
  }
  
  template <typename T, typename... Args>
  void VerboseHelper(long, const char* format, Args&&... args) 
  {
    printf("VERBOSE: ");
    printf(format, std::forward<Args>(args)...);
    printf("\n");
  }

  // SFINAE helper to detect if DebugImpl exists
  template <typename T, typename... Args>
  auto DebugHelper(int, const char* format, Args&&... args) 
    -> decltype(std::declval<T>().DebugImpl(format, std::forward<Args>(args)...), void())
  {
    static_cast<Derived*>(this)->DebugImpl(format, std::forward<Args>(args)...);
  }
  
  template <typename T, typename... Args>
  void DebugHelper(long, const char* format, Args&&... args) 
  {
    printf("DEBUG: ");
    printf(format, std::forward<Args>(args)...);
    printf("\n");
  }

  // SFINAE helper to detect if InfoImpl exists
  template <typename T, typename... Args>
  auto InfoHelper(int, const char* format, Args&&... args) 
    -> decltype(std::declval<T>().InfoImpl(format, std::forward<Args>(args)...), void())
  {
    static_cast<Derived*>(this)->InfoImpl(format, std::forward<Args>(args)...);
  }
  
  template <typename T, typename... Args>
  void InfoHelper(long, const char* format, Args&&... args) 
  {
    printf("INFO: ");
    printf(format, std::forward<Args>(args)...);
    printf("\n");
  }

  // SFINAE helper to detect if WarningImpl exists
  template <typename T, typename... Args>
  auto WarningHelper(int, const char* format, Args&&... args) 
    -> decltype(std::declval<T>().WarningImpl(format, std::forward<Args>(args)...), void())
  {
    static_cast<Derived*>(this)->WarningImpl(format, std::forward<Args>(args)...);
  }
  
  template <typename T, typename... Args>
  void WarningHelper(long, const char* format, Args&&... args) 
  {
    printf("WARNING: ");
    printf(format, std::forward<Args>(args)...);
    printf("\n");
  }

  // SFINAE helper to detect if ErrorImpl exists
  template <typename T, typename... Args>
  auto ErrorHelper(int, const char* format, Args&&... args) 
    -> decltype(std::declval<T>().ErrorImpl(format, std::forward<Args>(args)...), void())
  {
    static_cast<Derived*>(this)->ErrorImpl(format, std::forward<Args>(args)...);
  }
  
  template <typename T, typename... Args>
  void ErrorHelper(long, const char* format, Args&&... args) 
  {
    printf("ERROR: ");
    printf(format, std::forward<Args>(args)...);
    printf("\n");
  }

  // SFINAE helper to detect if FatalImpl exists
  template <typename T, typename... Args>
  auto FatalHelper(int, const char* format, Args&&... args) 
    -> decltype(std::declval<T>().FatalImpl(format, std::forward<Args>(args)...), void())
  {
    static_cast<Derived*>(this)->FatalImpl(format, std::forward<Args>(args)...);
  }
  
  template <typename T, typename... Args>
  void FatalHelper(long, const char* format, Args&&... args) 
  {
    printf("FATAL: ");
    printf(format, std::forward<Args>(args)...);
    printf("\n");
  }

public:
  template <typename... Args>
  void Verbose(const char* format, Args&&... args)
  {
    VerboseHelper<Derived>(0, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void Debug(const char* format, Args&&... args)
  {
    DebugHelper<Derived>(0, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void Info(const char* format, Args&&... args)
  {
    InfoHelper<Derived>(0, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void Warning(const char* format, Args&&... args)
  {
    WarningHelper<Derived>(0, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void Error(const char* format, Args&&... args)
  {
    ErrorHelper<Derived>(0, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void Fatal(const char* format, Args&&... args)
  {
    FatalHelper<Derived>(0, format, std::forward<Args>(args)...);
  }
};

} // namespace osal