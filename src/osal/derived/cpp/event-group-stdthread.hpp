#pragma once
#include "event-group-base.hpp"
#include <mutex>
#include <condition_variable>
#include <cstdint>

class EventGroupStdThread : public EventGroupBase<EventGroupStdThread>
{
public:
  EventGroupStdThread() = default;
  ~EventGroupStdThread() = default;

  void InitializeImpl() {}

  void SetBitsImpl(uint32_t bits)
  {
    std::lock_guard<std::mutex> lk(mtx_);
    flags_ |= bits;
    cv_.notify_all();
  }

  uint32_t WaitBitsForImpl(uint32_t bits_to_wait, bool wait_all, bool clear_on_exit, unsigned ms)
  {
    std::unique_lock<std::mutex> lk(mtx_);
    auto pred = [&]{
      if (wait_all) return (flags_ & bits_to_wait) == bits_to_wait;
      return (flags_ & bits_to_wait) != 0;
    };
    if (ms == 0) {
      if (!pred()) return flags_;
    } else {
      if (!cv_.wait_for(lk, std::chrono::milliseconds(ms), pred)) return flags_;
    }
    uint32_t r = flags_;
    if (clear_on_exit) flags_ &= ~bits_to_wait;
    return r;
  }

private:
  std::mutex mtx_;
  std::condition_variable cv_;
  uint32_t flags_{0};
};
