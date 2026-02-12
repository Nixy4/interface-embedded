#pragma once

#include "osal/ability/thread.hpp"
#include "cmsis_os2.h"
#include <string>

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
    if (id_) return OsStatus::Busy;

    entry_    = std::move(fn);
    user_arg_ = arg;
    name_     = name ? name : "task";
    priority_ = priority;

    osThreadAttr_t attr = {};
    attr.name       = name_.c_str();
    attr.stack_size = stack_size > 0 ? stack_size : 4096;
    attr.priority   = static_cast<osPriority_t>(
      MapPriority(static_cast<int>(priority), osPriorityIdle, osPriorityRealtime7));

    id_ = osThreadNew(&ThreadEntry, this, &attr);
    return id_ ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus TerminateImpl()
  {
    if (!id_) return OsStatus::Ok;
    osStatus_t rc = osThreadTerminate(id_);
    id_ = nullptr;
    return (rc == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus SuspendImpl()
  {
    if (!id_) return OsStatus::Error;
    return (osThreadSuspend(id_) == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus ResumeImpl()
  {
    if (!id_) return OsStatus::Error;
    return (osThreadResume(id_) == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus SetPriorityImpl(ThreadPriority priority)
  {
    if (!id_) return OsStatus::Error;
    priority_ = priority;
    osPriority_t p = static_cast<osPriority_t>(
      MapPriority(static_cast<int>(priority), osPriorityIdle, osPriorityRealtime7));
    return (osThreadSetPriority(id_, p) == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  ThreadPriority GetPriorityImpl() const { return priority_; }
  const char* GetNameImpl() const { return name_.c_str(); }
  bool IsRunningImpl() const { return id_ != nullptr; }

public:
  osThreadId_t GetHandle() const { return id_; }

private:
  static void ThreadEntry(void* argument)
  {
    auto* self = static_cast<Thread*>(argument);
    if (self && self->entry_)
      self->entry_(self->user_arg_);
    self->id_ = nullptr;
  }

  osThreadId_t   id_       = nullptr;
  ThreadFunc     entry_    = nullptr;
  void*          user_arg_ = nullptr;
  std::string    name_;
  ThreadPriority priority_ = ThreadPriority::Normal;
};

} // namespace ifce::os
