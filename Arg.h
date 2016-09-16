#ifndef ELFSPY_ARG_H
#define ELFSPY_ARG_H

#include "elfspy/Variadic.h"
#include "elfspy/Thunk.h"
#include "elfspy/ThunkHandle.h"
#include "elfspy/Capture.h"

namespace spy
{

/**
 * @namespace spy
 * @class Arg
 * This captures one argument of an invocation
 */

template <size_t N, typename... ArgTypes>
class Arg : public Thunk<ArgTypes...>,
            public Capture<typename Variadic<N, ArgTypes...>::Type>
{
public:
  virtual void invoke(ArgTypes&&... args) override;
};
  
template <size_t N, typename... ArgTypes>
void Arg<N, ArgTypes...>::invoke(ArgTypes&&... args)
{
  using Var = Variadic<N, ArgTypes...>;
  // forward arguments to avoid copying
  this->captures_.push_back(Var::get(std::forward<ArgTypes>(args)...));
}

template <size_t N, typename H>
inline auto arg(H& hook)
-> ThunkHandle<typename H::template ExportN<Arg>::template Type<N>>
{
  return hook.thunks();
}

template <size_t N, typename H>
inline auto new_arg(H& hook)
-> ThunkHandle<typename H::template ExportN<Arg>::template Type<N>>*
{
  using TP = ThunkHandle<typename H::template ExportN<Arg>::template Type<N>>;
  return new TP(hook.thunks());
}

} // namespace elfspy

#endif
