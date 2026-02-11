#pragma once
#include "timer-base.hpp"
#include <pthread.h>
#include <time.h>
#include <atomic>
#include <functional>
#include <cstring>
#include <errno.h>

class TimerPthread : public TimerBase<TimerPthread>
{
public:
  TimerPthread()
  {
    pthread_mutex_init(&mtx_, nullptr);
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    pthread_cond_init(&cv_, &cattr);
    pthread_condattr_destroy(&cattr);
  }
  ~TimerPthread()
  {
    StopImpl();
    pthread_mutex_destroy(&mtx_);
    pthread_cond_destroy(&cv_);
  }

  TimerPthread(const TimerPthread&) = delete;
  TimerPthread& operator=(const TimerPthread&) = delete;

  void InitializeImpl(unsigned period_ms, bool auto_reload)
  {
    pthread_mutex_lock(&mtx_);
    period_ms_ = period_ms;
    auto_reload_ = auto_reload;
    pthread_mutex_unlock(&mtx_);
  }

  void StartImpl()
  {
    pthread_mutex_lock(&mtx_);
    if (running_) { pthread_mutex_unlock(&mtx_); return; }
    stop_req_ = false;
    running_ = true;
    pthread_mutex_unlock(&mtx_);
    pthread_create(&th_, nullptr, &ThreadEntry, this);
  }

  void StopImpl()
  {
    pthread_mutex_lock(&mtx_);
    if (!running_) { pthread_mutex_unlock(&mtx_); return; }
    stop_req_ = true;
    pthread_cond_broadcast(&cv_);
    pthread_mutex_unlock(&mtx_);
    if (th_) { pthread_join(th_, nullptr); th_ = 0; }
    running_ = false;
  }

  void ResetImpl()
  {
    pthread_mutex_lock(&mtx_);
    pthread_cond_broadcast(&cv_);
    pthread_mutex_unlock(&mtx_);
  }

  void SetPeriodImpl(unsigned ms)
  {
    pthread_mutex_lock(&mtx_);
    period_ms_ = ms;
    pthread_cond_broadcast(&cv_);
    pthread_mutex_unlock(&mtx_);
  }

  unsigned GetPeriodImpl()
  {
    pthread_mutex_lock(&mtx_);
    unsigned ms = period_ms_;
    pthread_mutex_unlock(&mtx_);
    return ms;
  }

  bool IsRunningImpl()
  {
    return running_;
  }

  void TickImpl()
  {
    if (cb_) cb_();
    else if (c_cb_) c_cb_(c_user_);
  }

  void StartOnceImpl()
  {
    pthread_mutex_lock(&mtx_);
    auto_reload_ = false;
    bool already = running_;
    pthread_mutex_unlock(&mtx_);
    if (!already) {
      StartImpl();
      if (th_) { pthread_detach(th_); th_ = 0; }
    }
  }

  void SetCallbackImpl(std::function<void()> cb)
  {
    pthread_mutex_lock(&mtx_);
    cb_ = std::move(cb);
    c_cb_ = nullptr;
    c_user_ = nullptr;
    pthread_mutex_unlock(&mtx_);
  }

  void SetCallbackImpl(void (*cb)(void*), void* user)
  {
    pthread_mutex_lock(&mtx_);
    c_cb_ = cb;
    c_user_ = user;
    cb_ = {};
    pthread_mutex_unlock(&mtx_);
  }

private:
  static void* ThreadEntry(void* pv)
  {
    auto* self = static_cast<TimerPthread*>(pv);
    pthread_mutex_lock(&self->mtx_);
    while (!self->stop_req_)
    {
      struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);
      ts.tv_sec  += self->period_ms_ / 1000;
      ts.tv_nsec += (self->period_ms_ % 1000) * 1000000L;
      if (ts.tv_nsec >= 1000000000L)
      {
        ts.tv_sec += ts.tv_nsec / 1000000000L;
        ts.tv_nsec %= 1000000000L;
      }
      int rc = 0;
      while (rc == 0)
      {
        rc = pthread_cond_timedwait(&self->cv_, &self->mtx_, &ts);
        if (self->stop_req_) break;
        if (rc == ETIMEDOUT) break;
      }
      if (self->stop_req_) break;
      pthread_mutex_unlock(&self->mtx_);
      self->TickImpl();
      pthread_mutex_lock(&self->mtx_);
      if (!self->auto_reload_) break;
    }
    pthread_mutex_unlock(&self->mtx_);
    self->running_ = false;
    return nullptr;
  }

  pthread_t th_{0};
  pthread_mutex_t mtx_{};
  pthread_cond_t cv_{};
  unsigned period_ms_{0};
  bool auto_reload_{false};
  std::atomic<bool> running_{false};
  std::atomic<bool> stop_req_{false};
  std::function<void()> cb_{};
  void (*c_cb_)(void*){nullptr};
  void* c_user_{nullptr};
};
