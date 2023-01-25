CC=g++
CFLAGS=-I./PidUtilRepository
CFLAGS+=-Wall
FILES=Lab2.cpp
LIBS=-L./PidUtilRepository -lPidUtil

Lab2: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o Lab2 $(LIBS)
	
clean: 
	rm -f *.o Lab2

all: Lab2