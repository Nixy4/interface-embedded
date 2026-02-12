#pragma once

#if CONFIG_INTERFACE_EMBEDDED_OSAL_ESP_IDF
#include "osal/derived/esp-idf/logger-esp.hpp"
#elif CONFIG_INTERFACE_EMBEDDED_OSAL_POSIX
#include "osal/derived/posix/logger-stdio.hpp"
#elif CONFIG_INTERFACE_EMBEDDED_OSAL_CPP_STD
#include "osal/derived/cppstd/logger-stdio.hpp"
#endif