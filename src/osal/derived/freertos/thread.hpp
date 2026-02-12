#pragma once

#include "osal/ability/thread.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <functional>
#include <string>

namespace osal {

class Thread : public ThreadAbility<Thread>
{
  friend class ThreadAbility<Thread>;
  friend class DispatchBase<Thread>;

public:
  Thread()  = default;
  ~Thread() { TerminateImpl(); }

private:
  // --- Mandatory Impl ---

  OsStatus CreateImpl(const char* name, ThreadFunc fn, void* arg,
                       uint32_t stack_size, ThreadPriority priority)
  {
    if (handle_) return OsStatus::Busy;

    name_       = name ? name : "task";
    entry_      = std::move(fn);
    user_arg_   = arg;
    stack_size_ = stack_size > 0 ? stack_size : 4096;
    priority_   = priority;

    int prio = MapPriority(static_cast<int>(priority_),
                           tskIDLE_PRIORITY, configMAX_PRIORITIES - 1);
    BaseType_t rc;
#if defined(CONFIG_FREERTOS_UNICORE) || (configNUMBER_OF_CORES == 1)
    rc = xTaskCreate(&TaskEntry, name_.c_str(),
                     stack_size_ / sizeof(StackType_t),
                     this, prio, &handle_);
#else
    rc = xTaskCreatePinnedToCore(&TaskEntry, name_.c_str(),
                     stack_size_ / sizeof(StackType_t),
                     this, prio, &handle_, core_);
#endif
    return (rc == pdPASS) ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus TerminateImpl()
  {
    if (!handle_) return OsStatus::Ok;
    vTaskDelete(handle_);
    handle_ = nullptr;
    return OsStatus::Ok;
  }

  // --- Optional Impl ---

  OsStatus SuspendImpl()
  {
    if (!handle_) return OsStatus::Error;
    vTaskSuspend(handle_);
    return OsStatus::Ok;
  }

  OsStatus ResumeImpl()
  {
    if (!handle_) return OsStatus::Error;
    vTaskResume(handle_);
    return OsStatus::Ok;
  }

  OsStatus SetPriorityImpl(ThreadPriority priority)
  {
    if (!handle_) return OsStatus::Error;
    priority_ = priority;
    vTaskPrioritySet(handle_,
      MapPriority(static_cast<int>(priority_), tskIDLE_PRIORITY, configMAX_PRIORITIES - 1));
    return OsStatus::Ok;
  }

  ThreadPriority GetPriorityImpl() const { return priority_; }
  const char* GetNameImpl() const { return name_.c_str(); }
  uint32_t GetStackSizeImpl() const { return stack_size_; }
  bool IsRunningImpl() const { return handle_ != nullptr; }

  OsStatus SetAffinityImpl(int core)
  {
#if defined(CONFIG_FREERTOS_UNICORE) || (configNUMBER_OF_CORES == 1)
    (void)core;
    return OsStatus::Error;
#else
    core_ = core;
    return OsStatus::Ok;
#endif
  }

  // --- ISR helpers (FreeRTOS-specific, not in Ability interface) ---
public:
  OsStatus ResumeFromISR()
  {
    if (!handle_) return OsStatus::Error;
    BaseType_t yield = pdFALSE;
    xTaskResumeFromISR(handle_);
    portYIELD_FROM_ISR(yield);
    return OsStatus::Ok;
  }

  TaskHandle_t GetHandle() const { return handle_; }

private:
  static void TaskEntry(void* pv)
  {
    auto* self = static_cast<Thread*>(pv);
    if (self && self->entry_)
      self->entry_(self->user_arg_);
    self->handle_ = nullptr;
    vTaskDelete(nullptr);
  }

  TaskHandle_t   handle_     = nullptr;
  ThreadFunc     entry_      = nullptr;
  void*          user_arg_   = nullptr;
  std::string    name_;
  uint32_t       stack_size_ = 4096;
  ThreadPriority priority_   = ThreadPriority::Normal;
  int            core_       = 0;
};

} // namespace osal
