#ifndef ELFSPY_CALL_H
#define ELFSPY_CALL_H

#include "elfspy/Thunk.h"
#include "elfspy/ThunkHandle.h"

namespace spy
{

/**
 * @namespace spy
 * @class Call
 * Will count each call made to the Thunk's function
 */

class Call : public Thunk<>
{
public:
  virtual void invoke() override;
  
  std::size_t count() const;
  
private:
  std::size_t count_ = 0;
};

inline std::size_t Call::count() const
{
  return count_;
}

template <typename H>
inline ThunkHandle<Call> call(H&& hook)
{
  return hook.no_arg_thunks();
}

template <typename H>
inline ThunkHandle<Call>* new_call(H&& hook)
{
  return new ThunkHandle<Call>(hook.no_arg_thunks());
}

} // namespace spy

#endif
