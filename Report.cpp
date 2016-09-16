#include "elfspy/Report.h"

#include <cerrno>
#include <cstring>
#include "elfspy/Error.h"

namespace spy
{

Report::Report()
{
  error_ = errno;
}

Report::~Report()
{
  show();
}

void Report::show()
{
  if (error_)
  {
    *this << ": " << std::strerror(error_);
  }
  error(str().c_str());
}

} // namespace spy
