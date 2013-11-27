CFLAGS=-O2
CFLAGS+=-ggdb
CFLAGS+=-DDEBUG

all: fan streamer ar symer

fan: fan.c
	gcc $(CFLAGS) -o $@ $^

streamer: streamer.c
	gcc $(CFLAGS) -o $@ $^ -lshout

ar: ar.c
	gcc $(CFLAGS) -I/home/adam/work/spead/src -o $@ $^ -lspead

symer: symer.c
	gcc $(CFLAGS) -I/home/adam/work/spead/src -o $@ $^ -lspead

clean:
	rm fan streamer ar symer core

