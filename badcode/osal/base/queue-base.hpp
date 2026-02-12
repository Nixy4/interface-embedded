#pragma once
#include <type_traits>
#include <utility>
#include <cstddef>
#include <cstring>

namespace osal {

template <typename Derived, typename ItemType>
class QueueBase
{
  friend Derived;
protected:
  QueueBase() = default;
  ~QueueBase() = default;
  
  // Disable copy
  QueueBase(const QueueBase&) = delete;
  QueueBase& operator=(const QueueBase&) = delete;
  
  // Enable move
  QueueBase(QueueBase&&) = default;
  QueueBase& operator=(QueueBase&&) = default;

public:
  void Initialize(std::size_t capacity)
  {
    InitializeHelper<Derived>(0, sizeof(ItemType), capacity);
  }

  bool Push(const ItemType& item)
  {
    return PushHelper<Derived>(0, static_cast<const void*>(&item));
  }

  bool PushFor(const ItemType& item, unsigned ms)
  {
    return PushForHelper<Derived>(0, static_cast<const void*>(&item), ms);
  }

  bool Pull(ItemType& out)
  {
    return PullHelper<Derived>(0, static_cast<void*>(&out));
  }

  bool PullFor(ItemType& out, unsigned ms)
  {
    return PullForHelper<Derived>(0, static_cast<void*>(&out), ms);
  }

  bool PushBack(const ItemType& item)
  {
    return PushBackHelper<Derived>(0, static_cast<const void*>(&item));
  }

  bool PushFront(const ItemType& item)
  {
    return PushFrontHelper<Derived>(0, static_cast<const void*>(&item));
  }

  std::size_t Size()
  {
    return SizeHelper<Derived>(0);
  }

  void Clear()
  {
    ClearHelper<Derived>(0);
  }

private:
  template <typename T>
  auto InitializeHelper(int, std::size_t item_size, std::size_t capacity) -> decltype(std::declval<T>().InitializeImpl(item_size, capacity), void())
  {
    static_cast<Derived*>(this)->InitializeImpl(item_size, capacity);
  }
  template <typename T>
  void InitializeHelper(long, std::size_t, std::size_t) {}

  template <typename T>
  auto PushHelper(int, const void* item) -> decltype(std::declval<T>().PushImpl(item))
  {
    return static_cast<Derived*>(this)->PushImpl(item);
  }
  template <typename T>
  bool PushHelper(long, const void*)
  {
    return false;
  }

  template <typename T>
  auto PushForHelper(int, const void* item, unsigned ms) -> decltype(std::declval<T>().PushForImpl(item, ms))
  {
    return static_cast<Derived*>(this)->PushForImpl(item, ms);
  }
  template <typename T>
  bool PushForHelper(long, const void*, unsigned)
  {
    return false;
  }

  template <typename T>
  auto PullHelper(int, void* out) -> decltype(std::declval<T>().PullImpl(out))
  {
    return static_cast<Derived*>(this)->PullImpl(out);
  }
  template <typename T>
  bool PullHelper(long, void*)
  {
    return false;
  }

  template <typename T>
  auto PullForHelper(int, void* out, unsigned ms) -> decltype(std::declval<T>().PullForImpl(out, ms))
  {
    return static_cast<Derived*>(this)->PullForImpl(out, ms);
  }
  template <typename T>
  bool PullForHelper(long, void*, unsigned)
  {
    return false;
  }

  template <typename T>
  auto PushBackHelper(int, const void* item) -> decltype(std::declval<T>().PushBackImpl(item))
  {
    return static_cast<Derived*>(this)->PushBackImpl(item);
  }
  template <typename T>
  bool PushBackHelper(long, const void*)
  {
    return false;
  }

  template <typename T>
  auto PushFrontHelper(int, const void* item) -> decltype(std::declval<T>().PushFrontImpl(item))
  {
    return static_cast<Derived*>(this)->PushFrontImpl(item);
  }
  template <typename T>
  bool PushFrontHelper(long, const void*)
  {
    return false;
  }

  template <typename T>
  auto SizeHelper(int) -> decltype(std::declval<T>().SizeImpl())
  {
    return static_cast<Derived*>(this)->SizeImpl();
  }
  template <typename T>
  std::size_t SizeHelper(long)
  {
    return 0;
  }

  template <typename T>
  auto ClearHelper(int) -> decltype(std::declval<T>().ClearImpl(), void())
  {
    static_cast<Derived*>(this)->ClearImpl();
  }
  template <typename T>
  void ClearHelper(long) {}
};

} // namespace osal