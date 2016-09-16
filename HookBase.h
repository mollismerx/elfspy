#ifndef ELFSPY_HOOKBASE_H
#define ELFSPY_HOOKBASE_H

#include <vector>
#include "elfspy/Thunk.h"

namespace spy
{

/**
 * @namespace spy
 * @class HookBase
 * Contains the common part of HookImpl for void and non-void return values
 */

template <typename CRTP, typename ReturnType, typename... ArgTypes>
class HookBase
{
public:
  /// @return collection to register thunks that process function arguments
  static std::vector<Thunk<ArgTypes...>*>& thunks();
  /// @return collection to register thunks not using function arguments
  static std::vector<Thunk<>*>& no_arg_thunks();

  typedef ReturnType (*Signature)(ArgTypes...);
  /**
   * replace the original function with a different one
   * @param function to replace original with
   * @return previous function being called (typically the original)
   */
  static Signature patch(Signature function);

protected:
  static Signature real_;
  static Signature patch_;
  static std::vector<Thunk<ArgTypes...>*> thunks_;
  static std::vector<Thunk<>*> no_arg_thunks_;
  static void run_thunks(ArgTypes&&... args);
};

template <typename CRTP, typename ReturnType, typename... ArgTypes>
typename HookBase<CRTP, ReturnType, ArgTypes...>::Signature
HookBase<CRTP, ReturnType, ArgTypes...>::real_ = nullptr;

template <typename CRTP, typename ReturnType, typename... ArgTypes>
typename HookBase<CRTP, ReturnType, ArgTypes...>::Signature
HookBase<CRTP, ReturnType, ArgTypes...>::patch_ = nullptr;

template <typename CRTP, typename ReturnType, typename... ArgTypes>
std::vector<Thunk<ArgTypes...>*>
HookBase<CRTP, ReturnType, ArgTypes...>::thunks_;

template <typename CRTP, typename ReturnType, typename... ArgTypes>
std::vector<Thunk<>*> HookBase<CRTP, ReturnType, ArgTypes...>::no_arg_thunks_;

template <typename CRTP, typename ReturnType, typename... ArgTypes>
inline std::vector<Thunk<ArgTypes...>*>&
HookBase<CRTP, ReturnType, ArgTypes...>::thunks()
{
  return thunks_;
}

template <typename CRTP, typename ReturnType, typename... ArgTypes>
inline
std::vector<Thunk<>*>& HookBase<CRTP, ReturnType, ArgTypes...>::no_arg_thunks()
{
  return no_arg_thunks_;
}

template <typename CRTP, typename ReturnType, typename... ArgTypes>
void HookBase<CRTP, ReturnType, ArgTypes...>::run_thunks(ArgTypes&&... args)
{
  for (auto thunk : thunks_)
  {
    thunk->invoke(std::forward<ArgTypes>(args)...);
  }
  for (auto stub : no_arg_thunks_)
  {
    stub->invoke();
  }
}

template <typename CRTP, typename ReturnType, typename... ArgTypes>
typename HookBase<CRTP, ReturnType, ArgTypes...>::Signature
HookBase<CRTP, ReturnType, ArgTypes...>::
patch(typename HookBase<CRTP, ReturnType, ArgTypes...>::Signature function)
{
  auto copy = patch_;
  patch_ = function;
  return copy;
}

} // namespace spy

#endif
