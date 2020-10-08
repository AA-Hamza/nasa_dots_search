CC=gcc
CFLAGS=-lsqlite3
DEPS=filling_databases.h

search: main.c filling_databases.c
	$(CC) -o search main.c filling_databases.c $(CFLAGS)

clean:
	rm search
