#pragma once

#if __has_include("sdkconfig.h")
#include "sdkconfig.h"
#endif

#if CONFIG_INTERFACE_EMBEDDED_OSAL_ESP_IDF
#include "osal/derived/esp-idf/logger.hpp"
#elif CONFIG_INTERFACE_EMBEDDED_OSAL_POSIX
#include "osal/derived/posix/logger.hpp"
#elif CONFIG_INTERFACE_EMBEDDED_OSAL_CPP_STD
#include "osal/derived/cppstd/logger.hpp"
#else
#error "No OSAL implementation selected"
#endif