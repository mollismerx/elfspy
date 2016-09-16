#ifndef ELFSPY_DEMO_H
#define ELFSPY_DEMO_H

#include <iostream>

int add(int a, int b);
int sub(int a, int b);
int f();
void g();
void g(int);

class MyClass
{
public:
  virtual ~MyClass();
  void method(int n);
  virtual void virtual_method();
};

class MyDerivedClass : public MyClass
{
public:
  virtual void virtual_method();
};

#endif
