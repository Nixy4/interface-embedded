#pragma once

#include "osal/ability/timer.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

namespace ifce::os {

class Timer : public TimerAbility<Timer>
{
  friend class TimerAbility<Timer>;
  friend class ifce::DispatchBase<Timer>;

public:
  Timer()  = default;
  ~Timer() { DeleteImpl(); }

private:
  OsStatus CreateImpl(const char* name, TimerFunc callback, void* arg,
                       uint32_t period_ms, bool auto_reload)
  {
    if (created_) return OsStatus::Busy;
    callback_    = std::move(callback);
    user_arg_    = arg;
    period_ms_   = period_ms;
    auto_reload_ = auto_reload;
    (void)name;
    created_ = true;
    return OsStatus::Ok;
  }

  OsStatus DeleteImpl()
  {
    if (!created_) return OsStatus::Ok;
    StopImpl();
    created_ = false;
    return OsStatus::Ok;
  }

  OsStatus StartImpl()
  {
    if (!created_ || running_.load()) return OsStatus::Error;
    running_.store(true);
    thread_ = std::thread([this] { TimerLoop(); });
    return OsStatus::Ok;
  }

  OsStatus StopImpl()
  {
    if (!running_.load()) return OsStatus::Ok;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      running_.store(false);
    }
    cv_.notify_all();
    if (thread_.joinable())
      thread_.join();
    return OsStatus::Ok;
  }

  OsStatus SetPeriodImpl(uint32_t period_ms)
  {
    period_ms_ = period_ms;
    return OsStatus::Ok;
  }

  bool IsRunningImpl() const { return running_.load(); }

  void TimerLoop()
  {
    do {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait_for(lock, std::chrono::milliseconds(period_ms_),
                   [this] { return !running_.load(); });

      if (!running_.load()) break;

      if (callback_)
        callback_(user_arg_);

    } while (auto_reload_ && running_.load());

    running_.store(false);
  }

private:
  std::thread             thread_;
  std::mutex              mutex_;
  std::condition_variable cv_;
  TimerFunc               callback_    = nullptr;
  void*                   user_arg_    = nullptr;
  uint32_t                period_ms_   = 0;
  bool                    auto_reload_ = false;
  bool                    created_     = false;
  std::atomic<bool>       running_     {false};
};

} // namespace ifce::os
