#ifndef ELFSPY_THUNK_H
#define ELFSPY_THUNK_H

namespace spy
{

/**
 * @namespace spy
 * @class Thunk
 * Base class for Thunks that can be created by Hook
 * No return value type with a Thunk to avoid interfering with the value
 * signature is given by ArgTypes... and is either empty or the same as the
 * Thunk's function
 */

template <typename... ArgTypes>
class Thunk
{
public:
  typedef Thunk<ArgTypes...> Base;
  virtual ~Thunk() = default;

  virtual void invoke(ArgTypes&&... args) = 0;
};

} // namespace spy

#endif
