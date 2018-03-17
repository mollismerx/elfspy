.SECONDEXPANSION:

LIBRARIES := demo
BINARIES := example1 example2 example3 example5 example6 example7 example9

demo_OBJS := \
  demo.o

example1_LIBRARIES := demo
example2_LIBRARIES := demo
example3_LIBRARIES := demo
example5_LIBRARIES := demo
example6_LIBRARIES := demo
example7_LIBRARIES := demo
example9_LIBRARIES := demo
example1_LIBS := elfspy
example2_LIBS := elfspy
example3_LIBS := elfspy
example5_LIBS := elfspy
example6_LIBS := elfspy
example7_LIBS := elfspy
example9_LIBS := elfspy

INC_DIR := ../..
LIB_DIR := . ..

all : $(BINARIES)

include ../makefile.inc

LD_FLAGS += -Wl,-rpath,$(PWD)/.. -Wl,-rpath,$(PWD)

