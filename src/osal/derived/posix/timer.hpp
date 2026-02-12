#pragma once

#include "osal/ability/timer.hpp"
#include <pthread.h>
#include <ctime>
#include <atomic>

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
    if (created_) return OsStatus::Busy;
    callback_    = std::move(callback);
    user_arg_    = arg;
    period_ms_   = period_ms;
    auto_reload_ = auto_reload;
    (void)name;
    created_ = true;
    return OsStatus::Ok;
  }

  OsStatus DeleteImpl()
  {
    if (!created_) return OsStatus::Ok;
    StopImpl();
    created_ = false;
    return OsStatus::Ok;
  }

  OsStatus StartImpl()
  {
    if (!created_ || running_.load()) return OsStatus::Error;
    running_.store(true);

    pthread_mutex_init(&timer_mutex_, nullptr);
    pthread_cond_init(&timer_cond_, nullptr);

    int rc = pthread_create(&thread_, nullptr, &TimerThread, this);
    if (rc != 0) {
      running_.store(false);
      return OsStatus::NoMemory;
    }
    return OsStatus::Ok;
  }

  OsStatus StopImpl()
  {
    if (!running_.load()) return OsStatus::Ok;
    running_.store(false);

    pthread_mutex_lock(&timer_mutex_);
    pthread_cond_signal(&timer_cond_);
    pthread_mutex_unlock(&timer_mutex_);

    pthread_join(thread_, nullptr);
    pthread_cond_destroy(&timer_cond_);
    pthread_mutex_destroy(&timer_mutex_);
    return OsStatus::Ok;
  }

  OsStatus SetPeriodImpl(uint32_t period_ms)
  {
    period_ms_ = period_ms;
    return OsStatus::Ok;
  }

  bool IsRunningImpl() const { return running_.load(); }

private:
  static void* TimerThread(void* pv)
  {
    auto* self = static_cast<Timer*>(pv);
    do {
      // Wait for period
      pthread_mutex_lock(&self->timer_mutex_);
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      uint32_t ms = self->period_ms_;
      ts.tv_sec  += ms / 1000;
      ts.tv_nsec += (ms % 1000) * 1000000L;
      if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
      pthread_cond_timedwait(&self->timer_cond_, &self->timer_mutex_, &ts);
      pthread_mutex_unlock(&self->timer_mutex_);

      if (!self->running_.load()) break;

      if (self->callback_)
        self->callback_(self->user_arg_);

    } while (self->auto_reload_ && self->running_.load());

    self->running_.store(false);
    return nullptr;
  }

  pthread_t         thread_      = {};
  pthread_mutex_t   timer_mutex_ = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t    timer_cond_  = PTHREAD_COND_INITIALIZER;
  TimerFunc         callback_    = nullptr;
  void*             user_arg_    = nullptr;
  uint32_t          period_ms_   = 0;
  bool              auto_reload_ = false;
  bool              created_     = false;
  std::atomic<bool> running_     {false};
};

} // namespace osal
