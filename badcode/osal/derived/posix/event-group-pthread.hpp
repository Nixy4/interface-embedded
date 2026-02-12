#pragma once
#include "event-group-base.hpp"
#include <pthread.h>
#include <time.h>
#include <cstdint>

namespace osal {

class EventGroupPthread : public EventGroupBase<EventGroupPthread>
{
public:
  EventGroupPthread()
  {
    pthread_mutex_init(&mtx_, nullptr);
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    pthread_cond_init(&cv_, &cattr);
    pthread_condattr_destroy(&cattr);
  }
  ~EventGroupPthread()
  {
    pthread_mutex_destroy(&mtx_);
    pthread_cond_destroy(&cv_);
  }

  void InitializeImpl() {}

  void SetBitsImpl(uint32_t bits)
  {
    pthread_mutex_lock(&mtx_);
    flags_ |= bits;
    pthread_cond_broadcast(&cv_);
    pthread_mutex_unlock(&mtx_);
  }

  uint32_t WaitBitsForImpl(uint32_t bits_to_wait, bool wait_all, bool clear_on_exit, unsigned ms)
  {
    pthread_mutex_lock(&mtx_);
    auto pred = [&]{
      if (wait_all) return (flags_ & bits_to_wait) == bits_to_wait;
      return (flags_ & bits_to_wait) != 0;
    };
    if (ms == 0) {
      if (!pred()) { auto r=flags_; pthread_mutex_unlock(&mtx_); return r; }
    } else {
      struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);
      ts.tv_sec += ms / 1000;
      ts.tv_nsec += (ms % 1000) * 1000000L;
      if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec += ts.tv_nsec / 1000000000L;
        ts.tv_nsec %= 1000000000L;
      }
      while (!pred()) {
        int rc = pthread_cond_timedwait(&cv_, &mtx_, &ts);
        if (rc != 0) break;
      }
    }
    uint32_t r = flags_;
    if (clear_on_exit) flags_ &= ~bits_to_wait;
    pthread_mutex_unlock(&mtx_);
    return r;
  }

private:
  pthread_mutex_t mtx_{};
  pthread_cond_t cv_{};
  uint32_t flags_{0};
};

} // namespace osal