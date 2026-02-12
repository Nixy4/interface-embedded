#pragma once

#include "osal/types.hpp"
#include "osal/ability/dispatch.hpp"
#include <cstdint>

namespace osal {

template <typename Derived>
class SemaphoreAbility : protected DispatchBase<Derived>
{
  friend Derived;
  using Base = DispatchBase<Derived>;

public:
  SemaphoreAbility()  = default;
  ~SemaphoreAbility() = default;

  SemaphoreAbility(const SemaphoreAbility&)            = delete;
  SemaphoreAbility& operator=(const SemaphoreAbility&) = delete;

  // --- Mandatory ---

  OsStatus Create(uint32_t max_count, uint32_t initial_count)
  {
    return Base::Invoke(
      [](auto* s, uint32_t m, uint32_t i) -> decltype(s->CreateImpl(m, i)) {
        return s->CreateImpl(m, i);
      }, max_count, initial_count);
  }

  OsStatus Delete()
  {
    return Base::Invoke(
      [](auto* s) -> decltype(s->DeleteImpl()) { return s->DeleteImpl(); });
  }

  OsStatus Acquire(uint32_t timeout_ms = WaitForever)
  {
    return Base::Invoke(
      [](auto* s, uint32_t t) -> decltype(s->AcquireImpl(t)) { return s->AcquireImpl(t); },
      timeout_ms);
  }

  OsStatus Release()
  {
    return Base::Invoke(
      [](auto* s) -> decltype(s->ReleaseImpl()) { return s->ReleaseImpl(); });
  }

  // --- Optional ---

  uint32_t GetCount() const
  {
    return Base::Query(uint32_t(0),
      [](const auto* s) -> decltype(s->GetCountImpl()) { return s->GetCountImpl(); });
  }
};

} // namespace osal
