.SECONDEXPANSION:

LIBRARIES := elfspy

elfspy_OBJS := \
  Error.o \
  Report.o \
  Fail.o \
  MFile.o \
  SectionHeader.o \
  ELFInfo.o \
  GOTEntry.o \
  SPY.o \
  Call.o 

INC_DIR := ..
LIB_DIR := .

include makefile.inc

all : $(BINARIES) $(LIB_TARGETS)
	cd examples; make

.DEFAULT_GOAL := all
