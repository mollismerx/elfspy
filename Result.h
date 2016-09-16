#ifndef ELFSPY_RESULT_H
#define ELFSPY_RESULT_H

#include "elfspy/Variadic.h"
#include "elfspy/Thunk.h"
#include "elfspy/ThunkHandle.h"
#include "elfspy/Capture.h"

namespace spy
{

/**
 * @namespace spy
 * @class Result
 * This captures the return value of an invocation
 */

template <typename ReturnType>
class Result : public Thunk<ReturnType&&>,
               public Capture<ReturnType>
{
public:
  virtual void invoke(ReturnType&& result) override;
};
  
template <typename ReturnType>
void Result<ReturnType>::invoke(ReturnType&& result)
{
  // forward arguments to avoid copying
  this->captures_.push_back(std::forward<ReturnType>(result));
}

template <typename H>
inline auto result(H& hook) -> ThunkHandle<Result<typename H::Result>>
{
  return hook.return_thunks();
}

template <typename H>
inline auto new_result(H& hook) -> ThunkHandle<Result<typename H::Result>>*
{
  return new ThunkHandle<typename H::Result>(hook.return_thunks());
}

} // namespace elfspy

#endif
