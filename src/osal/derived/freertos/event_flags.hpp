#pragma once

#include "osal/ability/event_flags.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace ifce::os {

class EventFlags : public EventFlagsAbility<EventFlags>
{
  friend class EventFlagsAbility<EventFlags>;
  friend class ifce::DispatchBase<EventFlags>;

public:
  EventFlags()  = default;
  ~EventFlags() { DeleteImpl(); }

private:
  OsStatus CreateImpl()
  {
    if (handle_) return OsStatus::Busy;
    handle_ = xEventGroupCreate();
    return handle_ ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus DeleteImpl()
  {
    if (!handle_) return OsStatus::Ok;
    vEventGroupDelete(handle_);
    handle_ = nullptr;
    return OsStatus::Ok;
  }

  uint32_t SetImpl(uint32_t flags)
  {
    if (!handle_) return 0;
    return static_cast<uint32_t>(xEventGroupSetBits(handle_, static_cast<EventBits_t>(flags)));
  }

  uint32_t ClearImpl(uint32_t flags)
  {
    if (!handle_) return 0;
    return static_cast<uint32_t>(xEventGroupClearBits(handle_, static_cast<EventBits_t>(flags)));
  }

  uint32_t WaitImpl(uint32_t flags, bool wait_all, bool auto_clear, uint32_t timeout_ms)
  {
    if (!handle_) return 0;
    TickType_t ticks = (timeout_ms == WaitForever) ? portMAX_DELAY
                       : pdMS_TO_TICKS(timeout_ms);
    return static_cast<uint32_t>(xEventGroupWaitBits(
      handle_,
      static_cast<EventBits_t>(flags),
      auto_clear  ? pdTRUE : pdFALSE,
      wait_all    ? pdTRUE : pdFALSE,
      ticks));
  }

  uint32_t GetImpl() const
  {
    if (!handle_) return 0;
    return static_cast<uint32_t>(xEventGroupGetBits(handle_));
  }

public:
  // FreeRTOS-specific ISR helpers
  uint32_t SetFromISR(uint32_t flags, BaseType_t* pxHigherPriorityTaskWoken = nullptr)
  {
    if (!handle_) return 0;
    BaseType_t dummy = pdFALSE;
    BaseType_t* p = pxHigherPriorityTaskWoken ? pxHigherPriorityTaskWoken : &dummy;
    BaseType_t result = xEventGroupSetBitsFromISR(handle_, static_cast<EventBits_t>(flags), p);
    return (result == pdPASS) ? flags : 0;
  }

  uint32_t GetFromISR() const
  {
    if (!handle_) return 0;
    return static_cast<uint32_t>(xEventGroupGetBitsFromISR(handle_));
  }

  EventGroupHandle_t GetHandle() const { return handle_; }

private:
  EventGroupHandle_t handle_ = nullptr;
};

} // namespace ifce::os
