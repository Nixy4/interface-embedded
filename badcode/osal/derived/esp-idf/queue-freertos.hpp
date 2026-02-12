#pragma once
#include "queue-base.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <cstddef>
#include <cstdint>

namespace osal {

template <typename ItemType>
class QueueFreeRTOS : public QueueBase<QueueFreeRTOS<ItemType>, ItemType>
{
public:
  QueueFreeRTOS() = default;
  ~QueueFreeRTOS()
  {
    if (handle_) {
      vQueueDelete(handle_);
      handle_ = nullptr;
    }
  }

  QueueFreeRTOS(const QueueFreeRTOS&) = delete;
  QueueFreeRTOS& operator=(const QueueFreeRTOS&) = delete;

  QueueFreeRTOS(QueueFreeRTOS&& other) noexcept
    : handle_(other.handle_), item_size_(other.item_size_), capacity_(other.capacity_)
  {
    other.handle_ = nullptr;
    other.item_size_ = 0;
    other.capacity_ = 0;
  }
  QueueFreeRTOS& operator=(QueueFreeRTOS&& other) noexcept
  {
    if (this != &other) {
      if (handle_) vQueueDelete(handle_);
      handle_ = other.handle_;
      item_size_ = other.item_size_;
      capacity_ = other.capacity_;
      other.handle_ = nullptr;
      other.item_size_ = 0;
      other.capacity_ = 0;
    }
    return *this;
  }

  void InitializeImpl(std::size_t item_size, std::size_t capacity)
  {
    item_size_ = item_size;
    capacity_  = capacity;
    if (handle_) vQueueDelete(handle_);
    handle_ = xQueueCreate(static_cast<UBaseType_t>(capacity_), static_cast<UBaseType_t>(item_size_));
  }

  bool PushImpl(const void* item)
  {
    if (!handle_) return false;
    return xQueueSendToBack(handle_, item, 0) == pdTRUE;
  }

  bool PullImpl(void* out)
  {
    if (!handle_) return false;
    return xQueueReceive(handle_, out, 0) == pdTRUE;
  }

  bool PushForImpl(const void* item, unsigned ms)
  {
    if (!handle_) return false;
    TickType_t to = (ms == 0) ? 0 : pdMS_TO_TICKS(ms);
    if (ms == (unsigned)UINT32_MAX) to = portMAX_DELAY;
    return xQueueSendToBack(handle_, item, to) == pdTRUE;
  }

  bool PullForImpl(void* out, unsigned ms)
  {
    if (!handle_) return false;
    TickType_t to = (ms == 0) ? 0 : pdMS_TO_TICKS(ms);
    if (ms == (unsigned)UINT32_MAX) to = portMAX_DELAY;
    return xQueueReceive(handle_, out, to) == pdTRUE;
  }

  bool PushBackImpl(const void* item)
  {
    if (!handle_) return false;
    return xQueueSendToBack(handle_, item, 0) == pdTRUE;
  }

  bool PushFrontImpl(const void* item)
  {
    if (!handle_) return false;
    return xQueueSendToFront(handle_, item, 0) == pdTRUE;
  }

  std::size_t SizeImpl()
  {
    if (!handle_) return 0;
    return static_cast<std::size_t>(uxQueueMessagesWaiting(handle_));
  }

  void ClearImpl()
  {
    if (!handle_) return;
    xQueueReset(handle_);
  }

private:
  QueueHandle_t handle_{nullptr};
  std::size_t item_size_{0};
  std::size_t capacity_{0};
};

} // namespace osal
