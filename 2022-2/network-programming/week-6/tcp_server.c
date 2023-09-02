#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "accounts.h"

#define BACKLOG 20   /* Number of allowed connections */
#define BUFF_SIZE 1024
Account head = NULL;

void extract_username_passwd(char buffer[], char username[], char password[], char* delimiter);
/* Receive and echo message to client */
void *login_handler(void *);

int main(int argc, char *argv[])
{ 
	int listenfd, *connfd;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in *client; /* client's address information */
	socklen_t sin_size;
	pthread_t tid;
  int port;

  if (argc != 2) 
  {
    printf("[tcp_server.c]: The server needs to be binded with a port.\n");
    exit(EXIT_FAILURE);
  } 
  else 
  {
    port = atoi(argv[1]);
  }

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\n[tcp_server.c]: ");
		return 0;
	}
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(port); 
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   

	if(bind(listenfd,(struct sockaddr*)&server, sizeof(server))==-1){ 
		perror("\n[tcp_server.c]: ");
		return 0;
	}     

	if(listen(listenfd, BACKLOG) == -1){  
		perror("\n[tcp_server.c]: ");
		return 0;
	}
	
	sin_size=sizeof(struct sockaddr_in);
	client = malloc(sin_size);
  
  init_accounts(&head);

	while(1){		
		connfd = malloc(sizeof(int));
		if ((*connfd = accept(listenfd, (struct sockaddr *)client, &sin_size)) ==- 1)
			perror("\n[tcp_server.c]: ");
				
		printf("[tcp_server.c]: You got a connection from %s\n\n", inet_ntoa(client->sin_addr) ); /* prints client's IP */
		
		/* For each client, spawns a thread, and the thread handles the new client */
		pthread_create(&tid, NULL, &login_handler, connfd);	
	}
	
	close(listenfd);
	return 0;
}

void *login_handler(void *arg){
	int connfd;
	int bytes_sent, bytes_received;
	char buff[BUFF_SIZE + 1];
	char username[50], password[50], message[100];
  int choice;
  Account account = NULL;
  int is_login;
  char delimiter = '-';

	connfd = *((int *) arg);
	free(arg);
	pthread_detach(pthread_self());

  while (1) {
        bytes_received = recv(connfd, &choice, sizeof(choice), 0);
        if (bytes_received < 0)
          perror("\n[tcp_server.c]: ");
        else if (bytes_received == 0)
          printf("[tcp_server.c]: Connection closed.");

        printf("[tcp_server.c]: Received client choice: %d\n\n", choice);

        switch (choice) {
        case 1:
          if (is_login) {
            printf("[tcp_server.c]: Already logged in!\n\n");
            break;
          }

          bytes_received = recv(connfd, buff, BUFF_SIZE, 0); //blocking
          if (bytes_received < 0)
            perror("\n[tcp_server.c]: ");
          else if (bytes_received == 0)
            printf("[tcp_server.c]: Connection closed.");

          printf("[tcp_server.c]: Received %s\n", buff);
          extract_username_passwd(buff, username, password, &delimiter);
          printf("-> Username: %s\n", username);
          printf("-> Password: %s\n\n", password);

          account = login(&head, username, password);
          if (account == NULL) {
            strcpy(message, "[tcp_server.c]: Account does not exist!\n");
            is_login = 0;
          } else if (!account->status) {
            strcpy(message, "[tcp_server.c]: Account is blocked!\n");
            is_login = 0;
          } else if (!account->is_login && account->wrong_count <= 3) {
            strcpy(message, "[tcp_server.c]: Wrong password!\n");
            is_login = 0;
          } else if (account->is_login) {
            strcpy(message, "[tcp_server.c]: Logged in! Welcome back!\n");
            is_login = 1;
          }

          bytes_sent = send(connfd, message, BUFF_SIZE, 0);
          if (bytes_sent < 0)
            perror("\n[tcp_server.c]: ");
          printf("[tcp_server.c]: Sent %s\n\n", message);

          bytes_sent = send(connfd, &is_login, sizeof(is_login), 0);
          if (bytes_sent < 0)
            perror("\n[tcp_server.c]: ");
          printf("[tcp_server.c]: Sent login status to client. Status: %d\n\n", is_login);

          if (is_login)
            printf("[tcp_server.c]: Logged in!\n\n");
          else
            printf("[tcp_server.c]: Not logged in!\n\n");

          break;

        case 2:
          if (!is_login) {
            printf("[tcp_server.c]: You have to login first!\n\n");
            break;
          }

          logout(&account);
          printf("[tcp_server.c]: Logged out!\n\n");

          is_login = 0;
          bytes_sent = send(connfd, &is_login, sizeof(is_login), 0);
          if (bytes_sent < 0)
            perror("\n[tcp_server.c]: ");
          printf("[tcp_server.c]: Sent login status to client. Status: %d\n\n", is_login);
          break;

        default:
          printf("[tcp_server.c]: Closing connection...\n\n");
          break;
    }

    if (choice != 1 && choice != 2) {
        break; // Exit the loop if choice is neither 1 nor 2
    }
  } 
  
  close(connfd);	
  return NULL;
}

void extract_username_passwd(char buffer[], char username[], char password[], char* delimiter)
{
  char* token = strtok(buffer, delimiter);

  if (token != NULL) {
    strcpy(username, token);
    token = strtok(NULL, delimiter);
    if (token != NULL) {
        strcpy(password, token);
    } else {
      strcpy(password, "");
    }
  } else {
    strcpy(username, "");
    strcpy(password, "");
  }
}
