#include <cassert>
#include <iostream>

#include "elfspy/SPY.h"
#include "elfspy/Fake.h"
#include "demo.h"

bool g1_replaced = false;
bool g2_replaced = false;
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
  assert(g1_replaced);
  assert(g2_replaced);
  return 0;
}

void g1_replace(int n)
{
  g1_replaced = true;
  std::cout << "g1_replace(" << n << ")" << std::endl;
}

void g2_replace()
{
  g2_replaced = true;
  std::cout << "g2_replace()" << std::endl;
}

