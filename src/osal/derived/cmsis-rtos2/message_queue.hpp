#pragma once

#include "osal/ability/message_queue.hpp"
#include "cmsis_os2.h"
#include <cstring>

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
    if (id_) return OsStatus::Busy;
    capacity_ = capacity;
    id_ = osMessageQueueNew(capacity, sizeof(T), nullptr);
    return id_ ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus DeleteImpl()
  {
    if (!id_) return OsStatus::Ok;
    osStatus_t rc = osMessageQueueDelete(id_);
    id_ = nullptr;
    return (rc == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  OsStatus PutImpl(const T& msg, uint32_t timeout_ms)
  {
    if (!id_) return OsStatus::Error;
    uint32_t ticks = (timeout_ms == WaitForever) ? osWaitForever : timeout_ms;
    osStatus_t rc = osMessageQueuePut(id_, &msg, 0, ticks);
    if (rc == osOK)           return OsStatus::Ok;
    if (rc == osErrorTimeout) return OsStatus::Timeout;
    return OsStatus::Error;
  }

  OsStatus GetImpl(T& msg, uint32_t timeout_ms)
  {
    if (!id_) return OsStatus::Error;
    uint32_t ticks = (timeout_ms == WaitForever) ? osWaitForever : timeout_ms;
    osStatus_t rc = osMessageQueueGet(id_, &msg, nullptr, ticks);
    if (rc == osOK)           return OsStatus::Ok;
    if (rc == osErrorTimeout) return OsStatus::Timeout;
    return OsStatus::Error;
  }

  uint32_t GetCountImpl() const
  {
    if (!id_) return 0;
    return osMessageQueueGetCount(id_);
  }

  uint32_t GetCapacityImpl() const
  {
    if (!id_) return 0;
    return osMessageQueueGetCapacity(id_);
  }

  OsStatus ResetImpl()
  {
    if (!id_) return OsStatus::Error;
    return (osMessageQueueReset(id_) == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

public:
  osMessageQueueId_t GetHandle() const { return id_; }

private:
  osMessageQueueId_t id_       = nullptr;
  uint32_t           capacity_ = 0;
};

} // namespace ifce::os
