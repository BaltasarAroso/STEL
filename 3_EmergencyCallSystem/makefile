CC=gcc
CFLAGS=-Wall -I.
#CFLAGS+=-g
CFLAGS+=-std=gnu99
LDLIBS=-lm

file=sys_calls

$(file): src/$(file).c lib/linked_list.o
	$(CC) src/$(file).c lib/linked_list.o -o $(file) $(CFLAGS) $(LDLIBS)

clean:
	rm -f $(file) src/*.o  *.csv
	clear
