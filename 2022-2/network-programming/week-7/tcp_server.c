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
#include <errno.h>

#define BACKLOG 20   /* Number of allowed connections */
#define BUFF_SIZE 1024
#define MAX_CONNECTIONS 10
Account head = NULL;

void extract_username_passwd(char buffer[], char username[], char password[], char* delimiter);

int main(int argc, char *argv[])
{ 
	int listenfd;
  int new_fd;
	struct sockaddr_in server; /* server's address information */
	socklen_t sin_size;
  int port;
  int all_connections[MAX_CONNECTIONS];
  fd_set read_fd_set;
  int ret_val;

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
  
  init_accounts(&head);

  /* Initialize all_connections and set the first entry to server fd */
  for (int i = 0; i < MAX_CONNECTIONS; i++) {
    all_connections[i] = -1;
  }
  all_connections[0] = listenfd;

	while(1){		
    FD_ZERO(&read_fd_set);
      /* Reset the fd_set before passing it to the select call */
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
      if (all_connections[i] >= 0) {
        FD_SET(all_connections[i], &read_fd_set);
      }
    }

    /* Invoke select() and then wait for events! */
    printf("\nUsing select() to listen for incoming events\n");
    ret_val = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);

    /* select() woke up. Identify the fd that has events */
    if (ret_val >= 0 ) {
      printf("Select returned with %d\n", ret_val);
      /* Check if the fd with event is the server fd */
      if (FD_ISSET(listenfd, &read_fd_set)) { 
        /* accept the new connection */
        printf("Returned fd is %d (server's fd)\n", listenfd);
        new_fd = accept(listenfd, (struct sockaddr*)&server, &sin_size);
        if (new_fd >= 0) {
          printf("Accepted a new connection with fd: %d\n", new_fd);
          for (int i = 0; i < MAX_CONNECTIONS; i++) {
            if (all_connections[i] < 0) {
              all_connections[i] = new_fd; 
              break;
            }
          }
        } else {
          fprintf(stderr, "accept failed [%s]\n", strerror(errno));
        }
        ret_val--;
        if (!ret_val) continue;
      }

      int is_login = 0;
      /* Check if the fd with event is a non-server fd */
      for (int i = 1; i < MAX_CONNECTIONS; i++) {
        int bytes_sent, bytes_received;
        char buff[BUFF_SIZE];
        char username[50], password[50], message[100];
        int choice, msg_len;
        Account account = NULL;
        char delimiter = '-';

        if ((all_connections[i] > 0) && (FD_ISSET(all_connections[i], &read_fd_set))) 
        {
          // while(1) {
          /* read incoming data */   
          bytes_received = recv(all_connections[i], &choice, sizeof(choice), 0);
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

            bytes_received = recv(all_connections[i], &msg_len, sizeof(msg_len), 0); //blocking
            if (bytes_received < 0)
              perror("\n[tcp_server.c]: ");
            else if (bytes_received == 0)
              printf("[tcp_server.c]: Connection closed.");

            bytes_received = recv(all_connections[i], buff, BUFF_SIZE, 0); //blocking
            if (bytes_received < 0)
              perror("\n[tcp_server.c]: ");
            else if (bytes_received == 0)
              printf("[tcp_server.c]: Connection closed.");

            buff[msg_len] = '\0';
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

            bytes_sent = send(all_connections[i], message, BUFF_SIZE, 0);
            if (bytes_sent < 0)
              perror("\n[tcp_server.c]: ");
            printf("[tcp_server.c]: Sent %s\n\n", message);

            bytes_sent = send(all_connections[i], &is_login, sizeof(is_login), 0);
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
            bytes_sent = send(all_connections[i], &is_login, sizeof(is_login), 0);
            if (bytes_sent < 0)
              perror("\n[tcp_server.c]: ");
            printf("[tcp_server.c]: Sent login status to client. Status: %d\n\n", is_login);
            break;

          default:
            printf("[tcp_server.c]: Closing connection...\n\n");
            break;

          }
          // if (choice != 1 && choice != 2) {
          //   break; // Exit the loop if choice is neither 1 nor 2
          // }
          // }
          ret_val--;
          if (!ret_val) continue;
        }
      } /* for-loop */
    }
  }
  /* Last step: Close all the sockets */
  for (int i = 0;i < MAX_CONNECTIONS; i++) {
    if (all_connections[i] > 0) {
      close(all_connections[i]);
    }
  }
  return 0;
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
