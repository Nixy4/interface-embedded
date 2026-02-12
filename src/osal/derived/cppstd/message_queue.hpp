#pragma once

#include "osal/ability/message_queue.hpp"
#include <mutex>
#include <condition_variable>
#include <deque>
#include <chrono>

namespace osal {

template <typename T>
class MessageQueue : public MessageQueueAbility<MessageQueue<T>, T>
{
  friend class MessageQueueAbility<MessageQueue<T>, T>;
  friend class DispatchBase<MessageQueue<T>>;

public:
  MessageQueue()  = default;
  ~MessageQueue() { DeleteImpl(); }

private:
  OsStatus CreateImpl(uint32_t capacity)
  {
    if (initialized_) return OsStatus::Busy;
    capacity_    = capacity;
    initialized_ = true;
    return OsStatus::Ok;
  }

  OsStatus DeleteImpl()
  {
    if (!initialized_) return OsStatus::Ok;
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
    initialized_ = false;
    return OsStatus::Ok;
  }

  OsStatus PutImpl(const T& msg, uint32_t timeout_ms)
  {
    if (!initialized_) return OsStatus::Error;
    std::unique_lock<std::mutex> lock(mutex_);

    if (timeout_ms == WaitForever) {
      cv_not_full_.wait(lock, [this] { return queue_.size() < capacity_; });
    } else if (timeout_ms > 0) {
      if (!cv_not_full_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                                  [this] { return queue_.size() < capacity_; }))
        return OsStatus::Timeout;
    } else {
      if (queue_.size() >= capacity_) return OsStatus::Timeout;
    }

    queue_.push_back(msg);
    cv_not_empty_.notify_one();
    return OsStatus::Ok;
  }

  OsStatus GetImpl(T& msg, uint32_t timeout_ms)
  {
    if (!initialized_) return OsStatus::Error;
    std::unique_lock<std::mutex> lock(mutex_);

    if (timeout_ms == WaitForever) {
      cv_not_empty_.wait(lock, [this] { return !queue_.empty(); });
    } else if (timeout_ms > 0) {
      if (!cv_not_empty_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                                   [this] { return !queue_.empty(); }))
        return OsStatus::Timeout;
    } else {
      if (queue_.empty()) return OsStatus::Timeout;
    }

    msg = queue_.front();
    queue_.pop_front();
    cv_not_full_.notify_one();
    return OsStatus::Ok;
  }

  OsStatus PutToFrontImpl(const T& msg, uint32_t timeout_ms)
  {
    if (!initialized_) return OsStatus::Error;
    std::unique_lock<std::mutex> lock(mutex_);

    if (timeout_ms == WaitForever) {
      cv_not_full_.wait(lock, [this] { return queue_.size() < capacity_; });
    } else {
      if (queue_.size() >= capacity_) return OsStatus::Timeout;
    }

    queue_.push_front(msg);
    cv_not_empty_.notify_one();
    return OsStatus::Ok;
  }

  uint32_t GetCountImpl() const { return static_cast<uint32_t>(queue_.size()); }
  uint32_t GetCapacityImpl() const { return capacity_; }

  OsStatus ResetImpl()
  {
    if (!initialized_) return OsStatus::Error;
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
    cv_not_full_.notify_all();
    return OsStatus::Ok;
  }

private:
  std::mutex              mutex_;
  std::condition_variable cv_not_empty_;
  std::condition_variable cv_not_full_;
  std::deque<T>           queue_;
  uint32_t                capacity_    = 0;
  bool                    initialized_ = false;
};

} // namespace osal
