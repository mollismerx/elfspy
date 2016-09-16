.SECONDEXPANSION:

LIBRARIES := elfspy demo
BINARIES := example1 example2 example3 example5 example6 example7 example9

elfspy_OBJS := \
  Error.o \
  Report.o \
  Fail.o \
  MFile.o \
  ELFInfo.o \
  GOTEntry.o \
  SPY.o \
  Call.o 

demo_OBJS := \
  demo.o

example1_LIBRARIES := elfspy demo
example2_LIBRARIES := elfspy demo
example3_LIBRARIES := elfspy demo
example5_LIBRARIES := elfspy demo
example6_LIBRARIES := elfspy demo
example7_LIBRARIES := elfspy demo
example9_LIBRARIES := elfspy demo

all : $(BINARIES)

CXXFLAGS := -I.. -std=c++14 -g -fPIC -O0

GCLIB_PATH := /home/robin/glibc/2.23/bin \
GCLIB_LIBPATH := $(GCLIB_PATH)/lib \

DYNAMIC_DEBUG := \
  -L$(GCLIB_LIBPATH) \
  -Wl,--rpath=$(GCLIB_LIBPATH) \
  -Wl,--dynamic-linker=$(GCLIB_LIBPATH)/ld-linux-x86-64.so.2

BIND_ALL := -z now

LD_FLAGS := # $(BIND_ALL) 
#LD_FLAGS := $(DYNAMIC_DEBUG) -rdynamic -ldl

%.o : %.cpp
	g++ $(CXXFLAGS) -c $< -o $@

$(patsubst %,lib%.so,$(LIBRARIES)) : $$($$(patsubst lib%.so,%,$$@)_OBJS)
	g++ $(CXXFLAGS) -shared $^ -o $@ $(LD_FLAGS)
	readelf -Wa $@ | c++filt > $(@:%.so=%.elf)

$(BINARIES) : $$(@).cpp $$(patsubst %,lib%.so,$$($$@_LIBRARIES))
	g++ $(CXXFLAGS) $(@).cpp -o $@ $(LD_FLAGS) -L. $(patsubst %,-l%,$($@_LIBRARIES)) -rdynamic -ldl
	readelf -Wa $@ | c++filt > $(@).elf

clean :
	rm -f *.o *.so $(BINARIES) *.elf
