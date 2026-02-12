#pragma once

#include "osal/types.hpp"
#include "osal/ability/dispatch.hpp"
#include <cstdint>

namespace osal {

template <typename Derived>
class EventFlagsAbility : protected DispatchBase<Derived>
{
  friend Derived;
  using Base = DispatchBase<Derived>;

public:
  EventFlagsAbility()  = default;
  ~EventFlagsAbility() = default;

  EventFlagsAbility(const EventFlagsAbility&)            = delete;
  EventFlagsAbility& operator=(const EventFlagsAbility&) = delete;

  // --- Mandatory ---

  OsStatus Create()
  {
    return Base::Invoke(
      [](auto* s) -> decltype(s->CreateImpl()) { return s->CreateImpl(); });
  }

  OsStatus Delete()
  {
    return Base::Invoke(
      [](auto* s) -> decltype(s->DeleteImpl()) { return s->DeleteImpl(); });
  }

  uint32_t Set(uint32_t flags)
  {
    return Base::Invoke(
      [](auto* s, uint32_t f) -> decltype(s->SetImpl(f)) { return s->SetImpl(f); },
      flags);
  }

  uint32_t Clear(uint32_t flags)
  {
    return Base::Invoke(
      [](auto* s, uint32_t f) -> decltype(s->ClearImpl(f)) { return s->ClearImpl(f); },
      flags);
  }

  uint32_t Wait(uint32_t flags, bool wait_all, bool auto_clear,
                uint32_t timeout_ms = WaitForever)
  {
    return Base::Invoke(
      [](auto* s, uint32_t f, bool wa, bool ac, uint32_t t)
        -> decltype(s->WaitImpl(f, wa, ac, t)) {
          return s->WaitImpl(f, wa, ac, t);
      }, flags, wait_all, auto_clear, timeout_ms);
  }

  // --- Optional ---

  uint32_t Get() const
  {
    return Base::Query(uint32_t(0),
      [](const auto* s) -> decltype(s->GetImpl()) { return s->GetImpl(); });
  }
};

} // namespace osal
