#pragma once

#include <cstdint>
#include <functional>

namespace osal {

/// Unified return status (aligned with osStatus_t semantics)
enum class OsStatus : int32_t
{
  Ok          =  0,
  Error       = -1,
  Timeout     = -2,
  NoMemory    = -3,
  NotFound    = -4,
  Busy        = -5,
  NotReady    = -6,
};

/// Normalized thread priority levels (0-100)
enum class ThreadPriority : int
{
  Idle        =  0,
  Low         = 10,
  BelowNormal = 25,
  Normal      = 50,
  AboveNormal = 60,
  High        = 75,
  Realtime    = 99,
};

/// Infinite wait sentinel
static constexpr uint32_t WaitForever = 0xFFFFFFFFu;

/// Common callback signatures
using ThreadFunc = std::function<void(void*)>;
using TimerFunc  = std::function<void(void*)>;

/// Map a normalized priority (0-100) to a platform-specific range [min, max]
inline int MapPriority(int normalized, int platform_min, int platform_max)
{
  if (normalized < 0)   normalized = 0;
  if (normalized > 100) normalized = 100;
  return platform_min + (normalized * (platform_max - platform_min)) / 100;
}

} // namespace osal
