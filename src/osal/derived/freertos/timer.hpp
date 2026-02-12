#pragma once

#include "osal/ability/timer.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

namespace osal {

class Timer : public TimerAbility<Timer>
{
  friend class TimerAbility<Timer>;
  friend class DispatchBase<Timer>;

public:
  Timer()  = default;
  ~Timer() { DeleteImpl(); }

private:
  OsStatus CreateImpl(const char* name, TimerFunc callback, void* arg,
                       uint32_t period_ms, bool auto_reload)
  {
    if (handle_) return OsStatus::Busy;
    callback_    = std::move(callback);
    user_arg_    = arg;
    period_ms_   = period_ms;
    auto_reload_ = auto_reload;

    handle_ = xTimerCreate(
      name ? name : "timer",
      pdMS_TO_TICKS(period_ms),
      auto_reload ? pdTRUE : pdFALSE,
      this,
      &TimerCallback);

    return handle_ ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus DeleteImpl()
  {
    if (!handle_) return OsStatus::Ok;
    xTimerDelete(handle_, portMAX_DELAY);
    handle_ = nullptr;
    return OsStatus::Ok;
  }

  OsStatus StartImpl()
  {
    if (!handle_) return OsStatus::Error;
    return (xTimerStart(handle_, portMAX_DELAY) == pdPASS) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus StopImpl()
  {
    if (!handle_) return OsStatus::Error;
    return (xTimerStop(handle_, portMAX_DELAY) == pdPASS) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus SetPeriodImpl(uint32_t period_ms)
  {
    if (!handle_) return OsStatus::Error;
    period_ms_ = period_ms;
    return (xTimerChangePeriod(handle_, pdMS_TO_TICKS(period_ms), portMAX_DELAY) == pdPASS)
               ? OsStatus::Ok : OsStatus::Error;
  }

  bool IsRunningImpl() const
  {
    if (!handle_) return false;
    return xTimerIsTimerActive(handle_) != pdFALSE;
  }

public:
  TimerHandle_t GetHandle() const { return handle_; }

private:
  static void TimerCallback(TimerHandle_t xTimer)
  {
    auto* self = static_cast<Timer*>(pvTimerGetTimerID(xTimer));
    if (self && self->callback_)
      self->callback_(self->user_arg_);
  }

  TimerHandle_t handle_      = nullptr;
  TimerFunc     callback_    = nullptr;
  void*         user_arg_    = nullptr;
  uint32_t      period_ms_   = 0;
  bool          auto_reload_ = false;
};

} // namespace osal
