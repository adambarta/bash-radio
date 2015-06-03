CFLAGS=-O2
CFLAGS+=-ggdb
CFLAGS+=-DDEBUG
CFLAGS+=-DIKATCP
CFLAGS+=-fstack-protector
CFLAGS+=-Wall

all: fan streamer ar symer server

fan: fan.c
	gcc $(CFLAGS) -o $@ $^

streamer: streamer.c
	gcc $(CFLAGS) -o $@ $^ -lshout

ar: ar.c
	gcc $(CFLAGS) -I/home/adam/work/spead/src -o $@ $^ -lspead

symer: symer.c
	gcc $(CFLAGS) -I/home/adam/work/spead/src -o $@ $^ -lspead

#sslmin.o: sslmin.c sslmin.h
#	gcc $(CFLAGS) -c -o $@ $<

server.o: server.c
	gcc $(CFLAGS) -I/home/adam/work/spead/src -I/home/adam/build/matrixssl-3-4-2-open -c -o $@ $^ 

server: server.o
	gcc -Wall -o $@ $^ /home/adam/build/matrixssl-3-4-2-open/libmatrixssl.a -lspead 

clean:
	rm -f fan streamer ar symer core server *.o

