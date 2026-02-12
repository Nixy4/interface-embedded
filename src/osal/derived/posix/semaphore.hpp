#pragma once

#include "osal/ability/semaphore.hpp"
#include <semaphore.h>
#include <ctime>
#include <cerrno>

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
    if (initialized_) return OsStatus::Busy;
    max_count_ = max_count;
    if (sem_init(&sem_, 0, initial_count) != 0)
      return OsStatus::Error;
    initialized_ = true;
    return OsStatus::Ok;
  }

  OsStatus DeleteImpl()
  {
    if (!initialized_) return OsStatus::Ok;
    sem_destroy(&sem_);
    initialized_ = false;
    return OsStatus::Ok;
  }

  OsStatus AcquireImpl(uint32_t timeout_ms)
  {
    if (!initialized_) return OsStatus::Error;
    if (timeout_ms == WaitForever) {
      while (sem_wait(&sem_) != 0) {
        if (errno != EINTR) return OsStatus::Error;
      }
      return OsStatus::Ok;
    }
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec  += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000L;
    if (ts.tv_nsec >= 1000000000L) {
      ts.tv_sec  += 1;
      ts.tv_nsec -= 1000000000L;
    }
    int rc = sem_timedwait(&sem_, &ts);
    if (rc == 0)                       return OsStatus::Ok;
    if (errno == ETIMEDOUT)            return OsStatus::Timeout;
    return OsStatus::Error;
  }

  OsStatus ReleaseImpl()
  {
    if (!initialized_) return OsStatus::Error;
    return (sem_post(&sem_) == 0) ? OsStatus::Ok : OsStatus::Error;
  }

  uint32_t GetCountImpl() const
  {
    if (!initialized_) return 0;
    int val = 0;
    sem_getvalue(const_cast<sem_t*>(&sem_), &val);
    return (val >= 0) ? static_cast<uint32_t>(val) : 0;
  }

private:
  sem_t    sem_          = {};
  uint32_t max_count_    = 0;
  bool     initialized_  = false;
};

} // namespace osal
