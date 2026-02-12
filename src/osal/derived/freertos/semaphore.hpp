#pragma once

#include "osal/ability/semaphore.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

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
    if (handle_) return OsStatus::Busy;
    if (max_count == 1 && initial_count <= 1) {
      handle_ = xSemaphoreCreateBinary();
      if (handle_ && initial_count == 1)
        xSemaphoreGive(handle_);
    } else {
      handle_ = xSemaphoreCreateCounting(max_count, initial_count);
    }
    return handle_ ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus DeleteImpl()
  {
    if (!handle_) return OsStatus::Ok;
    vSemaphoreDelete(handle_);
    handle_ = nullptr;
    return OsStatus::Ok;
  }

  OsStatus AcquireImpl(uint32_t timeout_ms)
  {
    if (!handle_) return OsStatus::Error;
    TickType_t ticks = (timeout_ms == WaitForever) ? portMAX_DELAY
                       : pdMS_TO_TICKS(timeout_ms);
    return (xSemaphoreTake(handle_, ticks) == pdTRUE) ? OsStatus::Ok : OsStatus::Timeout;
  }

  OsStatus ReleaseImpl()
  {
    if (!handle_) return OsStatus::Error;
    return (xSemaphoreGive(handle_) == pdTRUE) ? OsStatus::Ok : OsStatus::Error;
  }

  uint32_t GetCountImpl() const
  {
    if (!handle_) return 0;
    return static_cast<uint32_t>(uxSemaphoreGetCount(handle_));
  }

public:
  // FreeRTOS-specific ISR helpers
  OsStatus AcquireFromISR(BaseType_t* pxHigherPriorityTaskWoken = nullptr)
  {
    if (!handle_) return OsStatus::Error;
    BaseType_t dummy = pdFALSE;
    BaseType_t* p = pxHigherPriorityTaskWoken ? pxHigherPriorityTaskWoken : &dummy;
    return (xSemaphoreTakeFromISR(handle_, p) == pdTRUE) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus ReleaseFromISR(BaseType_t* pxHigherPriorityTaskWoken = nullptr)
  {
    if (!handle_) return OsStatus::Error;
    BaseType_t dummy = pdFALSE;
    BaseType_t* p = pxHigherPriorityTaskWoken ? pxHigherPriorityTaskWoken : &dummy;
    return (xSemaphoreGiveFromISR(handle_, p) == pdTRUE) ? OsStatus::Ok : OsStatus::Error;
  }

  SemaphoreHandle_t GetHandle() const { return handle_; }

private:
  SemaphoreHandle_t handle_ = nullptr;
};

} // namespace osal
