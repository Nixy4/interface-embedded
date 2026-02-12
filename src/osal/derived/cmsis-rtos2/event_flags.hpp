#pragma once

#include "osal/ability/event_flags.hpp"
#include "cmsis_os2.h"

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
    if (id_) return OsStatus::Busy;
    id_ = osEventFlagsNew(nullptr);
    return id_ ? OsStatus::Ok : OsStatus::NoMemory;
  }

  OsStatus DeleteImpl()
  {
    if (!id_) return OsStatus::Ok;
    osStatus_t rc = osEventFlagsDelete(id_);
    id_ = nullptr;
    return (rc == osOK) ? OsStatus::Ok : OsStatus::Error;
  }

  uint32_t SetImpl(uint32_t flags)
  {
    if (!id_) return 0;
    uint32_t result = osEventFlagsSet(id_, flags);
    return (result & 0x80000000u) ? 0 : result;  // error codes have MSB set
  }

  uint32_t ClearImpl(uint32_t flags)
  {
    if (!id_) return 0;
    uint32_t result = osEventFlagsClear(id_, flags);
    return (result & 0x80000000u) ? 0 : result;
  }

  uint32_t WaitImpl(uint32_t flags, bool wait_all, bool auto_clear, uint32_t timeout_ms)
  {
    if (!id_) return 0;
    uint32_t options = 0;
    if (wait_all)   options |= osFlagsWaitAll;
    else            options |= osFlagsWaitAny;
    if (!auto_clear) options |= osFlagsNoClear;

    uint32_t ticks = (timeout_ms == WaitForever) ? osWaitForever : timeout_ms;
    uint32_t result = osEventFlagsWait(id_, flags, options, ticks);
    return (result & 0x80000000u) ? 0 : result;
  }

  uint32_t GetImpl() const
  {
    if (!id_) return 0;
    uint32_t result = osEventFlagsGet(id_);
    return (result & 0x80000000u) ? 0 : result;
  }

public:
  osEventFlagsId_t GetHandle() const { return id_; }

private:
  osEventFlagsId_t id_ = nullptr;
};

} // namespace ifce::os
