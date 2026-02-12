#pragma once

#include "osal/ability/thread.hpp"
#include <thread>
#include <string>
#include <atomic>

namespace osal {

class Thread : public ThreadAbility<Thread>
{
  friend class ThreadAbility<Thread>;
  friend class DispatchBase<Thread>;

public:
  Thread()  = default;
  ~Thread() { TerminateImpl(); }

private:
  OsStatus CreateImpl(const char* name, ThreadFunc fn, void* arg,
                       uint32_t stack_size, ThreadPriority priority)
  {
    if (running_.load()) return OsStatus::Busy;

    entry_      = std::move(fn);
    user_arg_   = arg;
    name_       = name ? name : "task";
    stack_size_ = stack_size;
    priority_   = priority;
    stop_requested_.store(false);
    running_.store(true);

    thread_ = std::thread([this] {
      if (entry_)
        entry_(user_arg_);
      running_.store(false);
    });
    return OsStatus::Ok;
  }

  OsStatus TerminateImpl()
  {
    if (!running_.load() && !thread_.joinable()) return OsStatus::Ok;
    stop_requested_.store(true);
    if (thread_.joinable())
      thread_.join();
    running_.store(false);
    return OsStatus::Ok;
  }

  OsStatus JoinImpl()
  {
    if (!thread_.joinable()) return OsStatus::Error;
    thread_.join();
    running_.store(false);
    return OsStatus::Ok;
  }

  OsStatus DetachImpl()
  {
    if (!thread_.joinable()) return OsStatus::Error;
    thread_.detach();
    return OsStatus::Ok;
  }

  ThreadPriority GetPriorityImpl() const { return priority_; }
  const char* GetNameImpl() const { return name_.c_str(); }
  uint32_t GetStackSizeImpl() const { return stack_size_; }
  bool IsRunningImpl() const { return running_.load(); }

public:
  /// Check if stop has been requested (for cooperative cancellation)
  bool ShouldStop() const { return stop_requested_.load(); }

private:
  std::thread        thread_;
  ThreadFunc         entry_          = nullptr;
  void*              user_arg_       = nullptr;
  std::string        name_;
  uint32_t           stack_size_     = 0;
  ThreadPriority     priority_       = ThreadPriority::Normal;
  std::atomic<bool>  running_        {false};
  std::atomic<bool>  stop_requested_ {false};
};

} // namespace osal
