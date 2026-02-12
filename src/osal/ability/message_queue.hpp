#pragma once

#include "osal/types.hpp"
#include "osal/ability/dispatch.hpp"
#include <cstdint>

namespace ifce::os {

template <typename Derived, typename T>
class MessageQueueAbility : protected ifce::DispatchBase<Derived>
{
  friend Derived;
  using Base = ifce::DispatchBase<Derived>;

public:
  MessageQueueAbility()  = default;
  ~MessageQueueAbility() = default;

  MessageQueueAbility(const MessageQueueAbility&)            = delete;
  MessageQueueAbility& operator=(const MessageQueueAbility&) = delete;

  // --- Mandatory ---

  OsStatus Create(uint32_t capacity)
  {
    return Base::Invoke(
      [](auto* s, uint32_t c) -> decltype(s->CreateImpl(c)) { return s->CreateImpl(c); },
      capacity);
  }

  OsStatus Delete()
  {
    return Base::Invoke(
      [](auto* s) -> decltype(s->DeleteImpl()) { return s->DeleteImpl(); });
  }

  OsStatus Put(const T& msg, uint32_t timeout_ms = WaitForever)
  {
    return Base::Invoke(
      [](auto* s, const T& m, uint32_t t) -> decltype(s->PutImpl(m, t)) {
        return s->PutImpl(m, t);
      }, msg, timeout_ms);
  }

  OsStatus Get(T& msg, uint32_t timeout_ms = WaitForever)
  {
    return Base::Invoke(
      [](auto* s, T& m, uint32_t t) -> decltype(s->GetImpl(m, t)) {
        return s->GetImpl(m, t);
      }, msg, timeout_ms);
  }

  // --- Optional ---

  OsStatus PutToFront(const T& msg, uint32_t timeout_ms = WaitForever)
  {
    return Base::QueryMut(OsStatus::Error,
      [](auto* s, const T& m, uint32_t t) -> decltype(s->PutToFrontImpl(m, t)) {
        return s->PutToFrontImpl(m, t);
      }, msg, timeout_ms);
  }

  uint32_t GetCount() const
  {
    return Base::Query(uint32_t(0),
      [](const auto* s) -> decltype(s->GetCountImpl()) { return s->GetCountImpl(); });
  }

  uint32_t GetCapacity() const
  {
    return Base::Query(uint32_t(0),
      [](const auto* s) -> decltype(s->GetCapacityImpl()) { return s->GetCapacityImpl(); });
  }

  OsStatus Reset()
  {
    return Base::QueryMut(OsStatus::Error,
      [](auto* s) -> decltype(s->ResetImpl()) { return s->ResetImpl(); });
  }
};

} // namespace ifce::os
