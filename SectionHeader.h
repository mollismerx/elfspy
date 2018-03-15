#ifndef ELFSPY_SECTIONHEADER_H
#define ELFSPY_SECTIONHEADER_H

#include <elf.h>
#include "elfspy/Section.h"

namespace spy
{
class ELFInfo;

class SectionHeader
{
public:
  SectionHeader() = default;
  const unsigned char* begin() const;
  const unsigned char* end() const;
  size_t entries() const;
  bool operator!() const;
  explicit operator bool() const;
  template <typename T>
  Section<T> as_section() const;
private:
  static const Elf_Shdr no_header_;
  const unsigned char* data_ = nullptr;
  const Elf_Shdr* header_ = &no_header_;
  friend class ELFInfo;
  SectionHeader(const unsigned char* data, const Elf_Shdr* header);
};

inline SectionHeader::SectionHeader(const unsigned char* data, const Elf_Shdr* header)
  :data_(data)
  ,header_(header)
{
}

inline const unsigned char* SectionHeader::begin() const
{
  return data_ + header_->sh_offset;
}

inline const unsigned char* SectionHeader::end() const
{
  return begin() + header_->sh_size;
}


inline size_t SectionHeader::entries() const
{
  return header_->sh_size / header_->sh_entsize;
}

inline bool SectionHeader::operator!() const
{
  return header_ == nullptr;
}

inline SectionHeader::operator bool() const
{
  return header_ != nullptr;
}

template <typename T>
inline Section<T> SectionHeader::as_section() const
{
  return { data_, header_ };
}

} // namespace spy

#endif
