CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES=server.cpp
FILES1+=client.cpp

server: $(FILES)
	$(CC) $(CFLAGS) -o server $(FILES)

client: $(FILES1)
	$(CC) $(CFLAGS) -o client $(FILES1)

clean:
	rm -f *.o server client

all: server client
