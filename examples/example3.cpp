#include "elfspy/SPY.h"
#include "elfspy/Fake.h"

#include <time.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

int main(int argc, char** argv)
{
  spy::initialise(argc, argv);
  time_t time_diff = 0;
  auto time_spy = SPY(&time);
  auto time_changer = [&time_diff, &time_spy](time_t* tloc) -> time_t {
    time_t fake_time = time_spy.invoke_real(tloc) + time_diff; // call time(time_t*) in libc
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
  return 0;
}
