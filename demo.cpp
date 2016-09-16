#include "elfspy/demo.h"

#include <iostream>

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
