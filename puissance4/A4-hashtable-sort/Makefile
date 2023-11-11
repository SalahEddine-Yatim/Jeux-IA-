CC = gcc
CFLAGS = -std=c99 -Wall -Werror -O3 -I../common
LDFLAGS = -flto
MAKE = make

alphabeta_hash: alphabeta_hash_sort.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	find . -type f -executable -delete
