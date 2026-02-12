#pragma once

#include "osal/ability/semaphore.hpp"
#include "cmsis_os2.h"

namespace osal {

class Semaphore : public SemaphoreAbility<Semaphore>
{
  friend class SemaphoreAbility<Semaphore>;
  friend class DispatchBase<Semaphore>;

public:
  Semaphore()  = default;
  ~Semaphore() { DeleteImpl(); }

private:
  OsStatus CreateImpl(uint32_t max_count, uint32_t initial_count)
  {
    if (id_) return OsStatus::Busy;
    id_ = osSemaphoreNew(max_count, initial_count, nullptr);
    return id_ ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus DeleteImpl()
  {
    if (!id_) return OsStatus::Ok;
    osStatus_t rc = osSemaphoreDelete(id_);
    id_ = nullptr;
    return (rc == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus AcquireImpl(uint32_t timeout_ms)
  {
    if (!id_) return OsStatus::Error;
    uint32_t ticks = (timeout_ms == WaitForever) ? osWaitForever : timeout_ms;
    osStatus_t rc = osSemaphoreAcquire(id_, ticks);
    if (rc == osOK)           return OsStatus::Ok;
    if (rc == osErrorTimeout) return OsStatus::Timeout;
    return OsStatus::Error;
  }

  OsStatus ReleaseImpl()
  {
    if (!id_) return OsStatus::Error;
    return (osSemaphoreRelease(id_) == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  uint32_t GetCountImpl() const
  {
    if (!id_) return 0;
    return osSemaphoreGetCount(id_);
  }

public:
  osSemaphoreId_t GetHandle() const { return id_; }

private:
  osSemaphoreId_t id_ = nullptr;
};

} // namespace osal
