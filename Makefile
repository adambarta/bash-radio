
CFLAGS=-O2

all: fan streamer

fan: fan.c
	gcc -o $@ $^ $(CFLAGS)

streamer: streamer.c
	gcc -o $@ $^ -lshout $(CFLAGS)

clean:
	rm fan streamer

