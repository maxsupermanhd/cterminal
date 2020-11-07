.PHONY: all clean

all: main

cterminal.o: cterminal.c cterminal.h
	g++ -c cterminal.c -o cterminal.o -g -Wall -Wextra -Wpedantic

main: cterminal.o main.c
	g++ cterminal.o main.c -o main -lpthread -g -Wall -Wextra -Wpedantic

clean:
	rm -f cterminal.o main
