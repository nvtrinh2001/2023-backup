#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

void menu() {
  printf("--------------------------------------------\n");
  printf("1. Login\n");
  printf("2. Logout\n");
	printf("--------------------------------------------\n");
	printf("Enter your choice (1 or 2, other to quit): ");
}

int main(int argc, char *argv[]){
	int client_sock;
	char buff[BUFF_SIZE + 1];
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;
  char username[50], password[50];
  int choice;
  int login_status = 0;
  int port;
  char ip[INET_ADDRSTRLEN];

  if (argc != 3)
	{
		printf("[tcp_client.c]: The client needs to be binded with an IP address, and a port.\n");
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
		printf("\n[tcp_client.c]: Error!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
		
	//Step 4: Communicate with server			

  while(1) {
    menu();
    scanf(" %d", &choice);
    bytes_sent = send(client_sock, &choice, sizeof(choice), 0);
    if(bytes_sent < 0)
      perror("\n[tcp_client.c]: ");
    printf("\n[tcp_client.c]: Successfully sent your choice to server\n\n");

    switch(choice) {
    case 1:
      if (login_status) {
        printf("[tcp_client.c]: You are already logged in.\n");
        break;
      }

      strcpy(username, "");
      strcpy(password, "");
      strcpy(buff, "");

      printf("-> What's your username: ");
      scanf("%s", username);
      printf("-> What's your password: ");
      scanf("%s", password);

      sprintf(buff, "%s-%s", username, password);
      // printf("%s\n", buffer);
      msg_len = strlen(buff);

      bytes_sent = send(client_sock, &msg_len, sizeof(msg_len), 0);
      if(bytes_sent < 0)
        perror("\n[tcp_client.c]: ");

      bytes_sent = send(client_sock, buff, msg_len, 0);
      if(bytes_sent < 0)
        perror("\n[tcp_client.c]: ");
      printf("\n[tcp_client.c]: Successfully sent to server\n");

      bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
      if (bytes_received < 0)
        perror("\n[tcp_client.c]: ");
      else if (bytes_received == 0)
        printf("[tcp_client.c]: Connection closed.\n");
      
      buff[bytes_received] = '\0';
      printf("\n[tcp_client.c]: Message from server: \n%s\n", buff);

      bytes_received = recv(client_sock, &login_status, sizeof(login_status), 0);
      if (bytes_received < 0)
        perror("\n[tcp_client.c]: ");
      else if (bytes_received == 0)
        printf("[tcp_client.c]: Connection closed.\n");

      break;

    case 2:
      if (!login_status) {
        printf("[tcp_client.c]: You have to login first.\n");
        break;
      }

      bytes_received = recv(client_sock, &login_status, sizeof(login_status), 0);
      if (bytes_received < 0)
        perror("\n[tcp_client.c]: ");
      else if (bytes_received == 0)
        printf("[tcp_client.c]: Connection closed.\n");
      
      if (!login_status) {
        printf("[tcp_client.c]: You are logged out.\n");
      }
      break;

    default:
      printf("[tcp_client.c]: Exitting...\n");
      break;
    } 

    if (choice != 1 && choice != 2) {
        break; // Exit the loop if choice is neither 1 nor 2
    }
  } 

	//Step 4: Close socket
	close(client_sock);
	return 0;
}
