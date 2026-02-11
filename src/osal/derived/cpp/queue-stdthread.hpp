#pragma once
#include "queue-base.hpp"
#include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <chrono>
#include <cstddef>
#include <cstdint>

template <typename ItemType>
class QueueStdThread : public QueueBase<QueueStdThread<ItemType>, ItemType>
{
public:
  QueueStdThread() = default;
  ~QueueStdThread() = default;

  QueueStdThread(const QueueStdThread&) = delete;
  QueueStdThread& operator=(const QueueStdThread&) = delete;

  QueueStdThread(QueueStdThread&& other) noexcept
  {
    std::lock_guard<std::mutex> lock(other.mtx_);
    item_size_ = other.item_size_;
    capacity_  = other.capacity_;
    dq_        = std::move(other.dq_);
  }
  QueueStdThread& operator=(QueueStdThread&& other) noexcept
  {
    if (this != &other) {
      std::scoped_lock lk(mtx_, other.mtx_);
      item_size_ = other.item_size_;
      capacity_  = other.capacity_;
      dq_        = std::move(other.dq_);
    }
    return *this;
  }

  void InitializeImpl(std::size_t item_size, std::size_t capacity)
  {
    std::lock_guard<std::mutex> lock(mtx_);
    item_size_ = item_size;
    capacity_  = capacity;
    dq_.clear();
  }

  bool PushImpl(const void* item)
  {
    std::lock_guard<std::mutex> lock(mtx_);
    if (dq_.size() >= capacity_) return false;
    dq_.push_back(copy_item(item));
    not_empty_.notify_one();
    return true;
  }

  bool PullImpl(void* out)
  {
    std::lock_guard<std::mutex> lock(mtx_);
    if (dq_.empty()) return false;
    copy_out(dq_.front(), out);
    dq_.pop_front();
    not_full_.notify_one();
    return true;
  }

  bool PushForImpl(const void* item, unsigned ms)
  {
    std::unique_lock<std::mutex> lock(mtx_);
    if (dq_.size() >= capacity_) {
      if (!not_full_.wait_for(lock, std::chrono::milliseconds(ms), [&]{ return dq_.size() < capacity_; })) {
        return false;
      }
    }
    dq_.push_back(copy_item(item));
    not_empty_.notify_one();
    return true;
  }

  bool PullForImpl(void* out, unsigned ms)
  {
    std::unique_lock<std::mutex> lock(mtx_);
    if (dq_.empty()) {
      if (!not_empty_.wait_for(lock, std::chrono::milliseconds(ms), [&]{ return !dq_.empty(); })) {
        return false;
      }
    }
    copy_out(dq_.front(), out);
    dq_.pop_front();
    not_full_.notify_one();
    return true;
  }

  bool PushBackImpl(const void* item)
  {
    return PushImpl(item);
  }

  bool PushFrontImpl(const void* item)
  {
    std::lock_guard<std::mutex> lock(mtx_);
    if (dq_.size() >= capacity_) return false;
    dq_.push_front(copy_item(item));
    not_empty_.notify_one();
    return true;
  }

  std::size_t SizeImpl()
  {
    std::lock_guard<std::mutex> lock(mtx_);
    return dq_.size();
  }

  void ClearImpl()
  {
    std::lock_guard<std::mutex> lock(mtx_);
    dq_.clear();
    not_full_.notify_all();
  }

private:
  std::vector<uint8_t> copy_item(const void* item)
  {
    std::vector<uint8_t> buf(item_size_);
    std::memcpy(buf.data(), item, item_size_);
    return buf;
  }

  void copy_out(const std::vector<uint8_t>& buf, void* out)
  {
    std::memcpy(out, buf.data(), item_size_);
  }

  std::deque<std::vector<uint8_t>> dq_;
  std::size_t item_size_{0};
  std::size_t capacity_{0};
  std::mutex mtx_;
  std::condition_variable not_empty_;
  std::condition_variable not_full_;
};
