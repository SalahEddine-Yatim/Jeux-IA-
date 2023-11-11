CC = gcc
CFLAGS = -std=c99 -Wall -Werror -O3 -I../common
LDFLAGS = -flto
MAKE = make

alphabeta_iter: p4_iter_skel.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	find . -type f -executable -delete
