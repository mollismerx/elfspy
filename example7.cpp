#include <cassert>
#include <iostream>

#include "elfspy/SPY.h"
#include "elfspy/Arg.h"
#include "elfspy/Fake.h"

#include "elfspy/demo.h"

void func(MyClass*)
{
  std::cout << "func()" << std::endl;
}

int main(int argc, char** argv)
{
  spy::initialise(argc, argv);
  auto method_spy = SPY(&MyClass::virtual_method);
  MyClass my_object;
  auto method_this = spy::arg<0>(method_spy);
  auto method_fake = spy::fake(method_spy, &func);
  my_object.virtual_method();
  assert(method_this.value() == &my_object);
  MyClass* my_derived_object = new MyDerivedClass;
  my_derived_object->virtual_method();
  assert(method_this.size() == 1); // no new value captured
  MyClass* my_heap_object = new MyClass;
  my_heap_object->virtual_method();
  assert(method_this.value(1) == my_heap_object);
  return 0;
}
