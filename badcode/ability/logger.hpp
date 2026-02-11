#pragma once

namespace interface
{
  template <typename Derived>
  class LoggerAbility
  {
    friend Derived;
protected:
    template <typename T, typename... Args>
    auto VerboseHelper(int, const char *format, Args &&...args)
        -> decltype(std::declval<T>().VerboseImpl(format, std::forward<Args>(args)...), void())
    {
      static_cast<Derived *>(this)->VerboseImpl(format, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    void VerboseHelper(long, const char *format, Args &&...args)
    {
      printf("VERBOSE: ");
      printf(format, std::forward<Args>(args)...);
      printf("\n");
    }

    template <typename T, typename... Args>
    auto DebugHelper(int, const char *format, Args &&...args)
        -> decltype(std::declval<T>().DebugImpl(format, std::forward<Args>(args)...), void())
    {
      static_cast<Derived *>(this)->DebugImpl(format, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    void DebugHelper(long, const char *format, Args &&...args)
    {
      printf("DEBUG: ");
      printf(format, std::forward<Args>(args)...);
      printf("\n");
    }

    template <typename T, typename... Args>
    auto InfoHelper(int, const char *format, Args &&...args)
        -> decltype(std::declval<T>().InfoImpl(format, std::forward<Args>(args)...), void())
    {
      static_cast<Derived *>(this)->InfoImpl(format, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    void InfoHelper(long, const char *format, Args &&...args)
    {
      printf("INFO: ");
      printf(format, std::forward<Args>(args)...);
      printf("\n");
    }

    template <typename T, typename... Args>
    auto WarningHelper(int, const char *format, Args &&...args)
        -> decltype(std::declval<T>().WarningImpl(format, std::forward<Args>(args)...), void())
    {
      static_cast<Derived *>(this)->WarningImpl(format, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    void WarningHelper(long, const char *format, Args &&...args)
    {
      printf("WARNING: ");
      printf(format, std::forward<Args>(args)...);
      printf("\n");
    }

    template <typename T, typename... Args>
    auto ErrorHelper(int, const char *format, Args &&...args)
        -> decltype(std::declval<T>().ErrorImpl(format, std::forward<Args>(args)...), void())
    {
      static_cast<Derived *>(this)->ErrorImpl(format, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    void ErrorHelper(long, const char *format, Args &&...args)
    {
      printf("ERROR: ");
      printf(format, std::forward<Args>(args)...);
      printf("\n");
    }

    template <typename T, typename... Args>
    auto FatalHelper(int, const char *format, Args &&...args)
        -> decltype(std::declval<T>().FatalImpl(format, std::forward<Args>(args)...), void())
    {
      static_cast<Derived *>(this)->FatalImpl(format, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    void FatalHelper(long, const char *format, Args &&...args)
    {
      printf("FATAL: ");
      printf(format, std::forward<Args>(args)...);
      printf("\n");
    }

  public:
    template <typename... Args>
    void Verbose(const char *format, Args &&...args)
    {
      VerboseHelper<Derived>(0, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Debug(const char *format, Args &&...args)
    {
      DebugHelper<Derived>(0, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Info(const char *format, Args &&...args)
    {
      InfoHelper<Derived>(0, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Warning(const char *format, Args &&...args)
    {
      WarningHelper<Derived>(0, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Error(const char *format, Args &&...args)
    {
      ErrorHelper<Derived>(0, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void Fatal(const char *format, Args &&...args)
    {
      FatalHelper<Derived>(0, format, std::forward<Args>(args)...);
    }
  };
}