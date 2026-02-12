#pragma once

#include "osal/types.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdint>

namespace osal {

inline void Delay(uint32_t ms)
{
  vTaskDelay(pdMS_TO_TICKS(ms));
}

inline void DelayUntil(uint32_t* previous_wake, uint32_t increment_ms)
{
  TickType_t prev = static_cast<TickType_t>(*previous_wake);
  xTaskDelayUntil(&prev, pdMS_TO_TICKS(increment_ms));
  *previous_wake = static_cast<uint32_t>(prev);
}

inline uint32_t GetTickCount()
{
  return static_cast<uint32_t>(xTaskGetTickCount());
}

inline uint32_t GetTickFreq()
{
  return configTICK_RATE_HZ;
}

} // namespace osal
