CC = musl-gcc
CFLAGS = -Os -Wall -Wextra -std=c99 -static
LDFLAGS = -static -s

all: mfetch

mfetch: src/mfetch.c
	$(CC) $(CFLAGS) $(LDFLAGS) src/mfetch.c -o mfetch

clean:
	rm -f mfetch
