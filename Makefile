CFLAGS=-O2
CFLAGS=-ggdb
CFLAGS=-DDEBUG

all: fan streamer ar symer

fan: fan.c
	gcc -o $@ $^ $(CFLAGS)

streamer: streamer.c
	gcc -o $@ $^ -lshout $(CFLAGS)

ar: ar.c
	gcc -o $@ $^ $(CFLAGS)

symer: symer.c
	gcc $(CFLAGS) -I/home/adam/work/spead/src -o $@ $^ -lspead

clean:
	rm fan streamer ar symer core

