CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: server client

client: client.o 
	${CC} client.o -o client

server: server.o 
	${CC} server.o -o server

server.o: server.c
	${CC} ${CFLAGS} server.c

client.o: client.c
	${CC} ${CFLAGS} client.c

clean:
	rm -f *.o server client
