#ifndef ELFSPY_HOOKIMPL_H
#define ELFSPY_HOOKIMPL_H

#include "elfspy/HookBase.h"

namespace spy
{

/**
 * @namespace spy
 * @class HookImpl
 * Part of the Hook implementation to distinguish between void and non-void
 * return values.
 */

template <typename CRTP, typename ReturnType, typename... ArgTypes>
class HookImpl : public HookBase<CRTP, ReturnType, ArgTypes...>
{
public:
  /// @return collection to register thunks that process return value
  static std::vector<Thunk<ReturnType&&>*>& return_thunks();

protected:
  static std::vector<Thunk<ReturnType&&>*> return_thunks_;
  static ReturnType&& use_return(ReturnType&& result);
  static ReturnType thunk(ArgTypes... args);
  using Base = HookBase<CRTP, ReturnType, ArgTypes...>;
};

template <typename CRTP, typename ReturnType, typename... ArgTypes>
std::vector<Thunk<ReturnType&&>*>
HookImpl<CRTP, ReturnType, ArgTypes...>::return_thunks_;

template <typename CRTP, typename ReturnType, typename... ArgTypes>
inline std::vector<Thunk<ReturnType&&>*>&
HookImpl<CRTP, ReturnType, ArgTypes...>::return_thunks()
{
  return return_thunks_;
}

template <typename CRTP, typename ReturnType, typename... ArgTypes>
ReturnType&&
HookImpl<CRTP, ReturnType, ArgTypes...>::use_return(ReturnType&& result)
{
  for (auto trap : return_thunks_)
  {
    trap->invoke(std::forward<ReturnType>(result));
  }
  return std::forward<ReturnType>(result);
}

template <typename CRTP, typename ReturnType, typename... ArgTypes>
ReturnType HookImpl<CRTP, ReturnType, ArgTypes...>::thunk(ArgTypes... args)
{
  Base::run_thunks(std::forward<ArgTypes>(args)...);
  return use_return(
    std::forward<ReturnType>((*Base::patch_)(std::forward<ArgTypes>(args)...)));
}

/// specialisation for void return
template <typename CRTP, typename... ArgTypes>
class HookImpl<CRTP, void, ArgTypes...>
  : public HookBase<CRTP, void, ArgTypes...>
{
protected:
  static void thunk(ArgTypes... args);

private:
  using Base = HookBase<CRTP, void, ArgTypes...>;
};

template <typename CRTP, typename... ArgTypes>
void HookImpl<CRTP, void, ArgTypes...>::thunk(ArgTypes... args)
{
  Base::run_thunks(std::forward<ArgTypes>(args)...);
  (*Base::patch_)(std::forward<ArgTypes>(args)...);
}


} // namespace spy

#endif
