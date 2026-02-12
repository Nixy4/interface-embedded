#pragma once
#include "mutex-base.hpp"
#include <mutex>
#include <chrono>
#include <shared_mutex>

namespace osal {

class MutexStdThread : public MutexBase<MutexStdThread>
{
public:
  MutexStdThread() = default;
  ~MutexStdThread() = default;

  MutexStdThread(const MutexStdThread&) = delete;
  MutexStdThread& operator=(const MutexStdThread&) = delete;

  MutexStdThread(MutexStdThread&&) noexcept = default;
  MutexStdThread& operator=(MutexStdThread&&) noexcept = default;

  void InitializeImpl() {}

  void LockImpl()
  {
    if (recursive_) {
      rtm_.lock();
    } else {
      tm_.lock();
    }
  }

  void UnlockImpl()
  {
    if (recursive_) {
      rtm_.unlock();
    } else {
      tm_.unlock();
    }
  }

  bool TryLockImpl()
  {
    if (recursive_) {
      return rtm_.try_lock();
    } else {
      return tm_.try_lock();
    }
  }

  bool TryLockForImpl(unsigned ms)
  {
    if (recursive_) {
      return rtm_.try_lock_for(std::chrono::milliseconds(ms));
    } else {
      return tm_.try_lock_for(std::chrono::milliseconds(ms));
    }
  }

  void SetRecursiveImpl(bool enable)
  {
    recursive_ = enable;
  }

private:
  std::timed_mutex tm_;
  std::recursive_timed_mutex rtm_;
  bool recursive_{false};
};

} // namespace osal
