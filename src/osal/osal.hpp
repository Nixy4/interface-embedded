#pragma once

/// OSAL — Operating System Abstraction Layer
/// Master include header. Include this to get all OSAL primitives
/// for the currently configured backend.

#include "osal/types.hpp"
#include "osal/lock_guard.hpp"

#include "osal/thread.hpp"
#include "osal/mutex.hpp"
#include "osal/semaphore.hpp"
#include "osal/message_queue.hpp"
#include "osal/event_flags.hpp"
#include "osal/timer.hpp"
#include "osal/memory_pool.hpp"
#include "osal/delay.hpp"

// Logger is an independent module — use #include "logger/logger.hpp" directly
