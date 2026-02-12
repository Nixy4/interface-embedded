#pragma once

#include "osal/types.hpp"
#include "osal/ability/dispatch.hpp"
#include <cstdint>
#include <functional>

namespace ifce::os {

template <typename Derived>
class ThreadAbility : protected ifce::DispatchBase<Derived>
{
  friend Derived;
  using Base = ifce::DispatchBase<Derived>;

public:
  ThreadAbility()  = default;
  ~ThreadAbility() = default;

  // Disable copy
  ThreadAbility(const ThreadAbility&)            = delete;
  ThreadAbility& operator=(const ThreadAbility&) = delete;

  // --- Mandatory ---

  OsStatus Create(const char* name, ThreadFunc fn, void* arg,
                   uint32_t stack_size, ThreadPriority priority)
  {
    return Base::Invoke(
      [](auto* s, const char* n, ThreadFunc f, void* a, uint32_t ss, ThreadPriority p)
        -> decltype(s->CreateImpl(n, std::move(f), a, ss, p)) {
          return s->CreateImpl(n, std::move(f), a, ss, p);
      }, name, std::move(fn), arg, stack_size, priority);
  }

  OsStatus Terminate()
  {
    return Base::Invoke(
      [](auto* s) -> decltype(s->TerminateImpl()) { return s->TerminateImpl(); });
  }

  // --- Optional ---

  OsStatus Suspend()
  {
    return Base::QueryMut(OsStatus::Error,
      [](auto* s) -> decltype(s->SuspendImpl()) { return s->SuspendImpl(); });
  }

  OsStatus Resume()
  {
    return Base::QueryMut(OsStatus::Error,
      [](auto* s) -> decltype(s->ResumeImpl()) { return s->ResumeImpl(); });
  }

  OsStatus SetPriority(ThreadPriority priority)
  {
    return Base::QueryMut(OsStatus::Error,
      [](auto* s, ThreadPriority p) -> decltype(s->SetPriorityImpl(p)) {
        return s->SetPriorityImpl(p);
      }, priority);
  }

  ThreadPriority GetPriority() const
  {
    return Base::Query(ThreadPriority::Normal,
      [](const auto* s) -> decltype(s->GetPriorityImpl()) {
        return s->GetPriorityImpl();
      });
  }

  const char* GetName() const
  {
    return Base::Query(static_cast<const char*>(""),
      [](const auto* s) -> decltype(s->GetNameImpl()) { return s->GetNameImpl(); });
  }

  uint32_t GetStackSize() const
  {
    return Base::Query(uint32_t(0),
      [](const auto* s) -> decltype(s->GetStackSizeImpl()) { return s->GetStackSizeImpl(); });
  }

  OsStatus SetAffinity(int core)
  {
    return Base::QueryMut(OsStatus::Error,
      [](auto* s, int c) -> decltype(s->SetAffinityImpl(c)) {
        return s->SetAffinityImpl(c);
      }, core);
  }

  OsStatus Join()
  {
    return Base::QueryMut(OsStatus::Error,
      [](auto* s) -> decltype(s->JoinImpl()) { return s->JoinImpl(); });
  }

  OsStatus Detach()
  {
    return Base::QueryMut(OsStatus::Error,
      [](auto* s) -> decltype(s->DetachImpl()) { return s->DetachImpl(); });
  }

  bool IsRunning() const
  {
    return Base::Query(false,
      [](const auto* s) -> decltype(s->IsRunningImpl()) { return s->IsRunningImpl(); });
  }
};

} // namespace ifce::os
