#pragma once

#include "osal/types.hpp"
#include <thread>
#include <chrono>
#include <cstdint>

namespace osal {

inline void Delay(uint32_t ms)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void DelayUntil(uint32_t* previous_wake, uint32_t increment_ms)
{
  auto now = std::chrono::steady_clock::now();
  auto now_ms = static_cast<uint32_t>(
    std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch()).count());

  uint32_t target = *previous_wake + increment_ms;
  if (target > now_ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(target - now_ms));
  }
  *previous_wake = target;
}

inline uint32_t GetTickCount()
{
  return static_cast<uint32_t>(
    std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count());
}

inline uint32_t GetTickFreq()
{
  return 1000; // millisecond resolution
}

} // namespace osal
