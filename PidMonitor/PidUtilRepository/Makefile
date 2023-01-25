CC=g++
CFLAGS=-I
CFLAGS+=-Wall
CFLAGS+=-c
AR=ar
pidUtil: pidUtil.cpp
	$(CC) $(CFLAGS) pidUtil.cpp -o pidUtil.o

lib: pidUtil.o
	$(AR) rcs libPidUtil.a pidUtil.o

clean: 
	rm -f *.o *.a

install: 
	cp libPidUtil.a ../.
	cp pidUtil.h ../.

all: pidUtil lib