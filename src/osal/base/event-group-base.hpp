#pragma once
#include <cstdint>
#include <utility>

template <typename Derived>
class EventGroupBase {
  friend Derived;
protected:
    EventGroupBase() = default;
    ~EventGroupBase() = default;
    EventGroupBase(const EventGroupBase&) = delete;
    EventGroupBase& operator=(const EventGroupBase&) = delete;
    EventGroupBase(EventGroupBase&&) = default;
    EventGroupBase& operator=(EventGroupBase&&) = default;

public:
    void Initialize() { InitializeHelper<Derived>(0); }
    void SetBits(uint32_t mask) { SetBitsHelper<Derived>(0, mask); }
    uint32_t WaitBits(uint32_t mask, bool wait_all = false, bool clear = false) { return WaitBitsHelper<Derived>(0, mask, wait_all, clear); }
    uint32_t WaitBitsFor(uint32_t mask, bool wait_all, bool clear, unsigned ms) { return WaitBitsForHelper<Derived>(0, mask, wait_all, clear, ms); }
    uint32_t TryWaitBits(uint32_t mask, bool wait_all = false, bool clear = false) { return TryWaitBitsHelper<Derived>(0, mask, wait_all, clear); }
    void ClearBits(uint32_t mask) { ClearBitsHelper<Derived>(0, mask); }
    uint32_t GetBits() const { return GetBitsHelper<Derived>(0); }

private:
    template <typename T>
    auto InitializeHelper(int) -> decltype(std::declval<T>().InitializeImpl(), void()) {
        static_cast<Derived*>(this)->InitializeImpl();
    }
    template <typename T>
    void InitializeHelper(long) {}

    template <typename T>
    auto SetBitsHelper(int, uint32_t mask) -> decltype(std::declval<T>().SetBitsImpl(mask), void()) {
        static_cast<Derived*>(this)->SetBitsImpl(mask);
    }
    template <typename T>
    void SetBitsHelper(long, uint32_t) {}

    template <typename T>
    auto WaitBitsHelper(int, uint32_t mask, bool wait_all, bool clear)
        -> decltype(std::declval<T>().WaitBitsImpl(mask, wait_all, clear)) {
        return static_cast<Derived*>(this)->WaitBitsImpl(mask, wait_all, clear);
    }
    template <typename T>
    uint32_t WaitBitsHelper(long, uint32_t, bool, bool) { return 0; }

    template <typename T>
    auto WaitBitsForHelper(int, uint32_t mask, bool wait_all, bool clear, unsigned ms)
        -> decltype(std::declval<T>().WaitBitsForImpl(mask, wait_all, clear, ms)) {
        return static_cast<Derived*>(this)->WaitBitsForImpl(mask, wait_all, clear, ms);
    }
    template <typename T>
    uint32_t WaitBitsForHelper(long, uint32_t, bool, bool, unsigned) { return 0; }

    template <typename T>
    auto TryWaitBitsHelper(int, uint32_t mask, bool wait_all, bool clear)
        -> decltype(std::declval<T>().TryWaitBitsImpl(mask, wait_all, clear)) {
        return static_cast<Derived*>(this)->TryWaitBitsImpl(mask, wait_all, clear);
    }
    template <typename T>
    uint32_t TryWaitBitsHelper(long, uint32_t, bool, bool) { return 0; }

    template <typename T>
    auto ClearBitsHelper(int, uint32_t mask) -> decltype(std::declval<T>().ClearBitsImpl(mask), void()) {
        static_cast<Derived*>(this)->ClearBitsImpl(mask);
    }
    template <typename T>
    void ClearBitsHelper(long, uint32_t) {}

    template <typename T>
    auto GetBitsHelper(int) const -> decltype(std::declval<const T>().GetBitsImpl()) {
        return static_cast<const Derived*>(this)->GetBitsImpl();
    }
    template <typename T>
    uint32_t GetBitsHelper(long) const { return 0; }
};
