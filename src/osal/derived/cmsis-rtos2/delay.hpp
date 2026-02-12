#pragma once

#include "osal/types.hpp"
#include "cmsis_os2.h"
#include <cstdint>

namespace ifce::os {

inline void Delay(uint32_t ms)
{
  osDelay(ms);
}

inline void DelayUntil(uint32_t* previous_wake, uint32_t increment_ms)
{
  (void)previous_wake;
  osDelayUntil(osKernelGetTickCount() + increment_ms);
}

inline uint32_t GetTickCount()
{
  return osKernelGetTickCount();
}

inline uint32_t GetTickFreq()
{
  return osKernelGetTickFreq();
}

} // namespace ifce::os
