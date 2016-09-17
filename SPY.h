#ifndef ELFSPY_SPY_H
#define ELFSPY_SPY_H

#include "elfspy/Hook.h"
#include "elfspy/Method.h"

namespace spy
{
void initialise(int argc, char** argv);

namespace impl
{

/**
 * @namespace spy
 * @namespace impl
 * class Function
 * A Hook for functions. Uses curiously recurring template pattern to create a
 * unique type for each function pointer it is instantiated with
 */

template <typename R, typename... Args>
struct Function
{
  // use CRTP as Hook uses static members
  template <R (*FUNC)(Args...)>
  struct Instance : public Hook<Instance<FUNC>, R, Args...>
  {
    using Base = Hook<Instance<FUNC>, R, Args...>;
    Instance(const char* name)
      :Base(name, FUNC)
    {
    }
  };
};

/**
 * @namespace spy
 * @namespace impl
 * class MutableMethod
 * A Hook for non const methods. Uses curiously recurring template pattern to
 * create a unique type for each method pointer it is instantiated with
 */

template <typename R, typename T, typename... Args>
struct MutableMethod
{
  template <R (T::*METHOD)(Args...)>
  struct Instance : public Hook<Instance<METHOD>, R, T*, Args...>
  {
    using Base = Hook<Instance<METHOD>, R, T*, Args...>;
    Instance(const char* name)
      :Base(name, Method<R, T, Args...>(METHOD).resolve())
    {
    }
  };
};

/**
 * @namespace spy
 * @namespace impl
 * class ConstMethod
 * A Hook for const methods. Uses curiously recurring template pattern to
 * create a unique type for each method pointer it is instantiated with
 */

template <typename R, typename T, typename... Args>
struct ConstMethod
{
  template <R (T::*METHOD)(Args...) const>
  struct Instance : public Hook<Instance<METHOD>, R, T*, Args...>
  {
    using Base = Hook<Instance<METHOD>, R, T*, Args...>;
    Instance(const char* name)
      :Base(name, Method<R, T, Args...>(METHOD).resolve())
    {
    }
  };
};

/**
 * @namespace spy
 * @namespace impl
 * class VolatileMethod
 * A Hook for volatile methods. Uses curiously recurring template pattern to
 * create a unique type for each method pointer it is instantiated with
 */

template <typename R, typename T, typename... Args>
struct VolatileMethod
{
  template <R (T::*METHOD)(Args...) volatile>
  struct Instance : public Hook<Instance<METHOD>, R, T*, Args...>
  {
    using Base = Hook<Instance<METHOD>, R, T*, Args...>;
    Instance(const char* name)
      :Base(name, Method<R, T, Args...>(METHOD).resolve())
    {
    }
  };
};

/**
 * @namespace spy
 * @namespace impl
 * class ConstVolatileMethod
 * A Hook for volatile methods. Uses curiously recurring template pattern to
 * create a unique type for each method pointer it is instantiated with
 */

template <typename R, typename T, typename... Args>
struct ConstVolatileMethod
{
  template <R (T::*METHOD)(Args...) const volatile>
  struct Instance : public Hook<Instance<METHOD>, R, T*, Args...>
  {
    using Base = Hook<Instance<METHOD>, R, T*, Args...>;
    Instance(const char* name)
      :Base(name, Method<R, T, Args...>(METHOD).resolve())
    {
    }
  };
};

/**
 * @brief
 * overloaded function only to get the right decltype depending on the
 * argument passed for the SPY macro. Hence no definition is provided.
 */
template <typename R, typename... Args>
auto create(R (*)(Args...)) -> Function<R, Args...>;
template <typename R, typename T, typename... Args>
auto create(R (T::*)(Args...)) -> MutableMethod<R, T, Args...>;
template <typename R, typename T, typename... Args>
auto create(R (T::*)(Args...) const) -> ConstMethod<R, T, Args...>;
template <typename R, typename T, typename... Args>
auto create(R (T::*)(Args...) volatile) -> VolatileMethod<R, T, Args...>;
template <typename R, typename T, typename... Args>
auto create(R (T::*)(Args...) const volatile)
    -> ConstVolatileMethod<R, T, Args...>;

} // namespace impl

} // namespace spy

/**
 * @def SPY(F)
 * Macro SPY captures signature of F whether F is a function or a method, and
 * injects a Hook for F into the Global Offset Table which is invoked between a
 * program's calls to F and the invocations of F.
 * SPY returns a (derived) instance of Hook that can be used to add Hooks to.
 */

#define SPY(F) decltype(spy::impl::create(F))::Instance<F>(#F)

#endif
