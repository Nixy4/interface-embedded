#pragma once
#include "mutex-base.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace osal {

class MutexFreeRTOS : public MutexBase<MutexFreeRTOS>
{
public:
  MutexFreeRTOS() = default;
  ~MutexFreeRTOS()
  {
    if (handle_) {
      vSemaphoreDelete(handle_);
      handle_ = nullptr;
    }
  }

  MutexFreeRTOS(const MutexFreeRTOS&) = delete;
  MutexFreeRTOS& operator=(const MutexFreeRTOS&) = delete;

  MutexFreeRTOS(MutexFreeRTOS&& other) noexcept
    : handle_(other.handle_)
  {
    other.handle_ = nullptr;
  }
  MutexFreeRTOS& operator=(MutexFreeRTOS&& other) noexcept
  {
    if (this != &other) {
      if (handle_) vSemaphoreDelete(handle_);
      handle_ = other.handle_;
      other.handle_ = nullptr;
    }
    return *this;
  }

  void InitializeImpl()
  {
    if (!handle_) {
      handle_ = recursive_ ? xSemaphoreCreateRecursiveMutex() : xSemaphoreCreateMutex();
    }
  }

  void LockImpl()
  {
    if (!handle_) return;
    if (recursive_) {
      xSemaphoreTakeRecursive(handle_, portMAX_DELAY);
    } else {
      xSemaphoreTake(handle_, portMAX_DELAY);
    }
  }

  void UnlockImpl()
  {
    if (!handle_) return;
    if (recursive_) {
      xSemaphoreGiveRecursive(handle_);
    } else {
      xSemaphoreGive(handle_);
    }
  }

  bool TryLockImpl()
  {
    if (!handle_) return false;
    if (recursive_) {
      return xSemaphoreTakeRecursive(handle_, 0) == pdTRUE;
    } else {
      return xSemaphoreTake(handle_, 0) == pdTRUE;
    }
  }

  bool TryLockForImpl(unsigned ms)
  {
    if (!handle_) return false;
    TickType_t to = (ms == 0) ? 0 : pdMS_TO_TICKS(ms);
    if (ms == (unsigned)UINT32_MAX) to = portMAX_DELAY;
    if (recursive_) {
      return xSemaphoreTakeRecursive(handle_, to) == pdTRUE;
    } else {
      return xSemaphoreTake(handle_, to) == pdTRUE;
    }
  }

  void SetRecursiveImpl(bool enable)
  {
    if (recursive_ == enable) return;
    recursive_ = enable;
    if (handle_) {
      vSemaphoreDelete(handle_);
      handle_ = recursive_ ? xSemaphoreCreateRecursiveMutex() : xSemaphoreCreateMutex();
    }
  }

private:
  SemaphoreHandle_t handle_{nullptr};
  bool recursive_{false};
};

} // namespace osal
