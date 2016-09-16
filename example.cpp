#include <cassert>
#include <cstring>
#include <iostream>
#include <time.h>
#include <unistd.h>

#include "elfspy/SPY.h"
#include "elfspy/Call.h"
#include "elfspy/Arg.h"
#include "elfspy/Result.h"
#include "elfspy/Fake.h"
#include "elfspy/Profiler.h"
//#include "elfspy/StackTrace.h"
//#include "elfspy/StackFrame.h"

#include "elfspy/demo.h"

int main(int argc, char** argv)
{
  // initialise
  spy::initialise(argc, argv);
  {
    // add some spies about things that happen in f()
    // auto f_call = spy::call(SPY(&f)); // nicer
    // auto f_call = spy::call(SPY(&f));
    auto add_spy = SPY(&add);
    auto add_arg0 = spy::arg<0>(add_spy);
    auto add_arg1 = spy::arg<1>(add_spy);
    auto add_call = spy::call(add_spy);
    auto add_result = spy::result(add_spy);
    //auto add_stack = spy::stack_trace(add_spy);
    int rv = f();
    assert(rv == 10);
    // assert(f_call->count() == 1);
    assert(add_call.count() == 2);
    assert(add_arg0.value(0) == 1);
    assert(add_arg1.value(0) == 2);
    assert(add_arg0.value(1) == 3);
    assert(add_arg1.value(1) == 4);
    assert(add_result.value(0) == 3);
    assert(add_result.value(1) == 7);
  }
  {
    auto add_spy = SPY(&add);
    auto add_patch = spy::fake(add_spy, &sub);
    int rv = f();
    assert(rv == -2);
  }
  {
    auto add_spy = SPY(&add);
    auto lambda = [](int a, int b) { return a - b; };
    auto add_patch = spy::fake(add_spy, lambda);
    int rv = f();
    assert(rv == -2);
  }
  int rv = f();
  assert(rv == 10);
  {
    time_t time_diff = 0;
    auto time_spy = SPY(&time);
    auto time_changer = [&time_diff, &time_spy](time_t* tloc) -> time_t {
      time_t fake_time = time_spy.invoke_real(tloc) + time_diff;
      if (tloc) {
        *tloc = fake_time;
      }
      return fake_time;
    };
    auto time_fake = spy::fake(time_spy, time_changer);
    // relive Y2K
    struct tm false_time;
    memset(&false_time, 0, sizeof(false_time));
    false_time.tm_sec = 58;
    false_time.tm_min = 59;
    false_time.tm_hour = 23;
    false_time.tm_mday = 31;
    false_time.tm_mon = 11;
    false_time.tm_year = 1999 - 1900;
    false_time.tm_isdst = -1;
    time_t in_the_past = mktime(&false_time);
    time_t now = time(nullptr);
    time_diff = in_the_past - now; // set the time_changer difference
    time_t reported_time = time(nullptr);
    std::cout << ctime(&reported_time) << std::endl;
    sleep(4);
    reported_time = time(nullptr);
    std::cout << ctime(&reported_time) << std::endl;
  }
#if 0
  assert(add_stack->value().contains(SPY(&f)));
  for (spy::StackFrame frame : add_stack->value()) {
    std::cout << frame.name() << std::endl;
  }
#endif
  {
    auto add_spy = SPY(&add);
    auto add_profiler = spy::profiler(add_spy);
    int rv = f();
    int run = 0;
    for (auto duration : add_profiler) {
      std::cout << ++run << ": " << duration << " nanoseconds" << std::endl;
    }
  }
  std::cout << "test passed" << std::endl;
  return 0;
}
