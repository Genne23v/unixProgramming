CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES=LogServer.cpp
LIBS=-lpthread

LogServer: $(FILES)
	$(CC) $(CFLAGS) -o LogServer $(FILES) $(LIBS)

clean:
	rm -f *.o LogServer

all: LogServer
