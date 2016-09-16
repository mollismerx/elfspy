#include "elfspy/Fail.h"

#include <cstdlib>

namespace spy
{

Fail::~Fail()
{
  show();
  std::exit(-1);
}

} // namespace spy
