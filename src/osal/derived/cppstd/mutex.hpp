#pragma once

#include "osal/ability/mutex.hpp"
#include <mutex>
#include <chrono>

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
    recursive_   = recursive;
    initialized_ = true;
    return OsStatus::Ok;
  }

  OsStatus DeleteImpl()
  {
    initialized_ = false;
    return OsStatus::Ok;
  }

  OsStatus LockImpl(uint32_t timeout_ms)
  {
    if (!initialized_) return OsStatus::Error;
    if (timeout_ms == WaitForever) {
      if (recursive_)
        recursive_mutex_.lock();
      else
        mutex_.lock();
      return OsStatus::Ok;
    }
    auto duration = std::chrono::milliseconds(timeout_ms);
    bool ok;
    if (recursive_)
      ok = recursive_mutex_.try_lock_for(duration);
    else
      ok = timed_mutex_.try_lock_for(duration);
    return ok ? OsStatus::Ok : OsStatus::Timeout;
  }

  OsStatus UnlockImpl()
  {
    if (!initialized_) return OsStatus::Error;
    if (recursive_)
      recursive_mutex_.unlock();
    else {
      // Unlock whichever was locked
      mutex_.unlock();
    }
    return OsStatus::Ok;
  }

  OsStatus TryLockImpl()
  {
    if (!initialized_) return OsStatus::Error;
    bool ok;
    if (recursive_)
      ok = recursive_mutex_.try_lock();
    else
      ok = mutex_.try_lock();
    return ok ? OsStatus::Ok : OsStatus::Busy;
  }

private:
  std::mutex                     mutex_;
  std::recursive_timed_mutex     recursive_mutex_;
  std::timed_mutex               timed_mutex_;
  bool                           recursive_   = false;
  bool                           initialized_ = false;
};

} // namespace osal
