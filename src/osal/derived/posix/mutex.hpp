#pragma once

#include "osal/ability/mutex.hpp"
#include <pthread.h>
#include <ctime>
#include <cerrno>

namespace osal {

class Mutex : public MutexAbility<Mutex>
{
  friend class MutexAbility<Mutex>;
  friend class DispatchBase<Mutex>;

public:
  Mutex()  = default;
  ~Mutex() { DeleteImpl(); }

private:
  OsStatus CreateImpl(bool recursive)
  {
    if (initialized_) return OsStatus::Busy;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if (recursive)
      pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    else
      pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    int rc = pthread_mutex_init(&mutex_, &attr);
    pthread_mutexattr_destroy(&attr);
    if (rc != 0) return OsStatus::Error;
    initialized_ = true;
    return OsStatus::Ok;
  }

  OsStatus DeleteImpl()
  {
    if (!initialized_) return OsStatus::Ok;
    pthread_mutex_destroy(&mutex_);
    initialized_ = false;
    return OsStatus::Ok;
  }

  OsStatus LockImpl(uint32_t timeout_ms)
  {
    if (!initialized_) return OsStatus::Error;
    if (timeout_ms == WaitForever) {
      return (pthread_mutex_lock(&mutex_) == 0) ? OsStatus::Ok : OsStatus::Error;
    }
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec  += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000L;
    if (ts.tv_nsec >= 1000000000L) {
      ts.tv_sec  += 1;
      ts.tv_nsec -= 1000000000L;
    }
    int rc = pthread_mutex_timedlock(&mutex_, &ts);
    if (rc == 0)          return OsStatus::Ok;
    if (rc == ETIMEDOUT)  return OsStatus::Timeout;
    return OsStatus::Error;
  }

  OsStatus UnlockImpl()
  {
    if (!initialized_) return OsStatus::Error;
    return (pthread_mutex_unlock(&mutex_) == 0) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus TryLockImpl()
  {
    if (!initialized_) return OsStatus::Error;
    int rc = pthread_mutex_trylock(&mutex_);
    if (rc == 0)      return OsStatus::Ok;
    if (rc == EBUSY)  return OsStatus::Busy;
    return OsStatus::Error;
  }

private:
  pthread_mutex_t mutex_       = PTHREAD_MUTEX_INITIALIZER;
  bool            initialized_ = false;
};

} // namespace osal
