CC = gcc
CFLAGS = -ansi -pedantic-errors -Wall -Werror -g
PROGS = mysh

.PHONY: all clean

all: $(PROGS)

clean:
	rm -f *.o $(PROGS)

$(PROGS): mysh.o execute.o parser.o

parser.o: parser.h
execute.o: execute.h
mysh.o: parser.h execute.h