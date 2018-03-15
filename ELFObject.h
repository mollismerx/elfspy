#ifndef ELFSPY_ELFOBJECT_HXX
#define ELFSPY_ELFOBJECT_HXX

#include <string>

namespace spy
{

struct ELFObject
{
  const char* name_ = "no name";
  unsigned char* base_ = nullptr;
  size_t size_ = 0;
};

} // namespace spy

#endif
