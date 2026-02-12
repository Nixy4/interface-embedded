#pragma once

#include "osal/ability/memory_pool.hpp"
#include "cmsis_os2.h"

namespace osal {

template <typename T>
class MemoryPool : public MemoryPoolAbility<MemoryPool<T>, T>
{
  friend class MemoryPoolAbility<MemoryPool<T>, T>;
  friend class DispatchBase<MemoryPool<T>>;

public:
  MemoryPool()  = default;
  ~MemoryPool() { DeleteImpl(); }

private:
  OsStatus CreateImpl(uint32_t block_count)
  {
    if (id_) return OsStatus::Busy;
    id_ = osMemoryPoolNew(block_count, sizeof(T), nullptr);
    return id_ ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus DeleteImpl()
  {
    if (!id_) return OsStatus::Ok;
    osStatus_t rc = osMemoryPoolDelete(id_);
    id_ = nullptr;
    return (rc == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  T* AllocImpl(uint32_t timeout_ms)
  {
    if (!id_) return nullptr;
    uint32_t ticks = (timeout_ms == WaitForever) ? osWaitForever : timeout_ms;
    return static_cast<T*>(osMemoryPoolAlloc(id_, ticks));
  }

  OsStatus FreeImpl(T* block)
  {
    if (!id_ || !block) return OsStatus::Error;
    return (osMemoryPoolFree(id_, block) == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  uint32_t GetCountImpl() const
  {
    if (!id_) return 0;
    return osMemoryPoolGetCapacity(id_);
  }

  uint32_t GetFreeCountImpl() const
  {
    if (!id_) return 0;
    return osMemoryPoolGetSpace(id_);
  }

public:
  osMemoryPoolId_t GetHandle() const { return id_; }

private:
  osMemoryPoolId_t id_ = nullptr;
};

} // namespace osal
