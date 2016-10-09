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

class One
{
public:
  virtual ~One() = default;
  virtual void one_method();

protected:
  long check1_ = 0xe110e110;
};

class Two : virtual public One
{
public:
  virtual void one_method() override;
  virtual void two_method();

protected:
  long check2_ = 0xdeafa1de;
};

class Three : virtual public One
{
public:
  virtual void one_method() override;
  virtual void three_method();

protected:
  long check3_ = 0xdeadbeef;
};

class Four : public Two, public Three
{
public:
  virtual void one_method() override;
  virtual void two_method() override;
  virtual void three_method() override;

protected:
  long check4_ = 0xbadfeed;
};

class Base1
{
public:
  virtual ~Base1() = default;
  virtual void method1();

protected:
  long check1_ = 12345678;
};

class Base2
{
public:
  virtual ~Base2() = default;
  virtual void method2();

protected:
  long check2_ = 87654321;
};

class Multiple : public Base1, public Base2
{
public:
  virtual void method1() override;
  virtual void method2() override;
};

#endif
