#ifndef ELFSPY_SECTION_H
#define ELFSPY_SECTION_H

#include <elf.h>
#include <stddef.h>

namespace spy
{
#ifdef __x86_64__
using Elf_Shdr = Elf64_Shdr;
#else
using Elf_Shdr = Elf32_Shdr;
#endif
class SectionHeader;

template <typename T>
class Section
{
public:
  class Iterator;
  Iterator begin() const;
  Iterator end() const;
  size_t entries() const;
  const T& operator[](size_t index) const;

private:
  const unsigned char* data_;
  const Elf_Shdr* header_;
  friend class SectionHeader;
  Section(const unsigned char* data, const Elf_Shdr* header);
};

template <typename T>
inline Section<T>::Section(const unsigned char* data, const Elf_Shdr* header)
  :data_(data)
  ,header_(header)
{
}

template <typename T>
class Section<T>::Iterator
{
public:
  Iterator& operator++();
  Iterator operator++(int);
  const T& operator*() const;
  const T* operator->() const;
  bool operator!=(const Iterator& other) const;

private:
  Iterator(const unsigned char* data, size_t size);
  const unsigned char* data_;
  size_t size_;
  friend class Section;
};

template <typename T>
inline Section<T>::Iterator::Iterator(const unsigned char* data, size_t size)
{
  data_ = data;
  size_ = size;
}

template <typename T>
inline typename Section<T>::Iterator& Section<T>::Iterator::operator++()
{
  data_ += size_;
  return *this;
}

template <typename T>
inline typename Section<T>::Iterator Section<T>::Iterator::operator++(int)
{
  Iterator copy(*this);
  data_ += size_;
  return copy;
}

template <typename T>
inline const T& Section<T>::Iterator::operator*() const
{
  return *reinterpret_cast<const T*>(data_);
}

template <typename T>
inline const T* Section<T>::Iterator::operator->() const
{
  return reinterpret_cast<const T*>(data_);
}

template <typename T>
inline bool Section<T>::Iterator::operator!=(const Section<T>::Iterator& other) const
{
  return data_ != other.data_;
}

template <typename T>
inline typename Section<T>::Iterator Section<T>::begin() const
{
  return { data_ + header_->sh_offset, header_->sh_entsize };
}

template <typename T>
inline typename Section<T>::Iterator Section<T>::end() const
{
  return { data_ + header_->sh_offset + header_->sh_size, header_->sh_entsize };
}

template <typename T>
inline size_t Section<T>::entries() const
{
  return header_->sh_size / header_->sh_entsize;
}

template <typename T>
const T& Section<T>::operator[](size_t index) const
{
  index *= header_->sh_entsize;
  return *reinterpret_cast<const T*>(data_ + header_->sh_offset + index);
}

} // namespace spy

#endif
