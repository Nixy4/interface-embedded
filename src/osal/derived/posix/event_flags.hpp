#pragma once

#include "osal/ability/event_flags.hpp"
#include <pthread.h>
#include <ctime>
#include <cerrno>
#include <atomic>

namespace ifce::os {

class EventFlags : public EventFlagsAbility<EventFlags>
{
  friend class EventFlagsAbility<EventFlags>;
  friend class ifce::DispatchBase<EventFlags>;

public:
  EventFlags()  = default;
  ~EventFlags() { DeleteImpl(); }

private:
  OsStatus CreateImpl()
  {
    if (initialized_) return OsStatus::Busy;
    pthread_mutex_init(&mutex_, nullptr);
    pthread_cond_init(&cond_, nullptr);
    flags_.store(0);
    initialized_ = true;
    return OsStatus::Ok;
  }

  OsStatus DeleteImpl()
  {
    if (!initialized_) return OsStatus::Ok;
    pthread_cond_destroy(&cond_);
    pthread_mutex_destroy(&mutex_);
    initialized_ = false;
    return OsStatus::Ok;
  }

  uint32_t SetImpl(uint32_t flags)
  {
    if (!initialized_) return 0;
    pthread_mutex_lock(&mutex_);
    flags_.fetch_or(flags);
    uint32_t current = flags_.load();
    pthread_cond_broadcast(&cond_);
    pthread_mutex_unlock(&mutex_);
    return current;
  }

  uint32_t ClearImpl(uint32_t flags)
  {
    if (!initialized_) return 0;
    uint32_t prev = flags_.fetch_and(~flags);
    return prev;
  }

  uint32_t WaitImpl(uint32_t flags, bool wait_all, bool auto_clear, uint32_t timeout_ms)
  {
    if (!initialized_) return 0;
    pthread_mutex_lock(&mutex_);

    auto condition_met = [&]() -> bool {
      uint32_t current = flags_.load();
      if (wait_all)
        return (current & flags) == flags;
      else
        return (current & flags) != 0;
    };

    if (timeout_ms == WaitForever) {
      while (!condition_met())
        pthread_cond_wait(&cond_, &mutex_);
    } else if (timeout_ms > 0) {
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      ts.tv_sec  += timeout_ms / 1000;
      ts.tv_nsec += (timeout_ms % 1000) * 1000000L;
      if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }

      while (!condition_met()) {
        if (pthread_cond_timedwait(&cond_, &mutex_, &ts) == ETIMEDOUT) {
          pthread_mutex_unlock(&mutex_);
          return 0;
        }
      }
    } else {
      if (!condition_met()) {
        pthread_mutex_unlock(&mutex_);
        return 0;
      }
    }

    uint32_t result = flags_.load() & flags;
    if (auto_clear)
      flags_.fetch_and(~flags);

    pthread_mutex_unlock(&mutex_);
    return result;
  }

  uint32_t GetImpl() const
  {
    return flags_.load();
  }

private:
  pthread_mutex_t       mutex_       = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t        cond_        = PTHREAD_COND_INITIALIZER;
  std::atomic<uint32_t> flags_       {0};
  bool                  initialized_ = false;
};

} // namespace ifce::os
