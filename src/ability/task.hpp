#pragma once

#include <type_traits>
#include <utility>

namespace interface
{
    template <typename Derived, typename DerivedConfig>
    class TaskAbility
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

        // Suspend
        template <typename T>
        auto SuspendHelper(int)
            -> decltype(std::declval<T>().SuspendImpl(), bool())
        {
            return static_cast<Derived *>(this)->SuspendImpl();
        }

        template <typename T>
        bool SuspendHelper(long)
        {
            return false;
        }

        // Resume
        template <typename T>
        auto ResumeHelper(int)
            -> decltype(std::declval<T>().ResumeImpl(), bool())
        {
            return static_cast<Derived *>(this)->ResumeImpl();
        }

        template <typename T>
        bool ResumeHelper(long)
        {
            return false;
        }

        // IsSuspended
        template <typename T>
        auto IsSuspendedHelper(int)
            -> decltype(std::declval<T>().IsSuspendedImpl(), bool())
        {
            return static_cast<Derived *>(this)->IsSuspendedImpl();
        }

        template <typename T>
        bool IsSuspendedHelper(long)
        {
            return false;
        }

    public:
        TaskAbility() = default;
        ~TaskAbility() = default;

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

        bool Suspend()
        {
            return SuspendHelper<Derived>(0);
        }

        bool Resume()
        {
            return ResumeHelper<Derived>(0);
        }

        bool IsSuspended()
        {
            return IsSuspendedHelper<Derived>(0);
        }
    };
} // namespace interface
