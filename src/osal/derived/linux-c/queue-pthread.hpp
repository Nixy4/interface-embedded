#pragma once
#include "queue-base.hpp"
#include <deque>
#include <vector>
#include <pthread.h>
#include <time.h>
#include <cstring>
#include <cstddef>
#include <cstdint>

template <typename ItemType>
class QueuePthread : public QueueBase<QueuePthread<ItemType>, ItemType>
{
public:
  QueuePthread()
  {
    pthread_mutex_init(&mtx_, nullptr);
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
    pthread_cond_init(&not_empty_, &cattr);
    pthread_cond_init(&not_full_, &cattr);
    pthread_condattr_destroy(&cattr);
  }

  ~QueuePthread()
  {
    pthread_mutex_destroy(&mtx_);
    pthread_cond_destroy(&not_empty_);
    pthread_cond_destroy(&not_full_);
  }

  QueuePthread(const QueuePthread&) = delete;
  QueuePthread& operator=(const QueuePthread&) = delete;

  QueuePthread(QueuePthread&& other) noexcept
  {
    pthread_mutex_lock(&other.mtx_);
    item_size_ = other.item_size_;
    capacity_  = other.capacity_;
    dq_        = std::move(other.dq_);
    pthread_mutex_unlock(&other.mtx_);
  }
  QueuePthread& operator=(QueuePthread&& other) noexcept
  {
    if (this != &other) {
      pthread_mutex_lock(&mtx_);
      pthread_mutex_lock(&other.mtx_);
      item_size_ = other.item_size_;
      capacity_  = other.capacity_;
      dq_        = std::move(other.dq_);
      pthread_mutex_unlock(&other.mtx_);
      pthread_mutex_unlock(&mtx_);
    }
    return *this;
  }

  void InitializeImpl(std::size_t item_size, std::size_t capacity)
  {
    pthread_mutex_lock(&mtx_);
    item_size_ = item_size;
    capacity_  = capacity;
    dq_.clear();
    pthread_mutex_unlock(&mtx_);
  }

  bool PushImpl(const void* item)
  {
    pthread_mutex_lock(&mtx_);
    if (dq_.size() >= capacity_) {
      pthread_mutex_unlock(&mtx_);
      return false;
    }
    dq_.push_back(copy_item(item));
    pthread_cond_signal(&not_empty_);
    pthread_mutex_unlock(&mtx_);
    return true;
  }

  bool PullImpl(void* out)
  {
    pthread_mutex_lock(&mtx_);
    if (dq_.empty()) {
      pthread_mutex_unlock(&mtx_);
      return false;
    }
    copy_out(dq_.front(), out);
    dq_.pop_front();
    pthread_cond_signal(&not_full_);
    pthread_mutex_unlock(&mtx_);
    return true;
  }

  bool PushForImpl(const void* item, unsigned ms)
  {
    pthread_mutex_lock(&mtx_);
    if (dq_.size() >= capacity_) {
      struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);
      ts.tv_sec  += ms / 1000;
      ts.tv_nsec += (ms % 1000) * 1000000L;
      if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec += ts.tv_nsec / 1000000000L;
        ts.tv_nsec %= 1000000000L;
      }
      int rc = 0;
      while (dq_.size() >= capacity_ && rc == 0) {
        rc = pthread_cond_timedwait(&not_full_, &mtx_, &ts);
      }
      if (dq_.size() >= capacity_) {
        pthread_mutex_unlock(&mtx_);
        return false;
      }
    }
    dq_.push_back(copy_item(item));
    pthread_cond_signal(&not_empty_);
    pthread_mutex_unlock(&mtx_);
    return true;
  }

  bool PullForImpl(void* out, unsigned ms)
  {
    pthread_mutex_lock(&mtx_);
    if (dq_.empty()) {
      struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);
      ts.tv_sec  += ms / 1000;
      ts.tv_nsec += (ms % 1000) * 1000000L;
      if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec += ts.tv_nsec / 1000000000L;
        ts.tv_nsec %= 1000000000L;
      }
      int rc = 0;
      while (dq_.empty() && rc == 0) {
        rc = pthread_cond_timedwait(&not_empty_, &mtx_, &ts);
      }
      if (dq_.empty()) {
        pthread_mutex_unlock(&mtx_);
        return false;
      }
    }
    copy_out(dq_.front(), out);
    dq_.pop_front();
    pthread_cond_signal(&not_full_);
    pthread_mutex_unlock(&mtx_);
    return true;
  }

  bool PushBackImpl(const void* item)
  {
    return PushImpl(item);
  }

  bool PushFrontImpl(const void* item)
  {
    pthread_mutex_lock(&mtx_);
    if (dq_.size() >= capacity_) {
      pthread_mutex_unlock(&mtx_);
      return false;
    }
    dq_.push_front(copy_item(item));
    pthread_cond_signal(&not_empty_);
    pthread_mutex_unlock(&mtx_);
    return true;
  }

  std::size_t SizeImpl()
  {
    pthread_mutex_lock(&mtx_);
    auto s = dq_.size();
    pthread_mutex_unlock(&mtx_);
    return s;
  }

  void ClearImpl()
  {
    pthread_mutex_lock(&mtx_);
    dq_.clear();
    pthread_cond_broadcast(&not_full_);
    pthread_mutex_unlock(&mtx_);
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
  pthread_mutex_t mtx_{};
  pthread_cond_t not_empty_{};
  pthread_cond_t not_full_{};
};
