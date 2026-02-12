#pragma once

#include "osal/ability/mutex.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

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
    if (handle_) return OsStatus::Busy;
    recursive_ = recursive;
    handle_ = recursive_ ? xSemaphoreCreateRecursiveMutex()
                          : xSemaphoreCreateMutex();
    return handle_ ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus DeleteImpl()
  {
    if (!handle_) return OsStatus::Ok;
    vSemaphoreDelete(handle_);
    handle_ = nullptr;
    return OsStatus::Ok;
  }

  OsStatus LockImpl(uint32_t timeout_ms)
  {
    if (!handle_) return OsStatus::Error;
    TickType_t ticks = (timeout_ms == WaitForever) ? portMAX_DELAY
                       : pdMS_TO_TICKS(timeout_ms);
    BaseType_t rc = recursive_ ? xSemaphoreTakeRecursive(handle_, ticks)
                                : xSemaphoreTake(handle_, ticks);
    return (rc == pdTRUE) ? OsStatus::Ok : OsStatus::Timeout;
  }

  OsStatus UnlockImpl()
  {
    if (!handle_) return OsStatus::Error;
    BaseType_t rc = recursive_ ? xSemaphoreGiveRecursive(handle_)
                                : xSemaphoreGive(handle_);
    return (rc == pdTRUE) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus TryLockImpl()
  {
    return LockImpl(0);
  }

public:
  // FreeRTOS-specific ISR helpers
  OsStatus GiveFromISR(BaseType_t* pxHigherPriorityTaskWoken = nullptr)
  {
    if (!handle_ || recursive_) return OsStatus::Error;
    BaseType_t dummy = pdFALSE;
    BaseType_t* p = pxHigherPriorityTaskWoken ? pxHigherPriorityTaskWoken : &dummy;
    return (xSemaphoreGiveFromISR(handle_, p) == pdTRUE) ? OsStatus::Ok : OsStatus::Error;
  }

  SemaphoreHandle_t GetHandle() const { return handle_; }

private:
  SemaphoreHandle_t handle_    = nullptr;
  bool              recursive_ = false;
};

} // namespace ifce::os
