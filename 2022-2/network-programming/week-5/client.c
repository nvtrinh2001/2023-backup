#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define BUFF_SIZE 8192
#define SIZE 1024

void menu() {
  printf("--------------------------------------------\n");
	printf("1. Send a message\n");
	printf("2. Send a file\n");
	printf("--------------------------------------------\n");
	printf("Enter your choice (1 or 2, other to quit): ");
}

int main(int argc, char *argv[]){
	int client_sock, choice;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received, bytes_read, total_bytes_sent, total_bytes_received;
  int port;
  char ip[INET_ADDRSTRLEN];

  if (argc != 3)
	{
		printf("[ERROR]: The client needs to be binded with an IP address, and a port.\n");
		exit(EXIT_FAILURE);
	} else {
    port = atoi(argv[2]);
    strcpy(ip, argv[1]);
  }

	//Step 1: Construct socket
	client_sock = socket(AF_INET,SOCK_STREAM,0);
	
	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);
	
	//Step 3: Request to connect server
	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately!\n");
		return 0;
	}
		
	//Step 4: Communicate with server			
	while(1){
    menu();
    scanf("%d", &choice);
    while (getchar() != '\n'); // Clear input buff

    bytes_sent = send(client_sock, &choice, sizeof(choice), 0);
    if(bytes_sent <= 0){
      printf("\nConnection closed!\n");
      break;
    }
    
    switch(choice){
      case 1:
      //send message
      printf("\nInsert string to send: ");
      memset(buff, '\0', (strlen(buff) + 1));
      fgets(buff, BUFF_SIZE, stdin);
      msg_len = strcspn(buff, "\n");  // Find index of first newline character
      buff[msg_len] = '\0';  // Replace newline character with null character
		
      msg_len = strlen(buff);
      if (msg_len == 0) break;
		
      bytes_sent = send(client_sock, buff, msg_len, 0);
      if(bytes_sent <= 0){
        printf("\nConnection closed!\n");
        break;
      }
		
      //receive echo reply
      bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
      if(bytes_received <= 0){
        printf("\nError!Cannot receive data from sever!\n");
        break;
      }
		
      buff[bytes_received] = '\0';
      printf("Reply from server: %s\n", buff);

      //receive echo reply
      bytes_received = recv(client_sock, buff, BUFF_SIZE-1, 0);
      if(bytes_received <= 0){
        printf("\nError!Cannot receive data from sever!\n");
        break;
      }
		
      buff[bytes_received] = '\0';
      printf("Reply from server: %s\n", buff);

      break;

      case 2:
      //send file
      printf("\nInput a file name:\n");
      char file_name[BUFF_SIZE];
      scanf("%s", file_name);
      FILE *fp = fopen(file_name, "rb");
      if (fp == NULL) {
        perror("[-]Error in reading file.");
        exit(1);
      }

      // Get file size
      fseek(fp, 0L, SEEK_END);
      int file_size = ftell(fp);
      fseek(fp, 0L, SEEK_SET);

      // Send file size over socket
      if (send(client_sock, &file_size, sizeof(file_size), 0) == -1) {
        perror("Error sending file size");
        return 1;
      }

      // Send file data over socket
      while ((bytes_read = fread(buff, 1, SIZE, fp)) > 0) {
        bytes_sent = send(client_sock, buff, bytes_read, 0);
        if (bytes_sent < 0) {
          perror("Error sending file data");
          return 1;
        }
        total_bytes_sent += bytes_sent;
        memset(buff, 0, SIZE);
      }
      printf("\nFile sent successfully. Total bytes sent: %d\n", total_bytes_sent);

      // Receive file data
      printf("\nReceive from server:\n");
      while (total_bytes_received < file_size) {
        bytes_received = recv(client_sock, buff, SIZE, 0);
        if (bytes_received < 0) {
          perror("Error receiving file data");
          return 1;
        }
        printf("%s\n", buff);
        total_bytes_received += bytes_received;
      }   

      break;

      default:
      break;
    }
	}
	
	//Step 4: Close socket
	close(client_sock);
	return 0;
}
