#include <cassert>
#include <iostream>

#include "elfspy/SPY.h"
#include "elfspy/Arg.h"
#include "elfspy/Fake.h"

#include "demo.h"

void func(MyClass*, int n)
{
  std::cout << "func(" << n << ")" << std::endl;
}

int main(int argc, char** argv)
{
  spy::initialise(argc, argv);
  auto method_spy = SPY(&MyClass::method);
  MyClass my_object;
  {
    auto method_this = spy::arg<0>(method_spy);
    auto method_arg0 = spy::arg<1>(method_spy);
    auto method_fake = spy::fake(method_spy, &func);
    my_object.method(117);
    assert(method_this.value() == &my_object);
    assert(method_arg0.value() == 117);
  }
  my_object.method(314);
  {
    auto method_this = spy::arg<0>(method_spy);
    auto method_arg0 = spy::arg<1>(method_spy);
    auto lambda = 
      [](MyClass* object, int n)
      {
        std::cout << "fake(" << n << ")" << std::endl;
      };
    auto method_fake = spy::fake(method_spy, lambda);
    my_object.method(256);
    assert(method_this.value() == &my_object);
    assert(method_arg0.value() == 256);
  }
  return 0;
}
