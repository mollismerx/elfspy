#include "demo.h"

#include <iostream>
#include <cassert>

int f()
{
  int rv = 0;
  rv += add(1, 2);
  rv += add(3, 4);
  return rv;
}

int add(int a, int b)
{
  std::cout << "add(" << a << ", " << b << ") -> " << a + b << std::endl;
  return a + b;
}

int sub(int a, int b)
{
  std::cout << "sub(" << a << ", " << b << ") -> " << a - b << std::endl;
  return a - b;
}

void g()
{
  std::cout << "g()" << std::endl;
}

void g(int n)
{
  std::cout << "g(" << n << ")" << std::endl;
}

MyClass::~MyClass()
{
}

void MyClass::method(int n)
{
  std::cout << "MyClass::method(" << n << ")" << std::endl;
}

void MyClass::virtual_method()
{
  std::cout << "MyClass::virtual_method" << std::endl;
}

void MyDerivedClass::virtual_method()
{
  method(2);
  std::cout << "MyDerivedClass::virtual_method" << std::endl;
}

void One::one_method()
{
  std::cout << "One::one" << std::endl;
  assert(check1_ == 0xe110e110);
}

void Two::one_method()
{
  std::cout << "Two::one" << std::endl;
  assert(check1_ == 0xe110e110);
  assert(check2_ == 0xdeafa1de);
}

void Two::two_method()
{
  std::cout << "Two::two" << std::endl;
  assert(check1_ == 0xe110e110);
  assert(check2_ == 0xdeafa1de);
}

void Three::one_method()
{
  std::cout << "Three::one" << std::endl;
  assert(check1_ == 0xe110e110);
  assert(check3_ == 0xdeadbeef);
}

void Three::three_method()
{
  std::cout << "Three::three" << std::endl;
  assert(check1_ == 0xe110e110);
  assert(check3_ == 0xdeadbeef);
}

void Four::one_method()
{
  std::cout << "Four::one" << std::endl;
  assert(check1_ == 0xe110e110);
  assert(check2_ == 0xdeafa1de);
  assert(check3_ == 0xdeadbeef);
  assert(check4_ == 0xbadfeed);
}

void Four::two_method()
{
  std::cout << "Four::two" << std::endl;
  assert(check1_ == 0xe110e110);
  assert(check2_ == 0xdeafa1de);
  assert(check3_ == 0xdeadbeef);
  assert(check4_ == 0xbadfeed);
}

void Four::three_method()
{
  std::cout << "Four::three" << std::endl;
  assert(check1_ == 0xe110e110);
  assert(check2_ == 0xdeafa1de);
  assert(check3_ == 0xdeadbeef);
  assert(check4_ == 0xbadfeed);
}

void Base1::method1()
{
  std::cout << "One::One" << std::endl;
  assert(check1_ == 12345678);
}

void Base2::method2()
{
  std::cout << "Two::Two" << std::endl;
  assert(check2_ == 87654321);
}

void Multiple::method1()
{
  std::cout << "Multiple::One" << std::endl;
  assert(check1_ == 12345678);
  assert(check2_ == 87654321);
}

void Multiple::method2()
{
  std::cout << "Multiple::Two" << std::endl;
  assert(check1_ == 12345678);
  assert(check2_ == 87654321);
}

