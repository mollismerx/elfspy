#include "elfspy/MFile.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>

#include "elfspy/Fail.h"

namespace spy
{

MFile::MFile(const char* file_name)
{
  fd_ = open(file_name, O_RDONLY, 0);
  if (fd_ < 0) {
    Fail() << "Cannot open " << file_name;
  }
  struct stat file;
  if (fstat(fd_, &file) < 0) {
    Fail() << "Cannot fstat " << file_name;
  }
  size_ = file.st_size;
  address_ = reinterpret_cast<unsigned char*>(
      mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_, 0));
  if (address_ == MAP_FAILED) {
    Fail() << "Cannot mmap " << file_name;
  }
}

MFile::~MFile()
{
  munmap(address_, size_);
  close(fd_);
}

} // namespace spy
