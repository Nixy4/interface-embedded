#pragma once
#include <type_traits>
#include <thread>
#include <chrono>
#include <utility>
#include <atomic>

// Normalized task priority levels (0-100)
enum class TaskPriority
{
  IDLE = 0,
  LOWEST = 10,
  LOW = 25,
  BELOW_NORMAL = 40,
  NORMAL = 50,
  ABOVE_NORMAL = 60,
  HIGH = 75,
  HIGHEST = 90,
  CRITICAL = 99
};

template <typename Derived>
class Task
{
  friend Derived;

protected:
  int priority_{0};
  std::atomic<bool> stop_requested_{false};
  
  Task() = default;
  ~Task() = default;
  
  // Disable copy
  Task(const Task&) = delete;
  Task& operator=(const Task&) = delete;
  
  // Enable move
  Task(Task&&) = default;
  Task& operator=(Task&&) = default;

public:
  void Initialize()
  {
    InitializeHelper<Derived>(0);
  }

  void Run()
  {
    RunHelper<Derived>(0);
  }

  void Stop()
  {
    StopHelper<Derived>(0);
  }

  // Request cooperative stop (non-blocking)
  void RequestStop()
  {
    stop_requested_.store(true, std::memory_order_release);
  }

  // Check if stop has been requested (for cooperative cancellation)
  bool ShouldStop() const
  {
    return stop_requested_.load(std::memory_order_acquire);
  }

  void SetPriority(int p)
  {
    SetPriorityHelper<Derived>(0, p);
  }

  void SetPriority(TaskPriority p)
  {
    SetPriority(static_cast<int>(p));
  }

  int GetPriority()
  {
    return GetPriorityHelper<Derived>(0);
  }

  void SleepMs(unsigned ms)
  {
    SleepMsHelper<Derived>(0, ms);
  }

  template <typename F>
  void SetEntry(F&& fn)
  {
    SetEntryHelper<Derived>(0, std::forward<F>(fn));
  }

  template <typename S>
  void SetName(S&& name)
  {
    SetNameHelper<Derived>(0, std::forward<S>(name));
  }

  bool SetStackSize(unsigned bytes)
  {
    return SetStackSizeHelper<Derived>(0, bytes);
  }

  bool SetCore(int core)
  {
    return SetCoreHelper<Derived>(0, core);
  }

  bool IsRunning()
  {
    return IsRunningHelper<Derived>(0);
  }

  void Join()
  {
    JoinHelper<Derived>(0);
  }

  void Detach()
  {
    DetachHelper<Derived>(0);
  }

  // Helper: Map normalized priority (0-100) to platform-specific range
  static int MapPriority(int normalized, int platform_min, int platform_max)
  {
    // Clamp to valid range
    if (normalized < 0) normalized = 0;
    if (normalized > 100) normalized = 100;
    
    // Linear mapping
    return platform_min + (normalized * (platform_max - platform_min)) / 100;
  }


private:
  template <typename T>
  auto InitializeHelper(int) -> decltype(std::declval<T>().InitializeImpl(), void())
  {
    static_cast<Derived*>(this)->InitializeImpl();
  }
  template <typename T>
  void InitializeHelper(long) {}

  template <typename T>
  auto RunHelper(int) -> decltype(std::declval<T>().RunImpl(), void())
  {
    static_cast<Derived*>(this)->RunImpl();
  }
  template <typename T>
  void RunHelper(long) {}

  template <typename T>
  auto StopHelper(int) -> decltype(std::declval<T>().StopImpl(), void())
  {
    static_cast<Derived*>(this)->StopImpl();
  }
  template <typename T>
  void StopHelper(long) {}

  template <typename T>
  auto SetPriorityHelper(int, int p) -> decltype(std::declval<T>().SetPriorityImpl(p), void())
  {
    static_cast<Derived*>(this)->SetPriorityImpl(p);
  }
  template <typename T>
  void SetPriorityHelper(long, int p)
  {
    priority_ = p;
  }

  template <typename T>
  auto GetPriorityHelper(int) -> decltype(std::declval<T>().GetPriorityImpl())
  {
    return static_cast<Derived*>(this)->GetPriorityImpl();
  }
  template <typename T>
  int GetPriorityHelper(long)
  {
    return priority_;
  }

  template <typename T>
  auto SleepMsHelper(int, unsigned ms) -> decltype(std::declval<T>().SleepMsImpl(ms), void())
  {
    static_cast<Derived*>(this)->SleepMsImpl(ms);
  }
  template <typename T>
  void SleepMsHelper(long, unsigned)
  {
    static_assert(sizeof(T) == 0, 
      "SleepMs requires platform-specific implementation in derived class. "
      "Do not rely on std::this_thread::sleep_for in RTOS environment.");
  }

  template <typename T, typename F>
  auto SetEntryHelper(int, F&& fn) -> decltype(std::declval<T>().SetEntryImpl(std::forward<F>(fn)), void())
  {
    static_cast<Derived*>(this)->SetEntryImpl(std::forward<F>(fn));
  }
  template <typename T, typename F>
  void SetEntryHelper(long, F&&) {}

  template <typename T, typename S>
  auto SetNameHelper(int, S&& name) -> decltype(std::declval<T>().SetNameImpl(std::forward<S>(name)), void())
  {
    static_cast<Derived*>(this)->SetNameImpl(std::forward<S>(name));
  }
  template <typename T, typename S>
  void SetNameHelper(long, S&&) {}

  template <typename T>
  auto SetStackSizeHelper(int, unsigned bytes) -> decltype(std::declval<T>().SetStackSizeImpl(bytes))
  {
    return static_cast<Derived*>(this)->SetStackSizeImpl(bytes);
  }
  template <typename T>
  bool SetStackSizeHelper(long, unsigned)
  {
    // Default: not supported
    return false;
  }

  template <typename T>
  auto SetCoreHelper(int, int core) -> decltype(std::declval<T>().SetCoreImpl(core))
  {
    return static_cast<Derived*>(this)->SetCoreImpl(core);
  }
  template <typename T>
  bool SetCoreHelper(long, int)
  {
    // Default: not supported
    return false;
  }

  template <typename T>
  auto IsRunningHelper(int) -> decltype(std::declval<T>().IsRunningImpl())
  {
    return static_cast<Derived*>(this)->IsRunningImpl();
  }
  template <typename T>
  bool IsRunningHelper(long)
  {
    return false;
  }

  template <typename T>
  auto JoinHelper(int) -> decltype(std::declval<T>().JoinImpl(), void())
  {
    static_cast<Derived*>(this)->JoinImpl();
  }
  template <typename T>
  void JoinHelper(long) {}

  template <typename T>
  auto DetachHelper(int) -> decltype(std::declval<T>().DetachImpl(), void())
  {
    static_cast<Derived*>(this)->DetachImpl();
  }
  template <typename T>
  void DetachHelper(long) {}
};
