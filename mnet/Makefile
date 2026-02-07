CC = musl-gcc
CFLAGS = -Os -Wall -Wextra -std=c99 -static
LDFLAGS = -static -s

all: mnet

mnet: src/mnet.c
	$(CC) $(CFLAGS) $(LDFLAGS) src/mnet.c -o mnet

clean:
	rm -f mnet
