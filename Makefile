
CFLAGS=-O2

all: fan streamer ar

fan: fan.c
	gcc -o $@ $^ $(CFLAGS)

streamer: streamer.c
	gcc -o $@ $^ -lshout $(CFLAGS)

ar: ar.c
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm fan streamer ar

