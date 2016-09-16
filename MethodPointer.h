#ifndef ELFSPY_METHODPOINTER_H
#define ELFSPY_METHODPOINTER_H

#include <cstddef>

namespace spy
{

/**
 * @namespace spy
 * @struct MethodPointer
 * handle details of a potentially virtual function pointer to a class method
 */

struct MethodPointer
{
  union
  {
    std::size_t index_;
    void* pointer_;
  };
  std::size_t offset_; // to adjust this pointer with

  bool is_virtual() const;
};

inline bool MethodPointer::is_virtual() const
{
  return index_ & 1UL;
}

} // namespace spy

#endif
