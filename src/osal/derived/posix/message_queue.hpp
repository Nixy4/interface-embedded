#pragma once

#include "osal/ability/message_queue.hpp"
#include <pthread.h>
#include <ctime>
#include <cerrno>
#include <deque>

namespace ifce::os {

template <typename T>
class MessageQueue : public MessageQueueAbility<MessageQueue<T>, T>
{
  friend class MessageQueueAbility<MessageQueue<T>, T>;
  friend class ifce::DispatchBase<MessageQueue<T>>;

public:
  MessageQueue()  = default;
  ~MessageQueue() { DeleteImpl(); }

private:
  OsStatus CreateImpl(uint32_t capacity)
  {
    if (initialized_) return OsStatus::Busy;
    capacity_ = capacity;
    pthread_mutex_init(&mutex_, nullptr);
    pthread_cond_init(&cond_not_empty_, nullptr);
    pthread_cond_init(&cond_not_full_, nullptr);
    initialized_ = true;
    return OsStatus::Ok;
  }

  OsStatus DeleteImpl()
  {
    if (!initialized_) return OsStatus::Ok;
    pthread_cond_destroy(&cond_not_full_);
    pthread_cond_destroy(&cond_not_empty_);
    pthread_mutex_destroy(&mutex_);
    queue_.clear();
    initialized_ = false;
    return OsStatus::Ok;
  }

  OsStatus PutImpl(const T& msg, uint32_t timeout_ms)
  {
    if (!initialized_) return OsStatus::Error;
    pthread_mutex_lock(&mutex_);

    if (timeout_ms == WaitForever) {
      while (queue_.size() >= capacity_)
        pthread_cond_wait(&cond_not_full_, &mutex_);
    } else if (timeout_ms > 0) {
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      ts.tv_sec  += timeout_ms / 1000;
      ts.tv_nsec += (timeout_ms % 1000) * 1000000L;
      if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
      while (queue_.size() >= capacity_) {
        if (pthread_cond_timedwait(&cond_not_full_, &mutex_, &ts) == ETIMEDOUT) {
          pthread_mutex_unlock(&mutex_);
          return OsStatus::Timeout;
        }
      }
    } else {
      if (queue_.size() >= capacity_) {
        pthread_mutex_unlock(&mutex_);
        return OsStatus::Timeout;
      }
    }

    queue_.push_back(msg);
    pthread_cond_signal(&cond_not_empty_);
    pthread_mutex_unlock(&mutex_);
    return OsStatus::Ok;
  }

  OsStatus GetImpl(T& msg, uint32_t timeout_ms)
  {
    if (!initialized_) return OsStatus::Error;
    pthread_mutex_lock(&mutex_);

    if (timeout_ms == WaitForever) {
      while (queue_.empty())
        pthread_cond_wait(&cond_not_empty_, &mutex_);
    } else if (timeout_ms > 0) {
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      ts.tv_sec  += timeout_ms / 1000;
      ts.tv_nsec += (timeout_ms % 1000) * 1000000L;
      if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
      while (queue_.empty()) {
        if (pthread_cond_timedwait(&cond_not_empty_, &mutex_, &ts) == ETIMEDOUT) {
          pthread_mutex_unlock(&mutex_);
          return OsStatus::Timeout;
        }
      }
    } else {
      if (queue_.empty()) {
        pthread_mutex_unlock(&mutex_);
        return OsStatus::Timeout;
      }
    }

    msg = queue_.front();
    queue_.pop_front();
    pthread_cond_signal(&cond_not_full_);
    pthread_mutex_unlock(&mutex_);
    return OsStatus::Ok;
  }

  OsStatus PutToFrontImpl(const T& msg, uint32_t timeout_ms)
  {
    if (!initialized_) return OsStatus::Error;
    pthread_mutex_lock(&mutex_);

    if (timeout_ms == WaitForever) {
      while (queue_.size() >= capacity_)
        pthread_cond_wait(&cond_not_full_, &mutex_);
    } else {
      if (queue_.size() >= capacity_) {
        pthread_mutex_unlock(&mutex_);
        return OsStatus::Timeout;
      }
    }

    queue_.push_front(msg);
    pthread_cond_signal(&cond_not_empty_);
    pthread_mutex_unlock(&mutex_);
    return OsStatus::Ok;
  }

  uint32_t GetCountImpl() const
  {
    return static_cast<uint32_t>(queue_.size());
  }

  uint32_t GetCapacityImpl() const { return capacity_; }

  OsStatus ResetImpl()
  {
    if (!initialized_) return OsStatus::Error;
    pthread_mutex_lock(&mutex_);
    queue_.clear();
    pthread_cond_broadcast(&cond_not_full_);
    pthread_mutex_unlock(&mutex_);
    return OsStatus::Ok;
  }

private:
  pthread_mutex_t mutex_          = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  cond_not_empty_ = PTHREAD_COND_INITIALIZER;
  pthread_cond_t  cond_not_full_  = PTHREAD_COND_INITIALIZER;
  std::deque<T>   queue_;
  uint32_t        capacity_       = 0;
  bool            initialized_    = false;
};

} // namespace ifce::os
