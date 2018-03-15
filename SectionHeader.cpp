#include "elfspy/SectionHeader.h"

#include <string.h>

namespace
{
spy::Elf_Shdr no_header()
{
  spy::Elf_Shdr zero_init;
  memset(&zero_init, 0, sizeof(zero_init));
  return zero_init;
}

} // namespace

namespace spy
{

// allow begin() and end() to work with SectionHeaders that were not found
const Elf_Shdr SectionHeader::no_header_ = no_header();

} // namespace spy
