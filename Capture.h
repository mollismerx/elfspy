#ifndef ELFSPY_CAPTURE_H
#define ELFSPY_CAPTURE_H

#include <vector>

namespace spy
{

/*
 * @namespace spy
 * @class Capture
 * Base class for holding copies of 1 or nore values encountered during
 * invocations. Provides methods for comparison in asserts after invocation.
 */
 
template <typename T>
class Capture
{
public:
  using Container = std::vector<T>;
  using const_iterator = typename Container::const_iterator;
  const_iterator begin() const;
  const_iterator end() const;
  std::size_t size() const;
  const T& value(std::size_t index = 0) const;

protected:
  Container captures_;
};

template <typename T>
inline typename Capture<T>::const_iterator Capture<T>::begin() const
{
  return captures_.begin();
}

template <typename T>
inline typename Capture<T>::const_iterator Capture<T>::end() const
{
  return captures_.end();
}

template <typename T>
//inline typename Capture<T>::size_type Capture<T>::size() const
inline std::size_t Capture<T>::size() const
{
  return captures_.size();
}

template <typename T>
const T& Capture<T>::value(std::size_t index) const
{
  return captures_[index];
}

} // namespace spy

#endif

