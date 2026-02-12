#pragma once

#include "osal/types.hpp"
#include "osal/ability/dispatch.hpp"
#include <cstdint>
#include <cstddef>
#include <new>

namespace osal {

template <typename Derived, typename T>
class MemoryPoolAbility : protected DispatchBase<Derived>
{
  friend Derived;
  using Base = DispatchBase<Derived>;

public:
  MemoryPoolAbility()  = default;
  ~MemoryPoolAbility() = default;

  MemoryPoolAbility(const MemoryPoolAbility&)            = delete;
  MemoryPoolAbility& operator=(const MemoryPoolAbility&) = delete;

  // --- Mandatory ---

  OsStatus Create(uint32_t block_count)
  {
    return Base::Invoke(
      [](auto* s, uint32_t c) -> decltype(s->CreateImpl(c)) { return s->CreateImpl(c); },
      block_count);
  }

  OsStatus Delete()
  {
    return Base::Invoke(
      [](auto* s) -> decltype(s->DeleteImpl()) { return s->DeleteImpl(); });
  }

  T* Alloc(uint32_t timeout_ms = 0)
  {
    return Base::Invoke(
      [](auto* s, uint32_t t) -> decltype(s->AllocImpl(t)) { return s->AllocImpl(t); },
      timeout_ms);
  }

  OsStatus Free(T* block)
  {
    return Base::Invoke(
      [](auto* s, T* b) -> decltype(s->FreeImpl(b)) { return s->FreeImpl(b); },
      block);
  }

  // --- Optional ---

  uint32_t GetCount() const
  {
    return Base::Query(uint32_t(0),
      [](const auto* s) -> decltype(s->GetCountImpl()) { return s->GetCountImpl(); });
  }

  uint32_t GetFreeCount() const
  {
    return Base::Query(uint32_t(0),
      [](const auto* s) -> decltype(s->GetFreeCountImpl()) { return s->GetFreeCountImpl(); });
  }
};

} // namespace osal
