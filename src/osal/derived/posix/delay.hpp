#pragma once

#include "osal/types.hpp"
#include <ctime>
#include <cstdint>
#include <unistd.h>

namespace ifce::os {

inline void Delay(uint32_t ms)
{
  struct timespec ts;
  ts.tv_sec  = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000L;
  nanosleep(&ts, nullptr);
}

inline void DelayUntil(uint32_t* previous_wake, uint32_t increment_ms)
{
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  uint32_t now_ms = static_cast<uint32_t>(now.tv_sec * 1000 + now.tv_nsec / 1000000);
  uint32_t target = *previous_wake + increment_ms;

  if (target > now_ms) {
    uint32_t remaining = target - now_ms;
    struct timespec ts;
    ts.tv_sec  = remaining / 1000;
    ts.tv_nsec = (remaining % 1000) * 1000000L;
    nanosleep(&ts, nullptr);
  }
  *previous_wake = target;
}

inline uint32_t GetTickCount()
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<uint32_t>(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

inline uint32_t GetTickFreq()
{
  return 1000; // millisecond resolution
}

} // namespace ifce::os
