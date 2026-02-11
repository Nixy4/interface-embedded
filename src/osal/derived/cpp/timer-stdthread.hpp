#pragma once
#include "timer-base.hpp"
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <functional>

class TimerStdThread : public TimerBase<TimerStdThread>
{
public:
  TimerStdThread() = default;
  ~TimerStdThread()
  {
    StopImpl();
  }

  TimerStdThread(const TimerStdThread&) = delete;
  TimerStdThread& operator=(const TimerStdThread&) = delete;

  TimerStdThread(TimerStdThread&& other) noexcept
  {
    std::lock_guard<std::mutex> lk(other.mtx_);
    period_ms_ = other.period_ms_;
    auto_reload_ = other.auto_reload_;
    running_.store(other.running_.load());
    stop_req_.store(other.stop_req_.load());
    cb_ = std::move(other.cb_);
    c_cb_ = other.c_cb_;
    c_user_ = other.c_user_;
  }
  TimerStdThread& operator=(TimerStdThread&& other) noexcept
  {
    if (this != &other)
    {
      StopImpl();
      std::lock_guard<std::mutex> lk(other.mtx_);
      period_ms_ = other.period_ms_;
      auto_reload_ = other.auto_reload_;
      running_.store(other.running_.load());
      stop_req_.store(other.stop_req_.load());
      cb_ = std::move(other.cb_);
      c_cb_ = other.c_cb_;
      c_user_ = other.c_user_;
    }
    return *this;
  }

  void InitializeImpl(unsigned period_ms, bool auto_reload)
  {
    std::lock_guard<std::mutex> lk(mtx_);
    period_ms_ = period_ms;
    auto_reload_ = auto_reload;
  }

  void StartImpl()
  {
    std::lock_guard<std::mutex> lk(mtx_);
    if (running_.load()) return;
    stop_req_.store(false);
    running_.store(true);
    th_ = std::thread([this]{
      std::unique_lock<std::mutex> lk(mtx_);
      while (running_.load() && !stop_req_.load())
      {
        auto to = std::chrono::milliseconds(period_ms_);
        if (cv_.wait_for(lk, to, [this]{ return stop_req_.load(); })) break;
        lk.unlock();
        TickImpl();
        lk.lock();
        if (!auto_reload_)
        {
          stop_req_.store(true);
          break;
        }
      }
      running_.store(false);
    });
  }

  void StopImpl()
  {
    {
      std::lock_guard<std::mutex> lk(mtx_);
      if (!running_.load()) return;
      stop_req_.store(true);
      cv_.notify_all();
    }
    if (th_.joinable()) th_.join();
    running_.store(false);
  }

  void ResetImpl()
  {
    std::lock_guard<std::mutex> lk(mtx_);
    cv_.notify_all();
  }

  void SetPeriodImpl(unsigned ms)
  {
    std::lock_guard<std::mutex> lk(mtx_);
    period_ms_ = ms;
    cv_.notify_all();
  }

  unsigned GetPeriodImpl()
  {
    std::lock_guard<std::mutex> lk(mtx_);
    return period_ms_;
  }

  bool IsRunningImpl()
  {
    return running_.load();
  }

  void TickImpl()
  {
    if (cb_) cb_();
    else if (c_cb_) c_cb_(c_user_);
  }

  void StartOnceImpl()
  {
    std::lock_guard<std::mutex> lk(mtx_);
    auto_reload_ = false;
    if (!running_.load())
    {
      stop_req_.store(false);
      running_.store(true);
      th_ = std::thread([this]{
        std::unique_lock<std::mutex> lk(mtx_);
        auto to = std::chrono::milliseconds(period_ms_);
        if (!cv_.wait_for(lk, to, [this]{ return stop_req_.load(); }))
        {
          TickImpl();
        }
        stop_req_.store(true);
        running_.store(false);
      });
      th_.detach();
    }
    else
    {
      auto_reload_ = false;
    }
  }

  void SetCallbackImpl(std::function<void()> cb)
  {
    std::lock_guard<std::mutex> lk(mtx_);
    cb_ = std::move(cb);
    c_cb_ = nullptr;
    c_user_ = nullptr;
  }

  void SetCallbackImpl(void (*cb)(void*), void* user)
  {
    std::lock_guard<std::mutex> lk(mtx_);
    c_cb_ = cb;
    c_user_ = user;
    cb_ = {};
  }

private:
  std::thread th_;
  std::mutex mtx_;
  std::condition_variable cv_;
  unsigned period_ms_{0};
  bool auto_reload_{false};
  std::atomic<bool> running_{false};
  std::atomic<bool> stop_req_{false};
  std::function<void()> cb_{};
  void (*c_cb_)(void*){nullptr};
  void* c_user_{nullptr};
};
