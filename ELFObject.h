#ifndef ELFSPY_ELFOBJECT_HXX
#define ELFSPY_ELFOBJECT_HXX

namespace spy
{

struct ELFObject
{
  const char* name_ = "no name";
  unsigned char* base_ = nullptr;
  size_t size_ = 0;
  size_t symbols_ = 0;
  size_t strings_ = 0;
  size_t rela_plt_ = 0;
};

} // namespace spy

#endif
