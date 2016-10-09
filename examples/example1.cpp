#include <cassert>
#include <iostream>

#include "elfspy/SPY.h"
#include "elfspy/Call.h"
#include "elfspy/Arg.h"
#include "elfspy/Result.h"

#include "elfspy/demo.h"

int main(int argc, char** argv)
{
  spy::initialise(argc, argv);
  {
    // add some spies about things that happen in f()
    auto add_spy = SPY(&add);
    auto add_arg0 = spy::arg<0>(add_spy); // capture first argument of add()
    auto add_arg1 = spy::arg<1>(add_spy); // capture second argument of add()
    auto add_call = spy::call(add_spy);   // capture number of calls to add()
    auto add_result = spy::result(add_spy); // capture return value from add()
    int rv = f();
    assert(rv == 10);
    assert(add_call.count() == 2);  // verify add is called twice
    assert(add_arg0.value(0) == 1); // verify first argument of first call
    assert(add_arg1.value(0) == 2); // verify second argument of first call
    assert(add_arg0.value(1) == 3); // verify first argument of second call
    assert(add_arg1.value(1) == 4); // verify second argument of second call
    assert(add_result.value(0) == 3); // verify return value of first call
    assert(add_result.value(1) == 7); // verify return value of second call
  }
  return 0;
}
