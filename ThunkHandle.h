#ifndef ELFSPY_THUNKHANDLE_H
#define ELFSPY_THUNKHANDLE_H

#include <vector>

namespace spy
{

/**
 * @namespace spy
 * @class ThunkHandle
 * This holds the return from a Thunk's creation function. The Thunk itself is
 * a base class. ThunkHandle registers/deregisters the Thunk to/from the Hook
 */

template <typename ThunkType>
class ThunkHandle : public ThunkType
{
public:
  /**
   * Register thunk member to hook and construct with optional arguments
   * @param thunks container to register/deregister Thunk from
   * @param construction arguments (if any) to pass to ThunkType constructor
   */
  template <typename... C>
  ThunkHandle(std::vector<typename ThunkType::Base*>& thunks,
              C&&... construction);
  ThunkHandle(const ThunkHandle&) = delete;
  ThunkHandle& operator=(const ThunkHandle&) = delete;
  ThunkHandle(ThunkHandle&& move);
  ThunkHandle& operator=(ThunkHandle&& move);
  /**
   * deregister from hook
   */
  ~ThunkHandle();

private:
  std::vector<typename ThunkType::Base*>& thunks_;
  void deregister();
};

template <typename ThunkType>
template <typename... C>
inline ThunkHandle<ThunkType>::
ThunkHandle(std::vector<typename ThunkType::Base*>& thunks,
                              C&&... construction)
  :ThunkType(std::forward<C>(construction)...)
  ,thunks_(thunks)
{
  // register the thunk
  thunks_.push_back(this);
}

template <typename ThunkType>
void ThunkHandle<ThunkType>::deregister()
{
  // find and deregister the thunk
  for (auto i = thunks_.begin(); i != thunks_.end(); ++i) {
    if (*i == this) {
      thunks_.erase(i);
      break;
    }
  }
}

template <typename ThunkType>
inline ThunkHandle<ThunkType>::ThunkHandle(ThunkHandle&& move)
  :ThunkType(move)
  ,thunks_(move.thunks_)
{
  move.deregister();
  thunks_.push_back(this);
}

template <typename ThunkType>
inline
ThunkHandle<ThunkType>& ThunkHandle<ThunkType>::operator=(ThunkHandle&& move)
{
  // thunks_ already points to the correct static member of the the ThunkType
  // as it has the function pointer in the template arguments
  move.deregister();
  thunks_.push_back(this);
  return *this;
}

template <typename ThunkType>
inline ThunkHandle<ThunkType>::~ThunkHandle()
{
  deregister();
}

} // namespace spy

#endif
