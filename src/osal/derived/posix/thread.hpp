#pragma once

#include "osal/ability/thread.hpp"
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <string>
#include <atomic>

namespace ifce::os {

class Thread : public ThreadAbility<Thread>
{
  friend class ThreadAbility<Thread>;
  friend class ifce::DispatchBase<Thread>;

public:
  Thread()  = default;
  ~Thread() { TerminateImpl(); }

private:
  OsStatus CreateImpl(const char* name, ThreadFunc fn, void* arg,
                       uint32_t stack_size, ThreadPriority priority)
  {
    if (running_.load()) return OsStatus::Busy;

    entry_    = std::move(fn);
    user_arg_ = arg;
    name_     = name ? name : "task";
    priority_ = priority;
    stack_size_ = stack_size;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (stack_size > 0)
      pthread_attr_setstacksize(&attr, stack_size);

    int rc = pthread_create(&thread_, &attr, &ThreadEntry, this);
    pthread_attr_destroy(&attr);

    if (rc != 0) return OsStatus::NoMemory;

#if !defined(__APPLE__)
    if (!name_.empty())
      pthread_setname_np(thread_, name_.substr(0, 15).c_str());
#endif

    running_.store(true);
    return OsStatus::Ok;
  }

  OsStatus TerminateImpl()
  {
    if (!running_.load()) return OsStatus::Ok;
    pthread_cancel(thread_);
    pthread_join(thread_, nullptr);
    running_.store(false);
    return OsStatus::Ok;
  }

  OsStatus JoinImpl()
  {
    if (!running_.load()) return OsStatus::Error;
    int rc = pthread_join(thread_, nullptr);
    running_.store(false);
    return (rc == 0) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus DetachImpl()
  {
    if (!running_.load()) return OsStatus::Error;
    int rc = pthread_detach(thread_);
    return (rc == 0) ? OsStatus::Ok : OsStatus::Error;
  }

  ThreadPriority GetPriorityImpl() const { return priority_; }
  const char* GetNameImpl() const { return name_.c_str(); }
  uint32_t GetStackSizeImpl() const { return stack_size_; }
  bool IsRunningImpl() const { return running_.load(); }

public:
  pthread_t GetHandle() const { return thread_; }

private:
  static void* ThreadEntry(void* pv)
  {
    auto* self = static_cast<Thread*>(pv);
    if (self && self->entry_)
      self->entry_(self->user_arg_);
    self->running_.store(false);
    return nullptr;
  }

  pthread_t          thread_     = {};
  ThreadFunc         entry_      = nullptr;
  void*              user_arg_   = nullptr;
  std::string        name_;
  uint32_t           stack_size_ = 0;
  ThreadPriority     priority_   = ThreadPriority::Normal;
  std::atomic<bool>  running_    {false};
};

} // namespace ifce::os
