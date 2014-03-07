CC=gcc
CFLAGS=-I.


all: linkedlist argv_parse

clean:
	rm -rf linkedlist argv_parse

linkedlist: linkedlist.c
	$(CC) linkedlist.c -o linkedlist -lm -I.
	
argv_parse: argv_parse.c
	$(CC) argv_parse.c -o argv_parse -lm -I.	
	

