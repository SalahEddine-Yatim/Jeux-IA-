CC = gcc
CFLAGS = -std=c99 -Wall -Werror -O3 -I../common
LDFLAGS = -flto
MAKE = make

minimax: p4_skeleton.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	find . -type f -executable -delete
