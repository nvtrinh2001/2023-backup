#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "accounts.h"

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
Account head = NULL;

void extract(char buffer[], char str1[], char str2[], char str3[], char* delimiter)
{
  char* token = strtok(buffer, delimiter);

  if (token != NULL) {
    strcpy(str1, token);
    token = strtok(NULL, delimiter);
    if (token != NULL) {
        strcpy(str2, token);
        token = strtok(NULL, delimiter);
        if (token != NULL) {
            strcpy(str3, token);
        } else {
            strcpy(str3, "");
        }
    } else {
      strcpy(str2, "");
      strcpy(str3, "");
    }
  } else {
    strcpy(str1, "");
    strcpy(str2, "");
    strcpy(str3, "");
  }
}

void separate_num_str(char* buffer, char** character, char** digit)
{
    int buffer_len = strlen(buffer);
    int character_len = 0;
    int digit_len = 0;
    *character = (char*)malloc(sizeof(char) * buffer_len);
    *digit = (char*)malloc(sizeof(char) * buffer_len);   

    for (int i = 0; i < buffer_len; i++)
    {
        if ((buffer[i] >= 'a' && buffer[i] <= 'z') || (buffer[i] >= 'A' && buffer[i] <= 'Z'))
        {
            (*character)[character_len] = buffer[i];
            character_len++;
        }
        else if (buffer[i] >= '0' && buffer[i] <= '9')
        {
            (*digit)[digit_len] = buffer[i];
            digit_len++;
        }
        else
        {
            strcpy(*character, "string errors");
            strcpy(*digit, "string errors");
            return;
        }
    }

    if (character_len != 0)
    {
        (*character)[character_len] = '\0';
    }
    else
    {
        strcpy(*character, "no characters");
    }

    if (digit_len != 0)
    {
        (*digit)[digit_len] = '\0';
    }
    else
    {
        strcpy(*digit, "no digits");
    }

    return;
}

void handle_client_request(int sd) {
  char buffer[BUFFER_SIZE];
  char str1[50];
  char str2[50];
  char str3[50];
  char delimiter = ' ';

  memset(buffer, 0, BUFFER_SIZE);
  memset(str1, 0, 50);
  memset(str2, 0, 50);
  memset(str3, 0, 50);

  int bytes_received = recv(sd, buffer, BUFFER_SIZE, 0);
  if (bytes_received < 0)
    perror("\n[tcp_server.c]: ");
  else if (bytes_received == 0)
    printf("[tcp_server.c]: Connection closed.");

  extract(buffer, str1, str2, str3, &delimiter);

  switch (atoi(str1)) {
    case 0:
      Account account = login(&head, str2, str3);
      char message[BUFFER_SIZE];

      if (account == NULL) {
        strcpy(message, "[tcp_server.c]: Account does not exist!\n");
      } else if (!account->status) {
        strcpy(message, "[tcp_server.c]: Account is blocked!\n");
      } else if (!account->is_login && account->wrong_count <= 3) {
        strcpy(message, "[tcp_server.c]: Wrong password!\n");
      } else if (account->is_login) {
        strcpy(message, "[tcp_server.c]: Logged in! Welcome back!\n");
      }

      int bytes_sent = send(sd, message, BUFFER_SIZE, 0);
      if (bytes_sent < 0)
        perror("\n[tcp_server.c]: ");
      printf("[tcp_server.c]: Sent %s\n\n", message);
      break;

    case 1:
      //receives message from client
      char* character = (char*)malloc(sizeof(char) * 50);
      char* digit = (char*)malloc(sizeof(char) * 50);

      memset(message, 0, BUFFER_SIZE);
      separate_num_str(str2, &character, &digit);
      printf("Result: %s\n", character);
      printf("Result: %s\n", digit);
      sprintf(message, "%s %s\n", character, digit);

      bytes_sent = send(sd, message, strlen(message), 0); 
      if (bytes_sent <= 0){
        printf("\nError sending back message\n");
        break;
      }

      break;

    case 2:
      FILE *file = fopen(strcat(str2, ".cp"), "wb");
      if (file == NULL) {
        printf("\nError opening file\n");
        break;
      }

      int bytes_read;
      while ((bytes_read = recv(sd, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes_read, file);
      } 

      fclose(file);

      printf("\nFile received!\n");
      
      break;

    case 3:
      account = search(head, str2);     
      if (account == NULL) {
        printf("\nAccount does not exist!\n");
        break;
      }
      logout(&account);
      printf("\nLogged out!\n");
      break;
  }
}

int main() {
    int sockfd, newsockfd, maxfd, activity, i, sd;
    int client_sockets[MAX_CLIENTS];
    fd_set readfds;
    struct sockaddr_in server_addr, client_addr;
    
    // Create TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    memset(client_sockets, 0, sizeof(client_sockets));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);
    
    // Bind the socket to a specific IP address and port
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind");
        exit(EXIT_FAILURE);
    }
    
    // Listen for incoming connections
    if (listen(sockfd, 3) < 0) {
        perror("Failed to listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port 8888...\n");
    
    // Accept incoming connections
    int addrlen = sizeof(client_addr);
    init_accounts(&head);
    print(head);

    while (1) {
        FD_ZERO(&readfds);
        
        FD_SET(sockfd, &readfds);
        maxfd = sockfd;
        
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];
            
            if (sd > 0)
                FD_SET(sd, &readfds);
            
            if (sd > maxfd)
                maxfd = sd;
        }
        
        // Wait for activity on any of the sockets
        activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }
        
        // Check for new connection
        if (FD_ISSET(sockfd, &readfds)) {
            if ((newsockfd = accept(sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen)) < 0) {
                perror("Accept error");
                exit(EXIT_FAILURE);
            }
            
            printf("New connection, socket fd is %d, IP is : %s, port : %d\n",
                   newsockfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            
            // Add the new socket to the array of client sockets
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = newsockfd;
                    break;
                }
            }
        }
        
        // Check for data from clients
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];
            
            if (FD_ISSET(sd, &readfds)) {
                handle_client_request(sd);
            }
        }
    }
    
    return 0;
}

