#pragma once
#include "task-base.hpp"
#include <pthread.h>
#include <functional>
#include <string>
#include <atomic>
#include <unistd.h>

class TaskPthread : public Task<TaskPthread>
{
public:
  TaskPthread() = default;
  ~TaskPthread() { StopImpl(); }

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

  bool SetStackSizeImpl(unsigned bytes)
  {
    stack_size_ = bytes;
    return true;
  }
  bool SetCoreImpl(int) { return false; }

  void RunImpl()
  {
    running_.store(true);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (stack_size_ > 0) pthread_attr_setstacksize(&attr, stack_size_);
    pthread_create(&th_, &attr, &ThreadEntry, this);
    pthread_attr_destroy(&attr);
  }

  void StopImpl()
  {
    if (running_.load()) {
      RequestStop();
      if (th_) { pthread_join(th_, nullptr); th_ = 0; }
      running_.store(false);
    }
  }

  bool IsRunningImpl() { return running_.load(); }

  void JoinImpl()
  {
    if (th_) { pthread_join(th_, nullptr); th_ = 0; }
  }

  void DetachImpl()
  {
    if (th_) { pthread_detach(th_); th_ = 0; }
  }

  void SleepMsImpl(unsigned ms)
  {
    usleep(ms * 1000);
  }

private:
  static void* ThreadEntry(void* pv)
  {
    auto* self = static_cast<TaskPthread*>(pv);
    if (self && self->entry_) self->entry_();
    return nullptr;
  }

  pthread_t th_{0};
  std::function<void()> entry_{};
  std::string name_;
  std::atomic<bool> running_{false};
  size_t stack_size_{0};
};
