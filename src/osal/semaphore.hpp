#pragma once

#if defined(CONFIG_INTERFACE_EMBEDDED_OSAL_FREERTOS) || defined(OSAL_BACKEND_FREERTOS)
  #include "osal/derived/freertos/semaphore.hpp"
#elif defined(CONFIG_INTERFACE_EMBEDDED_OSAL_CMSIS_RTOS2) || defined(OSAL_BACKEND_CMSIS_RTOS2)
  #include "osal/derived/cmsis-rtos2/semaphore.hpp"
#elif defined(CONFIG_INTERFACE_EMBEDDED_OSAL_POSIX) || defined(OSAL_BACKEND_POSIX)
  #include "osal/derived/posix/semaphore.hpp"
#elif defined(CONFIG_INTERFACE_EMBEDDED_OSAL_CPP_STD) || defined(OSAL_BACKEND_CPP_STD)
  #include "osal/derived/cppstd/semaphore.hpp"
#else
  #error "No OSAL backend selected for Semaphore"
#endif
