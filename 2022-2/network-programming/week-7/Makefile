CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: server client

client: tcp_client.o linked_list.o accounts.o
	${CC} tcp_client.o linked_list.o accounts.o -o client

server: tcp_server.o linked_list.o accounts.o
	${CC} tcp_server.o linked_list.o accounts.o -o server

tcp_server.o: tcp_server.c
	${CC} ${CFLAGS} tcp_server.c

linked_list.o: linked_list.c
	${CC} ${CFLAGS} linked_list.c

accounts.o: accounts.c
	${CC} ${CFLAGS} accounts.c

clean:
	rm -f *.o server client
