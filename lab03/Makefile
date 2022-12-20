all: lab4 clean

CC=gcc
CCFLAGS=-c -pedantic -std=gnu11
DEBUG_FLAGS =  -w -DDEBUG -c -g3 -O0
RELEASE_FLAGS = -O3

debug : CCFLAGS += $(DEBUG_FLAGS)
debug : all

release : CCFLAGS += $(RELEASE_FLAGS)
release : all

lab4: lab4.o
	${CC} lab4.o child.o -o lab4

lab4.o: lab4.c
	${CC} $(CCFLAGS) lab4.c child.c

clean:
	rm -rf *.o