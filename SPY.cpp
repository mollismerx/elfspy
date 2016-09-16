#include "elfspy/SPY.h"

#include "elfspy/GOTEntry.h"

namespace spy
{

void initialise(int argc, char** argv)
{
  GOTEntry::initialise(argc, argv);
}

} // namespace spy
