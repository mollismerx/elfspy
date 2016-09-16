#ifndef ELFSPY_METHOD_H
#define ELFSPY_METHOD_H

#include <typeinfo>
#include "elfspy/GOTEntry.h"
#include "elfspy/MethodPointer.h"
#include "elfspy/MethodInfo.h"

namespace spy
{

/**
 * @namespace spy
 * @union Method
 * Conversion union for all different method signatures
 */

template <typename R, typename T, typename... Args>
union Method
{
  R (T::*method_)(Args...);
  R (T::*const_method_)(Args...) const;
  R (T::*volatile_method_)(Args...) volatile;
  R (T::*cv_method_)(Args...) const volatile;
  Method(R (T::*method)(Args...));
  Method(R (T::*const_method)(Args...) const);
  Method(R (T::*volatile_method)(Args...) volatile);
  Method(R (T::*cv_method)(Args...) const volatile);
  R (*function_)(T*, Args...);
  void* address_;
  MethodPointer impl_;
  MethodInfo<R, T*, Args...> resolve();
};

template <typename R, typename T, typename... Args>
inline Method<R, T, Args...>::Method(R (T::*method)(Args...))
{
  method_ = method;
}

template <typename R, typename T, typename... Args>
inline Method<R, T, Args...>::Method(R (T::*const_method)(Args...) const)
{
  const_method_ = const_method;
}

template <typename R, typename T, typename... Args>
inline Method<R, T, Args...>::Method(R (T::*volatile_method)(Args...) volatile)
{
  volatile_method_ = volatile_method;
}

template <typename R, typename T, typename... Args>
inline Method<R, T, Args...>::Method(R (T::*cv_method)(Args...) const volatile)
{
  cv_method_ = cv_method;
}

template <typename R, typename T, typename... Args>
MethodInfo<R, T*, Args...> Method<R, T, Args...>::resolve()
{
  MethodInfo<R, T*, Args...> info;
  if (impl_.is_virtual()) {
    info.vtable_entry_ = GOTEntry::get_vtable_entry(typeid(T), impl_);
    info.address_ = *info.vtable_entry_;
  } else {
    info.vtable_entry_ = nullptr;
    info.address_ = address_;
  }
  return info;
}

} // namespace spy

#endif
