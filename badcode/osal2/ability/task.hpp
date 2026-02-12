#pragma once

#include <functional>
#include <string>
#include <cstdint>
#include <utility>
#include <type_traits>

namespace osal
{
    template <typename Derived>
    class TaskAbility
    {
        friend Derived;

    protected:
        /// Dispatch: invoke SFINAE-constrained callable on Derived*, no-op if not viable
        template <typename Fn, typename... Args>
        void Dispatch(Fn&& fn, Args&&... args)
        {
            if constexpr (std::is_invocable_v<Fn, Derived*, Args...>)
                std::forward<Fn>(fn)(static_cast<Derived*>(this), std::forward<Args>(args)...);
        }

        /// Query: invoke SFINAE-constrained callable on const Derived*, return fallback if not viable
        template <typename Ret, typename Fn>
        Ret Query(Ret fallback, Fn&& fn) const
        {
            if constexpr (std::is_invocable_v<Fn, const Derived*>)
                return std::forward<Fn>(fn)(static_cast<const Derived*>(this));
            else
                return fallback;
        }

    public:
        TaskAbility() = default;
        ~TaskAbility() = default;

        // --- Properties ---

        void SetName(const std::string& name)
        {
            Dispatch([](auto* s, const auto& v) -> decltype((void)s->SetNameImpl(v)) {
                s->SetNameImpl(v);
            }, name);
        }

        std::string GetName() const
        {
            return Query(std::string(), [](const auto* s) -> decltype(s->GetNameImpl()) {
                return s->GetNameImpl();
            });
        }

        void SetEntry(std::function<void(void*)> entry)
        {
            Dispatch([](auto* s, std::function<void(void*)> e) -> decltype((void)s->SetEntryImpl(e)) {
                s->SetEntryImpl(e);
            }, std::move(entry));
        }

        std::function<void(void*)> GetEntry() const
        {
            return Query(std::function<void(void*)>(nullptr),
                [](const auto* s) -> decltype(s->GetEntryImpl()) {
                    return s->GetEntryImpl();
                });
        }

        void SetStackSize(uint32_t bytes)
        {
            Dispatch([](auto* s, uint32_t v) -> decltype((void)s->SetStackSizeImpl(v)) {
                s->SetStackSizeImpl(v);
            }, bytes);
        }

        uint32_t GetStackSize() const
        {
            return Query(uint32_t(0), [](const auto* s) -> decltype(s->GetStackSizeImpl()) {
                return s->GetStackSizeImpl();
            });
        }

        void SetPriority(int priority)
        {
            Dispatch([](auto* s, int v) -> decltype((void)s->SetPriorityImpl(v)) {
                s->SetPriorityImpl(v);
            }, priority);
        }

        int GetPriority() const
        {
            return Query(int(-1), [](const auto* s) -> decltype(s->GetPriorityImpl()) {
                return s->GetPriorityImpl();
            });
        }

        // --- Lifecycle ---

        bool Create()
        {
            auto fn = [](auto* s) -> decltype(s->CreateImpl()) { return s->CreateImpl(); };
            if constexpr (std::is_invocable_v<decltype(fn), Derived*>)
                return fn(static_cast<Derived*>(this));
            else
                return false;
        }

        void Delete()
        {
            Dispatch([](auto* s) -> decltype((void)s->DeleteImpl()) { s->DeleteImpl(); });
        }

        void Suspend()
        {
            Dispatch([](auto* s) -> decltype((void)s->SuspendImpl()) { s->SuspendImpl(); });
        }

        void Resume()
        {
            Dispatch([](auto* s) -> decltype((void)s->ResumeImpl()) { s->ResumeImpl(); });
        }
    };

} // namespace osal
