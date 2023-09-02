#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // Discard characters
    }
}

void menu() {
  printf("--------------------------------------------\n");
	printf("1. Send a message\n");
	printf("2. Send an image file\n");
	printf("--------------------------------------------\n");
	printf("Enter your choice (1 or 2, other to quit): ");
}

int main() {
    int sockfd, is_login = 0;
    char is_yes;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char str1[50];
    char str2[50];
    
    // Create TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    
    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr)) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    
    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Connected to the server\n");
    
    while (1) {
        if (!is_login) {
          printf("\nNot login\n");
          printf("Do you want to login? (y/n) ");
          scanf("%c", &is_yes);
          if (is_yes != 'y') {
            break;
          }

          memset(buffer, 0, sizeof(buffer));
          memset(str1, 0, sizeof(str1));
          memset(str2, 0, sizeof(str2));

          printf("Enter username: ");
          scanf("%s", str1);
          printf("Enter password: ");
          scanf("%s", str2);

          sprintf(buffer, "0 %s %s", str1, str2);

          int bytes_sent = send(sockfd, buffer, strlen(buffer), 0);
          if (bytes_sent == -1) {
            perror("Send failed");
          }
          else if (bytes_sent == 0) {
            printf("Connection closed\n");
          }

          int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
          if (bytes_received == -1) {
            perror("Receive failed");
          }
          else if (bytes_received == 0) {
            printf("Connection closed\n");
          }
          else if (strcmp(buffer, "[tcp_server.c]: Logged in! Welcome back!\n") == 0) {
            is_login = 1;
          }
          clear_input_buffer();
        }
        else {
          menu();
          int choice;
          int bytes_sent;
          scanf("%d", &choice);

          switch (choice) {
            case 1:
              memset(buffer, 0, sizeof(buffer));
              char message[50];

              printf("Enter message: ");
              scanf("%s", message);

              sprintf(buffer, "1 %s", message);

              bytes_sent = send(sockfd, buffer, strlen(buffer), 0);
              if(bytes_sent <= 0){
                printf("\nConnection closed!\n");
                break;
              }
		
              //receive echo reply
              int bytes_received = recv(sockfd, buffer, BUFFER_SIZE-1, 0);
              if(bytes_received <= 0){
                printf("\nError!Cannot receive data from sever!\n");
                break;
              }
		
              buffer[bytes_received] = '\0';
              
              printf("Reply from server: %s\n", buffer);

              break;

            case 2:
              memset(buffer, 0, sizeof(buffer));
              char filename[50];
              printf("Enter filename: ");
              scanf("%s", filename);

              FILE *file = fopen(filename, "rb");
              if(file == NULL){
                printf("\nFile not found!\n");
                break;
              }

              // Determine the file size
              fseek(file, 0, SEEK_END);
              long file_size = ftell(file);
              rewind(file);

              sprintf(buffer, "2 %s %ld", filename, file_size);

              bytes_sent = send(sockfd, buffer, strlen(buffer), 0);
              if(bytes_sent <= 0){
                printf("\nConnection closed!\n");
                break;
              }

              int bytes_read;
              // Read and send the image data
              while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                if (send(sockfd, buffer, bytes_read, 0) != bytes_read) {
                  perror("Failed to send image data");
                }
              }              

              printf("File sent!\n");
              break;

            case 3:
              is_login = 0;
              memset(buffer, 0, sizeof(buffer));
              sprintf(buffer, "3 %s", str1);
              bytes_sent = send(sockfd, buffer, strlen(buffer), 0);
              if(bytes_sent <= 0){
                printf("\nConnection closed!\n");
                break;
              }
              printf("Logged out!\n");
              clear_input_buffer();
          
              break;
          }
        }
    }
    
    close(sockfd);
    
    return 0;
}

