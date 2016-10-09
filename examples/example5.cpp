#include <cassert>
#include <iostream>

#include "elfspy/SPY.h"
#include "elfspy/Profiler.h"

#include "demo.h"

int main(int argc, char** argv)
{
  spy::initialise(argc, argv);
  auto add_spy = SPY(&add);
  auto add_profiler = spy::profiler(add_spy);
  int rv = f();
  int run = 0;
  for (auto duration : add_profiler) {
    std::cout << ++run << ": " << duration << " nanoseconds" << std::endl;
  }
  return 0;
}
