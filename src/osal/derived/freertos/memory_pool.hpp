#pragma once

#include "osal/ability/memory_pool.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>

namespace ifce::os {

/// Fixed-block memory pool for FreeRTOS, implemented as a free-list
/// protected by a FreeRTOS mutex.
template <typename T>
class MemoryPool : public MemoryPoolAbility<MemoryPool<T>, T>
{
  friend class MemoryPoolAbility<MemoryPool<T>, T>;
  friend class ifce::DispatchBase<MemoryPool<T>>;

public:
  MemoryPool()  = default;
  ~MemoryPool() { DeleteImpl(); }

private:
  // Each free block stores a pointer to the next free block
  struct FreeNode { FreeNode* next; };

  static constexpr size_t kBlockSize =
    sizeof(T) > sizeof(FreeNode) ? sizeof(T) : sizeof(FreeNode);
  // Ensure proper alignment
  static constexpr size_t kAlignment = alignof(T) > alignof(FreeNode) ? alignof(T) : alignof(FreeNode);

  OsStatus CreateImpl(uint32_t block_count)
  {
    if (pool_ || lock_) return OsStatus::Busy;

    lock_ = xSemaphoreCreateMutex();
    if (!lock_) return OsStatus::NoMemory;

    // Allocate aligned memory
    size_t aligned_block = ((kBlockSize + kAlignment - 1) / kAlignment) * kAlignment;
    pool_ = static_cast<uint8_t*>(std::malloc(aligned_block * block_count));
    if (!pool_) {
      vSemaphoreDelete(lock_);
      lock_ = nullptr;
      return OsStatus::NoMemory;
    }

    block_count_ = block_count;
    free_count_  = block_count;
    block_size_  = aligned_block;

    // Build free list
    free_head_ = nullptr;
    for (uint32_t i = 0; i < block_count; ++i) {
      auto* node = reinterpret_cast<FreeNode*>(pool_ + i * aligned_block);
      node->next = free_head_;
      free_head_ = node;
    }

    return OsStatus::Ok;
  }

  OsStatus DeleteImpl()
  {
    if (lock_) {
      vSemaphoreDelete(lock_);
      lock_ = nullptr;
    }
    if (pool_) {
      std::free(pool_);
      pool_ = nullptr;
    }
    free_head_   = nullptr;
    block_count_ = 0;
    free_count_  = 0;
    return OsStatus::Ok;
  }

  T* AllocImpl(uint32_t timeout_ms)
  {
    if (!lock_ || !pool_) return nullptr;
    TickType_t ticks = (timeout_ms == WaitForever) ? portMAX_DELAY
                       : pdMS_TO_TICKS(timeout_ms);
    if (xSemaphoreTake(lock_, ticks) != pdTRUE)
      return nullptr;

    T* result = nullptr;
    if (free_head_) {
      result = reinterpret_cast<T*>(free_head_);
      free_head_ = free_head_->next;
      --free_count_;
    }

    xSemaphoreGive(lock_);
    return result;
  }

  OsStatus FreeImpl(T* block)
  {
    if (!lock_ || !pool_ || !block) return OsStatus::Error;

    // Validate pointer is within pool range
    auto* ptr = reinterpret_cast<uint8_t*>(block);
    if (ptr < pool_ || ptr >= pool_ + block_size_ * block_count_)
      return OsStatus::Error;

    if (xSemaphoreTake(lock_, portMAX_DELAY) != pdTRUE)
      return OsStatus::Error;

    auto* node = reinterpret_cast<FreeNode*>(block);
    node->next = free_head_;
    free_head_ = node;
    ++free_count_;

    xSemaphoreGive(lock_);
    return OsStatus::Ok;
  }

  uint32_t GetCountImpl() const { return block_count_; }
  uint32_t GetFreeCountImpl() const { return free_count_; }

private:
  SemaphoreHandle_t lock_        = nullptr;
  uint8_t*          pool_        = nullptr;
  FreeNode*         free_head_   = nullptr;
  uint32_t          block_count_ = 0;
  uint32_t          free_count_  = 0;
  size_t            block_size_  = 0;
};

} // namespace ifce::os
