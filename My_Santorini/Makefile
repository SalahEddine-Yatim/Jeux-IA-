CC = gcc
CFLAGS = -std=c99 -Wall -Werror -O3 -I../common
LDFLAGS = -flto
MAKE = make

santorini: copîenew.c macros.h 
	$(CC) $(CFLAGS) -o  $@ $^ $(LDFLAGS) 

clean:
	find . -type f -executable -delete
