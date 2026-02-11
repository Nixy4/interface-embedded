#pragma once

#include <vector>
#include <type_traits>
#include <utility>

namespace interface
{
    template <typename Derived, typename DerivedConfig, typename DataType>
    class QueueAbility
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

        // Send
        template <typename T>
        auto SendHelper(int, const std::vector<DataType> &data, int timeout)
            -> decltype(std::declval<T>().SendImpl(data, timeout), bool())
        {
            return static_cast<Derived *>(this)->SendImpl(data, timeout);
        }

        template <typename T>
        bool SendHelper(long, const std::vector<DataType> &data, int timeout)
        {
            return false;
        }

        // SendToFront
        template <typename T>
        auto SendToFrontHelper(int, const std::vector<DataType> &data, int timeout)
            -> decltype(std::declval<T>().SendToFrontImpl(data, timeout), bool())
        {
            return static_cast<Derived *>(this)->SendToFrontImpl(data, timeout);
        }

        template <typename T>
        bool SendToFrontHelper(long, const std::vector<DataType> &data, int timeout)
        {
            return false;
        }

        // SendToBack
        template <typename T>
        auto SendToBackHelper(int, const std::vector<DataType> &data, int timeout)
            -> decltype(std::declval<T>().SendToBackImpl(data, timeout), bool())
        {
            return static_cast<Derived *>(this)->SendToBackImpl(data, timeout);
        }

        template <typename T>
        bool SendToBackHelper(long, const std::vector<DataType> &data, int timeout)
        {
            return false;
        }

        // Receive
        template <typename T>
        auto ReceiveHelper(int, std::vector<DataType> &data, int timeout)
            -> decltype(std::declval<T>().ReceiveImpl(data, timeout), bool())
        {
            return static_cast<Derived *>(this)->ReceiveImpl(data, timeout);
        }

        template <typename T>
        bool ReceiveHelper(long, std::vector<DataType> &data, int timeout)
        {
            return false;
        }

        // Peek
        template <typename T>
        auto PeekHelper(int, std::vector<DataType> &data, int timeout)
            -> decltype(std::declval<T>().PeekImpl(data, timeout), bool())
        {
            return static_cast<Derived *>(this)->PeekImpl(data, timeout);
        }

        template <typename T>
        bool PeekHelper(long, std::vector<DataType> &data, int timeout)
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

        // IsFull
        template <typename T>
        auto IsFullHelper(int)
            -> decltype(std::declval<T>().IsFullImpl(), bool())
        {
            return static_cast<Derived *>(this)->IsFullImpl();
        }

        template <typename T>
        bool IsFullHelper(long)
        {
            return false;
        }

        // GetSize
        template <typename T>
        auto GetSizeHelper(int)
            -> decltype(std::declval<T>().GetSizeImpl(), size_t())
        {
            return static_cast<Derived *>(this)->GetSizeImpl();
        }

        template <typename T>
        size_t GetSizeHelper(long)
        {
            return 0;
        }

    public:
        QueueAbility() = default;
        ~QueueAbility() = default;

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

        bool Send(const std::vector<DataType> &data, int timeout)
        {
            return SendHelper<Derived>(0, data, timeout);
        }

        bool SendToFront(const std::vector<DataType> &data, int timeout)
        {
            return SendToFrontHelper<Derived>(0, data, timeout);
        }

        bool SendToBack(const std::vector<DataType> &data, int timeout)
        {
            return SendToBackHelper<Derived>(0, data, timeout);
        }

        bool Receive(std::vector<DataType> &data, int timeout)
        {
            return ReceiveHelper<Derived>(0, data, timeout);
        }

        bool Peek(std::vector<DataType> &data, int timeout)
        {
            return PeekHelper<Derived>(0, data, timeout);
        }

        bool Reset()
        {
            return ResetHelper<Derived>(0);
        }

        bool IsEmpty()
        {
            return IsEmptyHelper<Derived>(0);
        }

        bool IsFull()
        {
            return IsFullHelper<Derived>(0);
        }

        size_t GetSize()
        {
            return GetSizeHelper<Derived>(0);
        }
    };
} // namespace interface
