#pragma once

#include "osal/types.hpp"

namespace osal {

/// RAII lock guard. Works with any type that has Lock(uint32_t) and Unlock() methods.
template <typename MutexType>
class LockGuard
{
public:
  explicit LockGuard(MutexType& mutex, uint32_t timeout_ms = WaitForever)
    : mutex_(mutex), owned_(false)
  {
    owned_ = (mutex_.Lock(timeout_ms) == OsStatus::Ok);
  }

  ~LockGuard()
  {
    if (owned_)
      mutex_.Unlock();
  }

  /// Returns true if the lock was successfully acquired
  bool Owns() const { return owned_; }

  explicit operator bool() const { return owned_; }

  // Non-copyable, non-movable
  LockGuard(const LockGuard&)            = delete;
  LockGuard& operator=(const LockGuard&) = delete;
  LockGuard(LockGuard&&)                 = delete;
  LockGuard& operator=(LockGuard&&)      = delete;

private:
  MutexType& mutex_;
  bool       owned_;
};

} // namespace osal
