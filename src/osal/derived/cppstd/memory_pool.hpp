#pragma once

#include "osal/ability/memory_pool.hpp"
#include <mutex>
#include <cstdlib>
#include <cstdint>
#include <new>

namespace osal {

template <typename T>
class MemoryPool : public MemoryPoolAbility<MemoryPool<T>, T>
{
  friend class MemoryPoolAbility<MemoryPool<T>, T>;
  friend class DispatchBase<MemoryPool<T>>;

public:
  MemoryPool()  = default;
  ~MemoryPool() { DeleteImpl(); }

private:
  struct FreeNode { FreeNode* next; };

  static constexpr size_t kBlockSize =
    sizeof(T) > sizeof(FreeNode) ? sizeof(T) : sizeof(FreeNode);
  static constexpr size_t kAlignment =
    alignof(T) > alignof(FreeNode) ? alignof(T) : alignof(FreeNode);

  OsStatus CreateImpl(uint32_t block_count)
  {
    if (pool_) return OsStatus::Busy;

    size_t aligned_block = ((kBlockSize + kAlignment - 1) / kAlignment) * kAlignment;
    pool_ = static_cast<uint8_t*>(std::malloc(aligned_block * block_count));
    if (!pool_) return OsStatus::NoMemory;

    block_count_ = block_count;
    free_count_  = block_count;
    block_size_  = aligned_block;

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
    if (!pool_) return OsStatus::Ok;
    std::free(pool_);
    pool_        = nullptr;
    free_head_   = nullptr;
    block_count_ = 0;
    free_count_  = 0;
    return OsStatus::Ok;
  }

  T* AllocImpl(uint32_t /*timeout_ms*/)
  {
    if (!pool_) return nullptr;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!free_head_) return nullptr;
    T* result = reinterpret_cast<T*>(free_head_);
    free_head_ = free_head_->next;
    --free_count_;
    return result;
  }

  OsStatus FreeImpl(T* block)
  {
    if (!pool_ || !block) return OsStatus::Error;
    auto* ptr = reinterpret_cast<uint8_t*>(block);
    if (ptr < pool_ || ptr >= pool_ + block_size_ * block_count_)
      return OsStatus::Error;

    std::lock_guard<std::mutex> lock(mutex_);
    auto* node = reinterpret_cast<FreeNode*>(block);
    node->next = free_head_;
    free_head_ = node;
    ++free_count_;
    return OsStatus::Ok;
  }

  uint32_t GetCountImpl() const { return block_count_; }
  uint32_t GetFreeCountImpl() const { return free_count_; }

private:
  std::mutex  mutex_;
  uint8_t*    pool_        = nullptr;
  FreeNode*   free_head_   = nullptr;
  uint32_t    block_count_ = 0;
  uint32_t    free_count_  = 0;
  size_t      block_size_  = 0;
};

} // namespace osal
