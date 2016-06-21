CC=g++
CFLAGS = -Wall -g -std=c++11

all: main

clean:

	rm -rf *o
	rm tcp_server

main.o:
	${CC} ${CFLAGS} -c tcp_ex.cxx

main: main.o
	${CC} ${CFLAGS} tcp_ex.o -o tcp_server
