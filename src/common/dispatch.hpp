#pragma once

/// @file dispatch.hpp
/// @brief CRTP dispatch helpers using if-constexpr + lambda SFINAE.
///
/// Provides three dispatch patterns:
///   Dispatch  — optional void call (no-op if Derived lacks the method)
///   Query     — optional call with return value (returns fallback if missing)
///   Invoke    — mandatory call (static_assert fires if Derived lacks the method)
///
/// This is a shared utility used by both OSAL abilities and Logger.

#include <type_traits>
#include <utility>

namespace osal {

/// CRTP base providing Dispatch / Query / Invoke helpers.
/// Each Ability class should inherit this as `protected DispatchBase<Derived>`.
template <typename Derived>
class DispatchBase
{
protected:
  DispatchBase()  = default;
  ~DispatchBase() = default;

  // --- Dispatch: optional void call (silent no-op if not viable) ---

  template <typename Fn, typename... Args>
  void Dispatch(Fn&& fn, Args&&... args)
  {
    if constexpr (std::is_invocable_v<Fn, Derived*, Args...>)
      std::forward<Fn>(fn)(static_cast<Derived*>(this), std::forward<Args>(args)...);
  }

  template <typename Fn, typename... Args>
  void DispatchConst(Fn&& fn, Args&&... args) const
  {
    if constexpr (std::is_invocable_v<Fn, const Derived*, Args...>)
      std::forward<Fn>(fn)(static_cast<const Derived*>(this), std::forward<Args>(args)...);
  }

  // --- Query: optional call with return value + fallback ---

  template <typename Ret, typename Fn>
  Ret Query(Ret fallback, Fn&& fn) const
  {
    if constexpr (std::is_invocable_v<Fn, const Derived*>)
      return std::forward<Fn>(fn)(static_cast<const Derived*>(this));
    else
      return fallback;
  }

  template <typename Ret, typename Fn, typename... Args>
  Ret QueryMut(Ret fallback, Fn&& fn, Args&&... args)
  {
    if constexpr (std::is_invocable_v<Fn, Derived*, Args...>)
      return std::forward<Fn>(fn)(static_cast<Derived*>(this), std::forward<Args>(args)...);
    else
      return fallback;
  }

  // --- Invoke: mandatory call (compile error if Derived lacks the method) ---

  template <typename Fn, typename... Args>
  auto Invoke(Fn&& fn, Args&&... args)
  {
    static_assert(std::is_invocable_v<Fn, Derived*, Args...>,
      "Derived class must implement the required method");
    return std::forward<Fn>(fn)(static_cast<Derived*>(this), std::forward<Args>(args)...);
  }

  template <typename Fn, typename... Args>
  auto InvokeConst(Fn&& fn, Args&&... args) const
  {
    static_assert(std::is_invocable_v<Fn, const Derived*, Args...>,
      "Derived class must implement the required const method");
    return std::forward<Fn>(fn)(static_cast<const Derived*>(this), std::forward<Args>(args)...);
  }
};

} // namespace osal
