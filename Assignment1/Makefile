CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES1=interfaceMonitor.cpp
FILES2=networkMonitor.cpp

interfaceMonitor: $(FILES1)
	$(CC) $(CFLAGS) -o interfaceMonitor $(FILES1)

networkMonitor: $(FILES2)
	$(CC) $(CFLAGS) -o networkMonitor $(FILES2)

clean:
	rm -f *.o interfaceMonitor networkMonitor

all: interfaceMonitor networkMonitor
