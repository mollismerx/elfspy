#ifndef ELFSPY_FAKE_H
#define ELFSPY_FAKE_H

#include "elfspy/Lambda.h"

namespace spy
{

/**
 * @namespace spy
 * @class Fake
 * This replaces an existing function with another function, so that all calls
 * to the program are made to the other function.
 * The constructor installs the new function in place of the function, the
 * destructor uninstalls it
 */

template <typename H, typename ReturnType, typename... ArgTypes>
class Fake
{
public:
  Fake(H& hook, ReturnType (*func)(ArgTypes...));
  ~Fake();

private:
  H& hook_;
  ReturnType (*func_)(ArgTypes...);
};

template <typename H, typename ReturnType, typename... ArgTypes>
inline Fake<H, ReturnType, ArgTypes...>::
Fake(H& hook, ReturnType (*func)(ArgTypes...))
  :hook_(hook)
{
  func_ = hook_.patch(func);
}

template <typename H, typename ReturnType, typename... ArgTypes>
inline Fake<H, ReturnType, ArgTypes...>::~Fake()
{
  hook_.patch(func_);
}

template <typename H, typename ReturnType, typename... ArgTypes>
inline auto fake(H& hook, ReturnType (*patch)(ArgTypes...))
  -> Fake<H, ReturnType, ArgTypes...>
{
  return { hook, patch };
}

template <typename H, typename ReturnType, typename... ArgTypes>
inline auto new_fake(H& hook, ReturnType (*patch)(ArgTypes...))
  -> Fake<H, ReturnType, ArgTypes...>*
{
  return new Fake<H, ReturnType, ArgTypes...>(hook, patch);
}

} // namespace elfspy

#endif
