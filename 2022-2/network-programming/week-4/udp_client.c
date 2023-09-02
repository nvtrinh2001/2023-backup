#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[])
{
	int sockfd, choice;
	char buffer[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_received;
	socklen_t sin_size;
	char number[BUFF_SIZE];
	char username[50];
	char password[50];
	char newpasswd[50];

	if (argc != 3)
	{
		printf("[ERROR]: The client needs to be binded with an IP address, and a port.\n");
		exit(EXIT_FAILURE);
	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("[ERROR]: Failed creating a socket.");
		exit(EXIT_FAILURE);
	}
	printf("[SUCCESS]: Created a socket successfully.\n");

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	sendto(sockfd, "", strlen(""), MSG_CONFIRM, (const struct sockaddr *)&server_addr, sizeof(server_addr));

	do
	{
		printf("--------------------------------------------\n");
		printf("1. Login\n");
		printf("2. New password\n");
		printf("3. Chat\n");
		printf("4. Sign out\n");
		printf("--------------------------------------------\n");
		printf("Enter your choice (1 to 4, other to quit): ");
		scanf("%d", &choice);

		switch (choice)
		{
		case 1:
			strcpy(username, "");
			strcpy(password, "");
			strcpy(buffer, "");

			printf("What's your username: ");
			scanf("%s", username);
			printf("What's your password: ");
			scanf("%s", password);

			sprintf(buffer, "%d-%s-%s", 0, username, password);
			// printf("%s\n", buffer);

			sendto(sockfd, buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&server_addr, sizeof(server_addr));
			printf("[SUCCESS]: Successfully sent to server\n");

			bytes_received = recvfrom(sockfd, (char *)buffer, BUFF_SIZE,
									  MSG_WAITALL, (struct sockaddr *)&server_addr,
									  &sin_size);
			buffer[bytes_received] = '\0';
			printf("\n[INFO]: Message from server: \n%s\n", buffer);

			break;
		case 2:
			strcpy(username, "");
			strcpy(password, "");
			strcpy(newpasswd, "");
			strcpy(buffer, "");

			printf("What's your username: ");
			scanf("%s", username);
			printf("What's your password: ");
			scanf("%s", password);
			printf("What's your new password: ");
			scanf("%s", newpasswd);

			sprintf(buffer, "%d-%s-%s-%s", 1, username, password, newpasswd);
			sendto(sockfd, buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&server_addr, sizeof(server_addr));
			printf("[SUCCESS]: Successfully sent to server\n");

			bytes_received = recvfrom(sockfd, (char *)buffer, BUFF_SIZE,
									  MSG_WAITALL, (struct sockaddr *)&server_addr,
									  &sin_size);
			buffer[bytes_received] = '\0';
			bytes_received = recvfrom(sockfd, (char *)number, sizeof(number),
									  MSG_WAITALL, (struct sockaddr *)&server_addr,
									  &sin_size);
			number[bytes_received] = '\0';
			printf("\n[INFO]: Message from server: \n  %s\n  %s\n", buffer, number);
			break;
		case 3:
			// sendto(sockfd, "", strlen(""), MSG_CONFIRM, (const struct sockaddr *)&server_addr, sizeof(server_addr));
			printf("\n[ACTION]: Insert string: ");
			memset(buffer, '\0', (strlen(buffer) + 1));
			memset(number, '\0', (strlen(number) + 1));
			char temp[1000];
			scanf("%s", temp);
			sprintf(buffer, "%d-%s", 2, temp);

			sendto(sockfd, buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&server_addr, sizeof(server_addr));
			printf("[SUCCESS]: Successfully sent to server\n");

			bytes_received = recvfrom(sockfd, (char *)buffer, BUFF_SIZE,
									  MSG_WAITALL, (struct sockaddr *)&server_addr,
									  &sin_size);
			buffer[bytes_received] = '\0';
			bytes_received = recvfrom(sockfd, (char *)number, sizeof(number),
									  MSG_WAITALL, (struct sockaddr *)&server_addr,
									  &sin_size);
			number[bytes_received] = '\0';
			printf("\n[INFO]: Message from server: \n  %s\n  %s\n", buffer, number);
			break;
		case 4:
			// logout(&is_login);
			break;
		default:
			printf("\nGoodbye!\n");
			close(sockfd);
			return 0;
		}

	} while (choice >= 1 && choice <= 4);
}