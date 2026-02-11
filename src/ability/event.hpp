#pragma once

#include <type_traits>
#include <utility>

namespace interface
{
    template <typename Derived, typename DerivedConfig, typename EventType>
    class EventAbility
    {
        friend Derived;

    protected:
        // Configure
        template <typename T>
        auto ConfigureHelper(int, DerivedConfig &&config)
            -> decltype(std::declval<T>().ConfigureImpl(std::forward<DerivedConfig>(config)), bool())
        {
            return static_cast<Derived *>(this)->ConfigureImpl(std::forward<DerivedConfig>(config));
        }

        template <typename T>
        bool ConfigureHelper(long, DerivedConfig &&config)
        {
            return false;
        }

        // Create
        template <typename T>
        auto CreateHelper(int)
            -> decltype(std::declval<T>().CreateImpl(), bool())
        {
            return static_cast<Derived *>(this)->CreateImpl();
        }

        template <typename T>
        bool CreateHelper(long)
        {
            return false;
        }

        // Delete
        template <typename T>
        auto DeleteHelper(int)
            -> decltype(std::declval<T>().DeleteImpl(), bool())
        {
            return static_cast<Derived *>(this)->DeleteImpl();
        }

        template <typename T>
        bool DeleteHelper(long)
        {
            return false;
        }

        // Send (const ref)
        template <typename T>
        auto SendHelper(int, const EventType &event)
            -> decltype(std::declval<T>().SendImpl(event), bool())
        {
            return static_cast<Derived *>(this)->SendImpl(event);
        }

        template <typename T>
        bool SendHelper(long, const EventType &event)
        {
            return false;
        }

        // Send (rvalue ref)
        template <typename T>
        auto SendHelper(int, EventType &&event)
            -> decltype(std::declval<T>().SendImpl(std::move(event)), bool())
        {
            return static_cast<Derived *>(this)->SendImpl(std::move(event));
        }

        template <typename T>
        bool SendHelper(long, EventType &&event)
        {
            return false;
        }

        // Receive
        template <typename T>
        auto ReceiveHelper(int, EventType &event, int timeout)
            -> decltype(std::declval<T>().ReceiveImpl(event, timeout), bool())
        {
            return static_cast<Derived *>(this)->ReceiveImpl(event, timeout);
        }

        template <typename T>
        bool ReceiveHelper(long, EventType &event, int timeout)
        {
            return false;
        }

        // Wait
        template <typename T>
        auto WaitHelper(int, EventType &event, int timeout)
            -> decltype(std::declval<T>().WaitImpl(event, timeout), bool())
        {
            return static_cast<Derived *>(this)->WaitImpl(event, timeout);
        }

        template <typename T>
        bool WaitHelper(long, EventType &event, int timeout)
        {
            return false;
        }

        // Reset
        template <typename T>
        auto ResetHelper(int)
            -> decltype(std::declval<T>().ResetImpl(), bool())
        {
            return static_cast<Derived *>(this)->ResetImpl();
        }

        template <typename T>
        bool ResetHelper(long)
        {
            return false;
        }

        // IsEmpty
        template <typename T>
        auto IsEmptyHelper(int)
            -> decltype(std::declval<T>().IsEmptyImpl(), bool())
        {
            return static_cast<Derived *>(this)->IsEmptyImpl();
        }

        template <typename T>
        bool IsEmptyHelper(long)
        {
            return false;
        }

        // HasEvent
        template <typename T>
        auto HasEventHelper(int)
            -> decltype(std::declval<T>().HasEventImpl(), bool())
        {
            return static_cast<Derived *>(this)->HasEventImpl();
        }

        template <typename T>
        bool HasEventHelper(long)
        {
            return false;
        }

    public:
        EventAbility() = default;
        ~EventAbility() = default;

        bool Configure(DerivedConfig &&config)
        {
            return ConfigureHelper<Derived>(0, std::forward<DerivedConfig>(config));
        }

        bool Create()
        {
            return CreateHelper<Derived>(0);
        }

        bool Delete()
        {
            return DeleteHelper<Derived>(0);
        }

        bool Send(const EventType &event)
        {
            return SendHelper<Derived>(0, event);
        }

        bool Send(EventType &&event)
        {
            return SendHelper<Derived>(0, std::move(event));
        }

        bool Receive(EventType &event, int timeout)
        {
            return ReceiveHelper<Derived>(0, event, timeout);
        }

        bool Wait(EventType &event, int timeout)
        {
            return WaitHelper<Derived>(0, event, timeout);
        }

        bool Reset()
        {
            return ResetHelper<Derived>(0);
        }

        bool IsEmpty()
        {
            return IsEmptyHelper<Derived>(0);
        }

        bool HasEvent()
        {
            return HasEventHelper<Derived>(0);
        }
    };
} // namespace interface
