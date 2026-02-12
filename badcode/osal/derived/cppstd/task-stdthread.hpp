#pragma once
#include "task-base.hpp"
#include <thread>
#include <functional>
#include <string>
#include <atomic>
#include <chrono>

namespace osal {

class TaskStdThread : public Task<TaskStdThread>
{
public:
  TaskStdThread() = default;
  ~TaskStdThread() { StopImpl(); }

  void InitializeImpl() {}

  template <typename F>
  void SetEntryImpl(F&& fn)
  {
    entry_ = std::forward<F>(fn);
  }

  template <typename S>
  void SetNameImpl(S&& name)
  {
    name_ = std::forward<S>(name);
  }

  void SetPriorityImpl(int p)
  {
    priority_ = p;
  }

  int GetPriorityImpl()
  {
    return priority_;
  }

  bool SetStackSizeImpl(unsigned) { return false; }
  bool SetCoreImpl(int) { return false; }

  void RunImpl()
  {
    running_.store(true);
    th_ = std::thread([this]{
      if (entry_) entry_();
      running_.store(false);
    });
  }

  void StopImpl()
  {
    if (running_.load()) {
      RequestStop();
      if (th_.joinable()) th_.join();
      running_.store(false);
    }
  }

  bool IsRunningImpl() { return running_.load(); }

  void JoinImpl()
  {
    if (th_.joinable()) th_.join();
  }

  void DetachImpl()
  {
    if (th_.joinable()) th_.detach();
  }

  void SleepMsImpl(unsigned ms)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }

private:
  std::thread th_;
  std::function<void()> entry_{};
  std::string name_;
  std::atomic<bool> running_{false};
};

} // namespace osal
