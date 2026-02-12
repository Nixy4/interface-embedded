#pragma once

#include "osal/ability/timer.hpp"
#include "cmsis_os2.h"

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
    if (id_) return OsStatus::Busy;
    callback_  = std::move(callback);
    user_arg_  = arg;
    period_ms_ = period_ms;

    osTimerAttr_t attr = {};
    attr.name = name ? name : "timer";

    osTimerType_t type = auto_reload ? osTimerPeriodic : osTimerOnce;
    id_ = osTimerNew(&TimerCallback, type, this, &attr);
    return id_ ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus DeleteImpl()
  {
    if (!id_) return OsStatus::Ok;
    osStatus_t rc = osTimerDelete(id_);
    id_ = nullptr;
    return (rc == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus StartImpl()
  {
    if (!id_) return OsStatus::Error;
    return (osTimerStart(id_, period_ms_) == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus StopImpl()
  {
    if (!id_) return OsStatus::Error;
    return (osTimerStop(id_) == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus SetPeriodImpl(uint32_t period_ms)
  {
    period_ms_ = period_ms;
    if (id_ && osTimerIsRunning(id_)) {
      osTimerStop(id_);
      return (osTimerStart(id_, period_ms_) == osOK) ? OsStatus::Ok : OsStatus::Error;
    }
    return OsStatus::Ok;
  }

  bool IsRunningImpl() const
  {
    if (!id_) return false;
    return osTimerIsRunning(id_);
  }

public:
  osTimerId_t GetHandle() const { return id_; }

private:
  static void TimerCallback(void* argument)
  {
    auto* self = static_cast<Timer*>(argument);
    if (self && self->callback_)
      self->callback_(self->user_arg_);
  }

  osTimerId_t id_         = nullptr;
  TimerFunc   callback_   = nullptr;
  void*       user_arg_   = nullptr;
  uint32_t    period_ms_  = 0;
};

} // namespace osal
