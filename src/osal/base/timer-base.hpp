#pragma once
#include <type_traits>
#include <cstdint>
#include <utility>

template <typename Derived>
class TimerBase
{
  friend Derived;
protected:
  TimerBase() = default;
  ~TimerBase() = default;
  
  // Disable copy
  TimerBase(const TimerBase&) = delete;
  TimerBase& operator=(const TimerBase&) = delete;
  
  // Enable move
  TimerBase(TimerBase&&) = default;
  TimerBase& operator=(TimerBase&&) = default;

public:
  void Initialize(unsigned period_ms, bool auto_reload)
  {
    InitializeHelper<Derived>(0, period_ms, auto_reload);
  }

  void Start()
  {
    StartHelper<Derived>(0);
  }

  void Stop()
  {
    StopHelper<Derived>(0);
  }

  void Reset()
  {
    ResetHelper<Derived>(0);
  }

  void SetPeriod(unsigned period_ms)
  {
    SetPeriodHelper<Derived>(0, period_ms);
  }

  unsigned GetPeriod()
  {
    return GetPeriodHelper<Derived>(0);
  }

  bool IsRunning()
  {
    return IsRunningHelper<Derived>(0);
  }

  void Tick()
  {
    TickHelper<Derived>(0);
  }

  void StartOnce()
  {
    StartOnceHelper<Derived>(0);
  }

  template <typename F>
  void SetCallback(F&& cb)
  {
    SetCallbackHelper<Derived>(0, std::forward<F>(cb));
  }

  void SetCallback(void (*cb)(void*), void* user)
  {
    SetCallbackCHelper<Derived>(0, cb, user);
  }

private:
  template <typename T>
  auto InitializeHelper(int, unsigned period_ms, bool auto_reload) 
    -> decltype(std::declval<T>().InitializeImpl(period_ms, auto_reload), void())
  {
    static_cast<Derived*>(this)->InitializeImpl(period_ms, auto_reload);
  }
  template <typename T>
  void InitializeHelper(long, unsigned, bool) {}

  template <typename T>
  auto StartHelper(int) -> decltype(std::declval<T>().StartImpl(), void())
  {
    static_cast<Derived*>(this)->StartImpl();
  }
  template <typename T>
  void StartHelper(long) {}

  template <typename T>
  auto StopHelper(int) -> decltype(std::declval<T>().StopImpl(), void())
  {
    static_cast<Derived*>(this)->StopImpl();
  }
  template <typename T>
  void StopHelper(long) {}

  template <typename T>
  auto ResetHelper(int) -> decltype(std::declval<T>().ResetImpl(), void())
  {
    static_cast<Derived*>(this)->ResetImpl();
  }
  template <typename T>
  void ResetHelper(long) {}

  template <typename T>
  auto SetPeriodHelper(int, unsigned ms) -> decltype(std::declval<T>().SetPeriodImpl(ms), void())
  {
    static_cast<Derived*>(this)->SetPeriodImpl(ms);
  }
  template <typename T>
  void SetPeriodHelper(long, unsigned) {}

  template <typename T>
  auto GetPeriodHelper(int) -> decltype(std::declval<T>().GetPeriodImpl())
  {
    return static_cast<Derived*>(this)->GetPeriodImpl();
  }
  template <typename T>
  unsigned GetPeriodHelper(long)
  {
    return 0;
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
  auto TickHelper(int) -> decltype(std::declval<T>().TickImpl(), void())
  {
    static_cast<Derived*>(this)->TickImpl();
  }
  template <typename T>
  void TickHelper(long) {}

  template <typename T>
  auto StartOnceHelper(int) -> decltype(std::declval<T>().StartOnceImpl(), void())
  {
    static_cast<Derived*>(this)->StartOnceImpl();
  }
  template <typename T>
  void StartOnceHelper(long)
  {
    Start();
  }

  template <typename T, typename F>
  auto SetCallbackHelper(int, F&& cb) 
    -> decltype(std::declval<T>().SetCallbackImpl(std::forward<F>(cb)), void())
  {
    static_cast<Derived*>(this)->SetCallbackImpl(std::forward<F>(cb));
  }
  template <typename T, typename F>
  void SetCallbackHelper(long, F&&) {}

  template <typename T>
  auto SetCallbackCHelper(int, void (*cb)(void*), void* user)
    -> decltype(std::declval<T>().SetCallbackImpl(cb, user), void())
  {
    static_cast<Derived*>(this)->SetCallbackImpl(cb, user);
  }
  template <typename T>
  void SetCallbackCHelper(long, void (*)(void*), void*) {}
};
