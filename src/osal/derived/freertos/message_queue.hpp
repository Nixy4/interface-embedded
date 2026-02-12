#pragma once

#include "osal/ability/message_queue.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace ifce::os {

template <typename T>
class MessageQueue : public MessageQueueAbility<MessageQueue<T>, T>
{
  friend class MessageQueueAbility<MessageQueue<T>, T>;
  friend class ifce::DispatchBase<MessageQueue<T>>;

public:
  MessageQueue()  = default;
  ~MessageQueue() { DeleteImpl(); }

private:
  OsStatus CreateImpl(uint32_t capacity)
  {
    if (handle_) return OsStatus::Busy;
    capacity_ = capacity;
    handle_ = xQueueCreate(capacity, sizeof(T));
    return handle_ ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus DeleteImpl()
  {
    if (!handle_) return OsStatus::Ok;
    vQueueDelete(handle_);
    handle_ = nullptr;
    return OsStatus::Ok;
  }

  OsStatus PutImpl(const T& msg, uint32_t timeout_ms)
  {
    if (!handle_) return OsStatus::Error;
    TickType_t ticks = (timeout_ms == WaitForever) ? portMAX_DELAY
                       : pdMS_TO_TICKS(timeout_ms);
    return (xQueueSend(handle_, &msg, ticks) == pdTRUE) ? OsStatus::Ok : OsStatus::Timeout;
  }

  OsStatus GetImpl(T& msg, uint32_t timeout_ms)
  {
    if (!handle_) return OsStatus::Error;
    TickType_t ticks = (timeout_ms == WaitForever) ? portMAX_DELAY
                       : pdMS_TO_TICKS(timeout_ms);
    return (xQueueReceive(handle_, &msg, ticks) == pdTRUE) ? OsStatus::Ok : OsStatus::Timeout;
  }

  OsStatus PutToFrontImpl(const T& msg, uint32_t timeout_ms)
  {
    if (!handle_) return OsStatus::Error;
    TickType_t ticks = (timeout_ms == WaitForever) ? portMAX_DELAY
                       : pdMS_TO_TICKS(timeout_ms);
    return (xQueueSendToFront(handle_, &msg, ticks) == pdTRUE) ? OsStatus::Ok : OsStatus::Timeout;
  }

  uint32_t GetCountImpl() const
  {
    if (!handle_) return 0;
    return static_cast<uint32_t>(uxQueueMessagesWaiting(handle_));
  }

  uint32_t GetCapacityImpl() const { return capacity_; }

  OsStatus ResetImpl()
  {
    if (!handle_) return OsStatus::Error;
    xQueueReset(handle_);
    return OsStatus::Ok;
  }

public:
  // FreeRTOS-specific ISR helpers
  OsStatus PutFromISR(const T& msg, BaseType_t* pxHigherPriorityTaskWoken = nullptr)
  {
    if (!handle_) return OsStatus::Error;
    BaseType_t dummy = pdFALSE;
    BaseType_t* p = pxHigherPriorityTaskWoken ? pxHigherPriorityTaskWoken : &dummy;
    return (xQueueSendFromISR(handle_, &msg, p) == pdTRUE) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus GetFromISR(T& msg, BaseType_t* pxHigherPriorityTaskWoken = nullptr)
  {
    if (!handle_) return OsStatus::Error;
    BaseType_t dummy = pdFALSE;
    BaseType_t* p = pxHigherPriorityTaskWoken ? pxHigherPriorityTaskWoken : &dummy;
    return (xQueueReceiveFromISR(handle_, &msg, p) == pdTRUE) ? OsStatus::Ok : OsStatus::Error;
  }

  QueueHandle_t GetHandle() const { return handle_; }

private:
  QueueHandle_t handle_   = nullptr;
  uint32_t      capacity_ = 0;
};

} // namespace ifce::os
