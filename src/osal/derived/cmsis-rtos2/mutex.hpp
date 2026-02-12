#pragma once

#include "osal/ability/mutex.hpp"
#include "cmsis_os2.h"

namespace ifce::os {

class Mutex : public MutexAbility<Mutex>
{
  friend class MutexAbility<Mutex>;
  friend class ifce::DispatchBase<Mutex>;

public:
  Mutex()  = default;
  ~Mutex() { DeleteImpl(); }

private:
  OsStatus CreateImpl(bool recursive)
  {
    if (id_) return OsStatus::Busy;
    osMutexAttr_t attr = {};
    attr.attr_bits = osMutexPrioInherit;
    if (recursive)
      attr.attr_bits |= osMutexRecursive;
    id_ = osMutexNew(&attr);
    return id_ ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus DeleteImpl()
  {
    if (!id_) return OsStatus::Ok;
    osStatus_t rc = osMutexDelete(id_);
    id_ = nullptr;
    return (rc == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus LockImpl(uint32_t timeout_ms)
  {
    if (!id_) return OsStatus::Error;
    uint32_t ticks = (timeout_ms == WaitForever) ? osWaitForever : timeout_ms;
    osStatus_t rc = osMutexAcquire(id_, ticks);
    if (rc == osOK)             return OsStatus::Ok;
    if (rc == osErrorTimeout)   return OsStatus::Timeout;
    return OsStatus::Error;
  }

  OsStatus UnlockImpl()
  {
    if (!id_) return OsStatus::Error;
    return (osMutexRelease(id_) == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus TryLockImpl()
  {
    return LockImpl(0);
  }

public:
  osMutexId_t GetHandle() const { return id_; }

private:
  osMutexId_t id_ = nullptr;
};

} // namespace ifce::os
