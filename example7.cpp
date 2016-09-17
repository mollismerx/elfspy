#include <cassert>
#include <iostream>

#include "elfspy/SPY.h"
#include "elfspy/Arg.h"
#include "elfspy/Fake.h"

#define private public
#define protected public
#include "elfspy/demo.h"

void func(MyClass*)
{
  std::cout << "func()" << std::endl;
}

void one_method1(One* one)
{
  std::cout << "1::1" << std::endl;
  assert(one->check1_ == 0xe110e110);
}

void one_method2(Two* two)
{
  std::cout << "2::1" << std::endl;
  assert(two->check1_ == 0xe110e110);
  assert(two->check2_ == 0xdeafa1de);
}

void two_method2(Two* two)
{
  std::cout << "2::2" << std::endl;
  assert(two->check1_ == 0xe110e110);
  assert(two->check2_ == 0xdeafa1de);
}

void one_method3(Three* three)
{
  std::cout << "3::1" << std::endl;
  assert(three->check1_ == 0xe110e110);
  assert(three->check3_ == 0xdeadbeef);
}

void three_method3(Three* three)
{
  std::cout << "3::3" << std::endl;
  assert(three->check1_ == 0xe110e110);
  assert(three->check3_ == 0xdeadbeef);
}

void one_method4(Four* four)
{
  std::cout << "4::1" << std::endl;
  assert(four->check1_ == 0xe110e110);
  assert(four->check2_ == 0xdeafa1de);
  assert(four->check3_ == 0xdeadbeef);
  assert(four->check4_ == 0xbadfeed);
}

void two_method4(Four* four)
{
  std::cout << "4::2" << std::endl;
  assert(four->check1_ == 0xe110e110);
  assert(four->check2_ == 0xdeafa1de);
  assert(four->check3_ == 0xdeadbeef);
  assert(four->check4_ == 0xbadfeed);
}

void three_method4(Four* four)
{
  std::cout << "4::3" << std::endl;
  assert(four->check1_ == 0xe110e110);
  assert(four->check2_ == 0xdeafa1de);
  assert(four->check3_ == 0xdeadbeef);
  assert(four->check4_ == 0xbadfeed);
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
  auto spy11 = SPY(&One::one_method);
  auto spy21 = SPY(&Two::one_method);
  auto spy22 = SPY(&Two::two_method);
  auto spy31 = SPY(&Three::one_method);
  auto spy33 = SPY(&Three::three_method);
  auto spy41 = SPY(&Four::one_method);
  auto spy42 = SPY(&Four::two_method);
  auto spy43 = SPY(&Four::three_method);
  auto fake11 = spy::fake(spy11, &one_method1);
  auto fake21 = spy::fake(spy21, &one_method2);
  auto fake22 = spy::fake(spy22, &two_method2);
  auto fake31 = spy::fake(spy31, &one_method3);
  auto fake33 = spy::fake(spy33, &three_method3);
  auto fake41 = spy::fake(spy41, &one_method4);
  auto fake42 = spy::fake(spy42, &two_method4);
  auto fake43 = spy::fake(spy43, &three_method4);
  One* one = new One;
  Two* two = new Two;
  Three* three = new Three;
  Four* four = new Four;
  one->one_method();
  two->one_method();
  two->two_method();
  three->one_method();
  three->three_method();
  four->one_method();
  four->two_method();
  four->three_method();
  return 0;
}
