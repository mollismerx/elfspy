#ifndef ELFSPY_MFILE_H
#define ELFSPY_MFILE_H

#include <stddef.h>

namespace spy
{
class Symbol;

/**
 * @namespace spy
 * @class MFile
 * maps a file to memory
 */

class MFile
{
public:
  MFile(const char* file_name);
  MFile(const MFile&) = delete;
  MFile& operator=(const MFile&) = delete;
  ~MFile();

  const char* name() const;
  unsigned char* address() const;
  size_t size() const;

private:
  const char* name_;
  int fd_;
  size_t size_;
  unsigned char* address_;
};

inline const char* MFile::name() const
{
  return name_;
}

inline unsigned char* MFile::address() const
{
  return address_;
}

inline size_t MFile::size() const
{
  return size_;
}

} // namespace spy

#endif
