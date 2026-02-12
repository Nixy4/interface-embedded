#pragma once

#include "osal/ability/event_flags.hpp"
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

namespace osal {

class EventFlags : public EventFlagsAbility<EventFlags>
{
  friend class EventFlagsAbility<EventFlags>;
  friend class DispatchBase<EventFlags>;

public:
  EventFlags()  = default;
  ~EventFlags() { DeleteImpl(); }

private:
  OsStatus CreateImpl()
  {
    if (initialized_) return OsStatus::Busy;
    flags_.store(0);
    initialized_ = true;
    return OsStatus::Ok;
  }

  OsStatus DeleteImpl()
  {
    initialized_ = false;
    return OsStatus::Ok;
  }

  uint32_t SetImpl(uint32_t flags)
  {
    if (!initialized_) return 0;
    uint32_t result;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      flags_.fetch_or(flags);
      result = flags_.load();
    }
    cv_.notify_all();
    return result;
  }

  uint32_t ClearImpl(uint32_t flags)
  {
    if (!initialized_) return 0;
    return flags_.fetch_and(~flags);
  }

  uint32_t WaitImpl(uint32_t flags, bool wait_all, bool auto_clear, uint32_t timeout_ms)
  {
    if (!initialized_) return 0;
    std::unique_lock<std::mutex> lock(mutex_);

    auto condition = [&]() -> bool {
      uint32_t current = flags_.load();
      return wait_all ? ((current & flags) == flags)
                      : ((current & flags) != 0);
    };

    if (timeout_ms == WaitForever) {
      cv_.wait(lock, condition);
    } else if (timeout_ms > 0) {
      if (!cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), condition))
        return 0;
    } else {
      if (!condition()) return 0;
    }

    uint32_t result = flags_.load() & flags;
    if (auto_clear)
      flags_.fetch_and(~flags);
    return result;
  }

  uint32_t GetImpl() const
  {
    return flags_.load();
  }

private:
  std::mutex              mutex_;
  std::condition_variable cv_;
  std::atomic<uint32_t>   flags_       {0};
  bool                    initialized_ = false;
};

} // namespace osal
