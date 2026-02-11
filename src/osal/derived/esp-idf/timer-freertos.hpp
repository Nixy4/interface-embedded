#pragma once
#include "timer-base.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <functional>
#include <cstdint>

class TimerFreeRTOS : public TimerBase<TimerFreeRTOS>
{
public:
  TimerFreeRTOS() = default;
  ~TimerFreeRTOS()
  {
    if (handle_)
    {
      xTimerDelete(handle_, 0);
      handle_ = nullptr;
    }
  }

  TimerFreeRTOS(const TimerFreeRTOS&) = delete;
  TimerFreeRTOS& operator=(const TimerFreeRTOS&) = delete;

  TimerFreeRTOS(TimerFreeRTOS&& other) noexcept
    : handle_(other.handle_),
      period_ms_(other.period_ms_),
      auto_reload_(other.auto_reload_),
      cb_(std::move(other.cb_)),
      c_cb_(other.c_cb_),
      c_user_(other.c_user_)
  {
    other.handle_ = nullptr;
    other.c_cb_ = nullptr;
    other.c_user_ = nullptr;
  }

  TimerFreeRTOS& operator=(TimerFreeRTOS&& other) noexcept
  {
    if (this != &other)
    {
      if (handle_)
      {
        xTimerDelete(handle_, 0);
      }
      handle_ = other.handle_;
      period_ms_ = other.period_ms_;
      auto_reload_ = other.auto_reload_;
      cb_ = std::move(other.cb_);
      c_cb_ = other.c_cb_;
      c_user_ = other.c_user_;

      other.handle_ = nullptr;
      other.c_cb_ = nullptr;
      other.c_user_ = nullptr;
    }
    return *this;
  }

  void InitializeImpl(unsigned period_ms, bool auto_reload)
  {
    period_ms_ = period_ms;
    auto_reload_ = auto_reload;
    if (handle_)
    {
      xTimerDelete(handle_, 0);
      handle_ = nullptr;
    }
    handle_ = xTimerCreate(
      "osal_timer",
      pdMS_TO_TICKS(period_ms_),
      auto_reload_ ? pdTRUE : pdFALSE,
      this,
      &TimerCallback
    );
  }

  void StartImpl()
  {
    if (handle_) xTimerStart(handle_, 0);
  }

  void StopImpl()
  {
    if (handle_) xTimerStop(handle_, 0);
  }

  void ResetImpl()
  {
    if (handle_) xTimerReset(handle_, 0);
  }

  void SetPeriodImpl(unsigned ms)
  {
    period_ms_ = ms;
    if (handle_)
    {
      xTimerChangePeriod(handle_, pdMS_TO_TICKS(period_ms_), 0);
    }
  }

  unsigned GetPeriodImpl()
  {
    return period_ms_;
  }

  bool IsRunningImpl()
  {
    if (!handle_) return false;
    return xTimerIsTimerActive(handle_) == pdTRUE;
  }

  void TickImpl()
  {
    if (cb_) cb_();
    else if (c_cb_) c_cb_(c_user_);
  }

  void StartOnceImpl()
  {
    auto_reload_ = false;
    if (handle_)
    {
      xTimerStop(handle_, 0);
      vTimerSetReloadMode(handle_, pdFALSE);
      xTimerStart(handle_, 0);
    }
  }

  void SetCallbackImpl(std::function<void()> cb)
  {
    cb_ = std::move(cb);
    c_cb_ = nullptr;
    c_user_ = nullptr;
  }

  void SetCallbackImpl(void (*cb)(void*), void* user)
  {
    c_cb_ = cb;
    c_user_ = user;
    cb_ = {};
  }

private:
  static void TimerCallback(TimerHandle_t xTimer)
  {
    void* pv = pvTimerGetTimerID(xTimer);
    auto* self = static_cast<TimerFreeRTOS*>(pv);
    if (!self) return;
    if (self->cb_)
    {
      self->cb_();
    }
    else if (self->c_cb_)
    {
      self->c_cb_(self->c_user_);
    }
  }

  TimerHandle_t handle_{nullptr};
  unsigned period_ms_{0};
  bool auto_reload_{false};
  std::function<void()> cb_{};
  void (*c_cb_)(void*){nullptr};
  void* c_user_{nullptr};
};
