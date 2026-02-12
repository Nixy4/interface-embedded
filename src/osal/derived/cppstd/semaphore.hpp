#pragma once

#include "osal/ability/semaphore.hpp"
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace ifce::os {

/// Counting semaphore implemented with std::mutex + std::condition_variable (C++17 compatible)
class Semaphore : public SemaphoreAbility<Semaphore>
{
  friend class SemaphoreAbility<Semaphore>;
  friend class ifce::DispatchBase<Semaphore>;

public:
  Semaphore()  = default;
  ~Semaphore() { DeleteImpl(); }

private:
  OsStatus CreateImpl(uint32_t max_count, uint32_t initial_count)
  {
    if (initialized_) return OsStatus::Busy;
    max_count_   = max_count;
    count_       = initial_count;
    initialized_ = true;
    return OsStatus::Ok;
  }

  OsStatus DeleteImpl()
  {
    initialized_ = false;
    return OsStatus::Ok;
  }

  OsStatus AcquireImpl(uint32_t timeout_ms)
  {
    if (!initialized_) return OsStatus::Error;
    std::unique_lock<std::mutex> lock(mutex_);

    if (timeout_ms == WaitForever) {
      cv_.wait(lock, [this] { return count_ > 0; });
    } else {
      if (!cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                        [this] { return count_ > 0; }))
        return OsStatus::Timeout;
    }
    --count_;
    return OsStatus::Ok;
  }

  OsStatus ReleaseImpl()
  {
    if (!initialized_) return OsStatus::Error;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (count_ >= max_count_) return OsStatus::Error;
      ++count_;
    }
    cv_.notify_one();
    return OsStatus::Ok;
  }

  uint32_t GetCountImpl() const { return count_; }

private:
  std::mutex              mutex_;
  std::condition_variable cv_;
  uint32_t                max_count_   = 0;
  uint32_t                count_       = 0;
  bool                    initialized_ = false;
};

} // namespace ifce::os
