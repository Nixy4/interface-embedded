#pragma once
#include "mutex-base.hpp"
#include <pthread.h>
#include <time.h>

class MutexPthread : public MutexBase<MutexPthread>
{
public:
  MutexPthread() = default;
  ~MutexPthread()
  {
    if (initialized_) pthread_mutex_destroy(&m_);
  }

  MutexPthread(const MutexPthread&) = delete;
  MutexPthread& operator=(const MutexPthread&) = delete;

  MutexPthread(MutexPthread&& other) noexcept
    : m_(other.m_), initialized_(other.initialized_)
  {
    other.initialized_ = false;
  }
  MutexPthread& operator=(MutexPthread&& other) noexcept
  {
    if (this != &other) {
      if (initialized_) pthread_mutex_destroy(&m_);
      m_ = other.m_;
      initialized_ = other.initialized_;
      other.initialized_ = false;
    }
    return *this;
  }

  void InitializeImpl()
  {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
#ifdef PTHREAD_PRIO_INHERIT
    pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
#endif
    pthread_mutexattr_settype(&attr, recursive_ ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_DEFAULT);
    pthread_mutex_init(&m_, &attr);
    pthread_mutexattr_destroy(&attr);
    initialized_ = true;
  }

  void LockImpl()
  {
    pthread_mutex_lock(&m_);
  }

  void UnlockImpl()
  {
    pthread_mutex_unlock(&m_);
  }

  bool TryLockImpl()
  {
    return pthread_mutex_trylock(&m_) == 0;
  }

  bool TryLockForImpl(unsigned ms)
  {
#if defined(_POSIX_TIMEOUTS) && (_POSIX_TIMEOUTS >= 200112L)
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += ms / 1000;
    ts.tv_nsec += (ms % 1000) * 1000000L;
    if (ts.tv_nsec >= 1000000000L) {
      ts.tv_sec += ts.tv_nsec / 1000000000L;
      ts.tv_nsec %= 1000000000L;
    }
    return pthread_mutex_timedlock(&m_, &ts) == 0;
#else
    struct timespec req{0, static_cast<long>(ms) * 1000000L};
    nanosleep(&req, nullptr);
    return TryLockImpl();
#endif
  }

  void SetRecursiveImpl(bool enable)
  {
    if (recursive_ == enable) return;
    recursive_ = enable;
    if (initialized_) {
      pthread_mutex_destroy(&m_);
      InitializeImpl();
    }
  }

private:
  pthread_mutex_t m_{};
  bool initialized_{false};
  bool recursive_{false};
};
