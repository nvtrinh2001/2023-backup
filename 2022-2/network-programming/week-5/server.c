#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#define PORT 5550   /* Port that will be opened */ 
#define BACKLOG 2   /* Number of allowed connections */
#define BUFF_SIZE 1024
#define FILE_NAME "recv.txt"

void separate_num_str(char* buffer, char** character, char** digit);

int main(int argc, char* argv[])
{
	int listen_sock, conn_sock; /* file descriptors */
	char recv_data[BUFF_SIZE];
  char *character = NULL;
  char *digit = NULL;
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	int sin_size, choice;
  int bytes_written, total_bytes_written;

  //Step 0: Initialize
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    exit(1);
  }
  
  int port = atoi(argv[1]);
	
	//Step 1: Construct a TCP socket to listen connection request
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: \n");
		return 0;
	}
	
	//Step 2: Bind address to socket
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(port);   /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   
	if(bind(listen_sock, (struct sockaddr*)&server, sizeof(server))==-1){ /* calls bind() */
		perror("\nError: \n");
		return 0;
	}     
	
	//Step 3: Listen request from client
	if(listen(listen_sock, BACKLOG) == -1){  /* calls listen() */
		perror("\nError: \n");
		return 0;
	}
	
	//Step 4: Communicate with client
	while(1){
		//accept request
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock,( struct sockaddr *)&client, &sin_size)) == -1) 
			perror("\nError: \n");
  
		printf("You got a connection from %s\n", inet_ntoa(client.sin_addr) ); /* prints client's IP */
		
		//start conversation
		while(1){
			//receives message from client
      bytes_received = recv(conn_sock, &choice, sizeof(choice), 0);
			if (bytes_received <= 0){
				printf("\nConnection closed\n");
				break;
			}
			else{
				printf("\nChoice: %d\n", choice);
			}
      
      switch (choice) {
        case 1:
          //receives message from client
          memset(recv_data, '\0', BUFF_SIZE);
			    bytes_received = recv(conn_sock, recv_data, BUFF_SIZE-1, 0); //blocking
			    if (bytes_received <= 0){
				    printf("\nConnection closed\n");
				    break;
			    }
			    else{
				    recv_data[bytes_received] = '\0';
				    printf("\nReceive: %s\n", recv_data);
			    }

          separate_num_str(recv_data, &character, &digit);
          printf("Result: %s\n", character);
          printf("Result: %s\n", digit);

          bytes_sent = send(conn_sock, character, strlen(character) * sizeof(char), 0); 
          if (bytes_sent <= 0){
            printf("\nError sending back message\n");
            break;
          }

          bytes_sent = send(conn_sock, digit, strlen(digit) * sizeof(char), 0);
          if (bytes_sent <= 0){
            printf("\nError sending back message\n");
            break;
          }
          break;
        
        case 2:
          // Open output file for writing
          FILE* fp = fopen(FILE_NAME, "wb");
          if (fp == NULL) {
            perror("Error opening output file");
            return 1;
          } 

          // Receive file size from client
          int file_size;
          if (recv(conn_sock, &file_size, sizeof(file_size), 0) == -1) {
            perror("Error receiving file size");
            return 1;
          }
          printf("\nFile size: %d\n", file_size);

          // Receive file
          printf("\nReceive:\n");

          // Receive file data over socket
          while (total_bytes_written < file_size) {
            bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0);
            printf("%s\n", recv_data);
            if (bytes_received == -1) {
              perror("Error receiving file data");
              return 1;
            }

            if (bytes_received == 0) {
              printf("Client disconnected\n");
              break;
            }

            bytes_sent = send(conn_sock, recv_data, bytes_received, 0);
            if (bytes_sent == -1) {
              perror("Error sending file data");
              return 1;
            }

            total_bytes_written += bytes_received;
            memset(recv_data, 0, BUFF_SIZE);
          }

          printf("File received successfully. Total bytes received: %d\n", total_bytes_written);
          break;

        default:
          break;
      }
		}//end conversation
    close(conn_sock);	
	}
	
	close(listen_sock);
	return 0;
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
