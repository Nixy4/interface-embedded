#pragma once
#include "task-base.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <functional>
#include <string>

class TaskFreeRTOS : public Task<TaskFreeRTOS>
{
public:
  TaskFreeRTOS() = default;
  ~TaskFreeRTOS() { StopImpl(); }

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

  bool SetCoreImpl(int core)
  {
#ifdef CONFIG_FREERTOS_UNICORE
    return false;
#else
    core_ = core;
    return true;
#endif
  }

  void RunImpl()
  {
    if (!entry_) return;
    BaseType_t rc;
#ifdef CONFIG_FREERTOS_UNICORE
    rc = xTaskCreate(&TaskEntry, name_.empty() ? "task" : name_.c_str(),
                     stack_size_ > 0 ? stack_size_ / sizeof(StackType_t) : 4096,
                     this, MapPriority(priority_, tskIDLE_PRIORITY, configMAX_PRIORITIES-1), &handle_);
#else
    rc = xTaskCreatePinnedToCore(&TaskEntry, name_.empty() ? "task" : name_.c_str(),
                     stack_size_ > 0 ? stack_size_ / sizeof(StackType_t) : 4096,
                     this, MapPriority(priority_, tskIDLE_PRIORITY, configMAX_PRIORITIES-1), &handle_, core_);
#endif
    (void)rc;
  }

  void StopImpl()
  {
    if (handle_) {
      vTaskDelete(handle_);
      handle_ = nullptr;
    }
  }

  bool IsRunningImpl() { return handle_ != nullptr; }

  void JoinImpl() {}
  void DetachImpl() {}

  void SleepMsImpl(unsigned ms)
  {
    vTaskDelay(pdMS_TO_TICKS(ms));
  }

private:
  static void TaskEntry(void* pv)
  {
    auto* self = static_cast<TaskFreeRTOS*>(pv);
    if (self && self->entry_) self->entry_();
    vTaskDelete(nullptr);
  }

  TaskHandle_t handle_{nullptr};
  std::function<void()> entry_{};
  std::string name_;
  int core_{0};
  size_t stack_size_{0};
};
