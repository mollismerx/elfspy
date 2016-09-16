#include <cassert>
#include <iostream>

#include "elfspy/SPY.h"
#include "elfspy/Fake.h"
#include "elfspy/demo.h"

void g1_replace(int n);
void g2_replace();

int main(int argc, char** argv)
{
  spy::initialise(argc, argv);
  auto g_int_spy = SPY((void (*)(int))&g);
  auto g_spy = SPY((void (*)())&g);
  auto g_int_fake = spy::fake(g_int_spy, &g1_replace);
  auto g_fake = spy::fake(g_spy, &g2_replace);
  g(1);
  g();
  return 0;
}

void g1_replace(int n)
{
  std::cout << "g1_replace(" << n << ")" << std::endl;
}

void g2_replace()
{
  std::cout << "g2_replace()" << std::endl;
}

