#ifndef ELFSPY_HOOK_H
#define ELFSPY_HOOK_H

#include "elfspy/HookImpl.h"
#include "elfspy/GOTEntry.h"
#include "elfspy/Function.h"
#include "elfspy/MethodInfo.h"
#include "elfspy/Thunk.h"

namespace spy
{

/**
 * @namespace spy
 * @class Hook
 * An instance is a function that is executed after a call to a
 * function/method and before it is run, essentially injected between the two.
 * It is very lightweight as it only holds reference(s) to static data members.
 * Calling convention is unified between methods and functions so that methods
 * are functions where the first type in Args... is the class type.
 */

template <typename CRTP, typename ReturnType, typename... ArgTypes>
class Hook : public HookImpl<CRTP, ReturnType, ArgTypes...>
{
public:
  using Result = ReturnType;
  /**
   * typedefs to re-export variadic ArgTypes to other variadic templates
   */
  template <template <typename...> class Other, typename... Extra>
  struct Export
  {
    using Type = Other<Extra..., ArgTypes...>;
  };
  template <template <size_t, typename...> class Other>
  struct ExportN
  {
    template <size_t N>
    using Type = Other<N, ArgTypes...>;
  };
  /**
   * construct from function pointer
   */
  Hook(const char* name, ReturnType (*function)(ArgTypes...));
  /**
   * construct from method pointer
   */
  Hook(const char* name, MethodInfo<ReturnType, ArgTypes...> method);
  /// restore original function
  ~Hook();
  /**
   * invoke the original function
   * @param args arguments to pass
   * @return return value of function
   */
  static ReturnType invoke_real(ArgTypes... args);

  operator const GOTEntry&() const;

private:
  static GOTEntry got_entry_;
  static const char* name_;
  using Base = HookImpl<CRTP, ReturnType, ArgTypes...>;
};

template <typename CRTP, typename ReturnType, typename... ArgTypes>
GOTEntry Hook<CRTP, ReturnType, ArgTypes...>::got_entry_;
template <typename CRTP, typename ReturnType, typename... ArgTypes>
const char* Hook<CRTP, ReturnType, ArgTypes...>::name_ = "no name";

template <typename CRTP, typename ReturnType, typename... ArgTypes>
Hook<CRTP, ReturnType, ArgTypes...>::Hook(const char* name, 
                                          ReturnType (*function)(ArgTypes...))
{
  name_ = name;
  // use union to get and convert address of function
  Function<ReturnType, ArgTypes...> converter;
  converter.address_ = function;
  got_entry_.set(converter.pointer_, name);
  converter.address_ = &Base::thunk;
  converter.pointer_ = got_entry_.spy_with(converter.pointer_);
  Base::patch_ = Base::real_ = converter.address_;
}

template <typename CRTP, typename ReturnType, typename... ArgTypes>
inline Hook<CRTP, ReturnType, ArgTypes...>::
Hook(const char* name, MethodInfo<ReturnType, ArgTypes...> method)
{
  name_ = name;
  got_entry_.set(method.address_, name);
  if (method.vtable_entry_) {
    got_entry_.make_entry(method.vtable_entry_);
  }
  Function<ReturnType, ArgTypes...> converter;
  converter.address_ = &Base::thunk;
  converter.pointer_ = got_entry_.spy_with(converter.pointer_);
  Base::patch_ = Base::real_ = converter.address_;
}

template <typename CRTP, typename ReturnType, typename... ArgTypes>
inline Hook<CRTP, ReturnType, ArgTypes...>::~Hook()
{
  got_entry_.restore();
}

template <typename CRTP, typename ReturnType, typename... ArgTypes>
inline ReturnType Hook<CRTP, ReturnType, ArgTypes...>::invoke_real(ArgTypes... args)
{
  return (*Base::real_)(args...);
}

template <typename CRTP, typename ReturnType, typename... ArgTypes>
inline Hook<CRTP, ReturnType, ArgTypes...>::operator const GOTEntry&() const
{
  return got_entry_;
}

} // namespace spy

#endif
