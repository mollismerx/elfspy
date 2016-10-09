#include <cassert>
#include <iostream>

#include "elfspy/SPY.h"
#include "elfspy/Fake.h"
#include "demo.h"

int main(int argc, char** argv)
{
  spy::initialise(argc, argv);
  auto add_spy = SPY(&add);
  {
    auto add_fake = spy::fake(add_spy, &sub);
    int rv = f();
    assert(rv == -2);
  }
  int rv = f();
  assert(rv == 10);
  {
    auto multiply = [](int a, int b) { return a * b; };
    auto add_fake = spy::fake(add_spy, multiply);
    int rv = f();
    assert(rv == 14);
  }
  return 0;
}
