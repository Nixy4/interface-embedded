#pragma once

#include "osal/types.hpp"
#include "osal/ability/dispatch.hpp"
#include <cstdint>
#include <functional>

namespace ifce::os {

template <typename Derived>
class TimerAbility : protected ifce::DispatchBase<Derived>
{
  friend Derived;
  using Base = ifce::DispatchBase<Derived>;

public:
  TimerAbility()  = default;
  ~TimerAbility() = default;

  TimerAbility(const TimerAbility&)            = delete;
  TimerAbility& operator=(const TimerAbility&) = delete;

  // --- Mandatory ---

  OsStatus Create(const char* name, TimerFunc callback, void* arg,
                   uint32_t period_ms, bool auto_reload)
  {
    return Base::Invoke(
      [](auto* s, const char* n, TimerFunc cb, void* a, uint32_t p, bool ar)
        -> decltype(s->CreateImpl(n, std::move(cb), a, p, ar)) {
          return s->CreateImpl(n, std::move(cb), a, p, ar);
      }, name, std::move(callback), arg, period_ms, auto_reload);
  }

  OsStatus Delete()
  {
    return Base::Invoke(
      [](auto* s) -> decltype(s->DeleteImpl()) { return s->DeleteImpl(); });
  }

  OsStatus Start()
  {
    return Base::Invoke(
      [](auto* s) -> decltype(s->StartImpl()) { return s->StartImpl(); });
  }

  OsStatus Stop()
  {
    return Base::Invoke(
      [](auto* s) -> decltype(s->StopImpl()) { return s->StopImpl(); });
  }

  // --- Optional ---

  OsStatus SetPeriod(uint32_t period_ms)
  {
    return Base::QueryMut(OsStatus::Error,
      [](auto* s, uint32_t p) -> decltype(s->SetPeriodImpl(p)) {
        return s->SetPeriodImpl(p);
      }, period_ms);
  }

  bool IsRunning() const
  {
    return Base::Query(false,
      [](const auto* s) -> decltype(s->IsRunningImpl()) { return s->IsRunningImpl(); });
  }
};

} // namespace ifce::os
