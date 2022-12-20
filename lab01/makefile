TARGET = main
CC = gcc

CCFLAGS = -Wall -pedantic -std=c11
DEBUG_FLAGS =  -w -DDEBUG -c -g3 -O0
RELEASE_FLAGS = -O3

debug : CCFLAGS += $(DEBUG_FLAGS)
debug : all

release : CCFLAGS += $(RELEASE_FLAGS)
release : all

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

all : $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) 

%.o : %.c
	$(CC) $(CCFLAGS) -c $<  -o $@

clean:
	rm $(TARGET) *.o