CC=gcc
CFLAGS=-I.

justcache: main.c
	$(CC) -o logger main.c $(CFLAGS) -g -ggdb3