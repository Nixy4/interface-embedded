#pragma once

#include "osal/types.hpp"
#include "osal/ability/dispatch.hpp"
#include <cstdint>

namespace ifce::os {

template <typename Derived>
class MutexAbility : protected ifce::DispatchBase<Derived>
{
  friend Derived;
  using Base = ifce::DispatchBase<Derived>;

public:
  MutexAbility()  = default;
  ~MutexAbility() = default;

  MutexAbility(const MutexAbility&)            = delete;
  MutexAbility& operator=(const MutexAbility&) = delete;

  // --- Mandatory ---

  OsStatus Create(bool recursive = false)
  {
    return Base::Invoke(
      [](auto* s, bool r) -> decltype(s->CreateImpl(r)) { return s->CreateImpl(r); },
      recursive);
  }

  OsStatus Delete()
  {
    return Base::Invoke(
      [](auto* s) -> decltype(s->DeleteImpl()) { return s->DeleteImpl(); });
  }

  OsStatus Lock(uint32_t timeout_ms = WaitForever)
  {
    return Base::Invoke(
      [](auto* s, uint32_t t) -> decltype(s->LockImpl(t)) { return s->LockImpl(t); },
      timeout_ms);
  }

  OsStatus Unlock()
  {
    return Base::Invoke(
      [](auto* s) -> decltype(s->UnlockImpl()) { return s->UnlockImpl(); });
  }

  // --- Optional ---

  OsStatus TryLock()
  {
    return Base::QueryMut(OsStatus::Error,
      [](auto* s) -> decltype(s->TryLockImpl()) { return s->TryLockImpl(); });
  }
};

} // namespace ifce::os
