#pragma once
#include <type_traits>

namespace osal {

template <typename Derived>
class MutexBase
{
  friend Derived;
protected:
  MutexBase() = default;
  ~MutexBase() = default;
  
  // Disable copy
  MutexBase(const MutexBase&) = delete;
  MutexBase& operator=(const MutexBase&) = delete;
  
  // Enable move
  MutexBase(MutexBase&&) = default;
  MutexBase& operator=(MutexBase&&) = default;

public:
  void Initialize()
  {
    InitializeHelper<Derived>(0);
  }
  
  void SetRecursive(bool enable)
  {
    SetRecursiveHelper<Derived>(0, enable);
  }

  void Lock()
  {
    LockHelper<Derived>(0);
  }

  void Unlock()
  {
    UnlockHelper<Derived>(0);
  }

  bool TryLock()
  {
    return TryLockHelper<Derived>(0);
  }

  bool TryLockFor(unsigned ms)
  {
    return TryLockForHelper<Derived>(0, ms);
  }

private:
  template <typename T>
  auto InitializeHelper(int) -> decltype(std::declval<T>().InitializeImpl(), void())
  {
    static_cast<Derived*>(this)->InitializeImpl();
  }
  template <typename T>
  void InitializeHelper(long) {}

  template <typename T>
  auto LockHelper(int) -> decltype(std::declval<T>().LockImpl(), void())
  {
    static_cast<Derived*>(this)->LockImpl();
  }
  template <typename T>
  void LockHelper(long) {}

  template <typename T>
  auto UnlockHelper(int) -> decltype(std::declval<T>().UnlockImpl(), void())
  {
    static_cast<Derived*>(this)->UnlockImpl();
  }
  template <typename T>
  void UnlockHelper(long) {}

  template <typename T>
  auto TryLockHelper(int) -> decltype(std::declval<T>().TryLockImpl())
  {
    return static_cast<Derived*>(this)->TryLockImpl();
  }
  template <typename T>
  bool TryLockHelper(long)
  {
    return false;
  }

  template <typename T>
  auto TryLockForHelper(int, unsigned ms) -> decltype(std::declval<T>().TryLockForImpl(ms))
  {
    return static_cast<Derived*>(this)->TryLockForImpl(ms);
  }
  template <typename T>
  bool TryLockForHelper(long, unsigned)
  {
    return false;
  }
  
  template <typename T>
  auto SetRecursiveHelper(int, bool enable) -> decltype(std::declval<T>().SetRecursiveImpl(enable), void())
  {
    static_cast<Derived*>(this)->SetRecursiveImpl(enable);
  }
  template <typename T>
  void SetRecursiveHelper(long, bool) {}
};

}