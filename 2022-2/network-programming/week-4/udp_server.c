#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "accounts.h"

#define BUFF_SIZE 1024

bool separate_num_str(char buffer[], char string[], char digit[]);
void extract_option(char buffer[], int *choice);
void extract_username_passwd(char buffer[], char username[], char password[]);
void extract_username_passwd_newpasswd(char buffer[], char username[], char password[], char newpasswd[]);

int main(int argc, char *argv[])
{
	Account head = NULL;
	int is_logged_in = 0;
	char username[50], password[50], newpasswd[50], message[100];
	int choice;

	socklen_t sin_size;
	int sockfd, count, bytes_received;
	char buffer[BUFF_SIZE], character[BUFF_SIZE], digit[BUFF_SIZE];
	struct sockaddr_in server_addr, client_addr;
	struct sockaddr_in *client_addrs = (struct sockaddr_in *)(malloc(sizeof(struct sockaddr_in) * 2));

	if (argc != 2)
	{
		printf("[ERROR]: Invalid arguments.\n");
		exit(EXIT_FAILURE);
	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("[ERROR]: Failed trying to create a socket!\n");
		exit(EXIT_FAILURE);
	}
	printf("[SUCCESS]: Created a socket successfully!\n");

	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));

	server_addr.sin_family = AF_INET; // IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(atoi(argv[1]));

	if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("[ERROR]: Failed trying to bind the server to a socket!\n");
		exit(EXIT_FAILURE);
	}
	printf("[SUCCESS]: Bind the socket to the server successfully!\n");

	init_accounts(&head);
	// print_accounts(head, 1);
	while (true)
	{
		bytes_received = recvfrom(sockfd, (char *)buffer, BUFF_SIZE, MSG_WAITALL, (struct sockaddr *)&client_addr, &sin_size);
		buffer[bytes_received] = '\0';
		printf("\n[INFO]: Received %s from %s:%d\n", buffer, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		count++;
		client_addrs[count % 2] = client_addr;
		extract_option(buffer, &choice);

		switch (choice)
		{
		case 0:
			extract_username_passwd(buffer, username, password);
			printf("Username: %s\n", username);
			printf("Password: %s\n", password);
			Account node = login(&head, &is_logged_in, username, password);

			if (node == NULL)
			{
				strcpy(message, "[ERROR]: Account not exist!\n");
			}
			else if (!node->status)
			{
				strcpy(message, "[ERROR]: Account is blocked!\n");
			}
			else if (!is_logged_in && node->wrong_count <= 3)
			{
				strcpy(message, "[ERROR]: Wrong password!\n");
			}
			else if (is_logged_in)
			{
				strcpy(message, "[SUCCESS]: Logged in! Welcome back!\n");
			}

			sendto(sockfd, message, strlen(message), MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr));
			printf("\n[SUCCESS]: Successfully sent confirmation message to %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			break;
		case 1:
			switch (is_logged_in)
			{
			case 0:
				strcpy(character, "[ERROR]: Please sign in first!\n");
				strcpy(digit, "");
				break;

			case 1:
				extract_username_passwd_newpasswd(buffer, username, password, newpasswd);
				printf("Username: %s\n", username);
				printf("Password: %s\n", password);
				printf("New password: %s\n", newpasswd);
				bool passwd_check = check_password(head, username, password);
				if (!passwd_check)
				{
					printf("\n[ERROR]: Wrong password!\n");
					strcpy(character, "wrong password");
					strcpy(digit, "");
					break;
				}

				bool is_string_valid = separate_num_str(newpasswd, character, digit);
				if (is_string_valid)
				{
					change_password(&head, username, newpasswd);
				}
				else
				{
					printf("\n[ERROR]: Invalid string!\n");
					strcpy(character, "contain invalid characters");
					strcpy(digit, "contain invalid digits");
				}
				break;
			}

			sendto(sockfd, (char *)character, strlen(character), MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr));
			sendto(sockfd, (char *)digit, strlen(digit), MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr));

			printf("\n[SUCCESS]: Successfully sent `%s` and `%s` to %s:%d\n", character, digit, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			break;
		case 2:
			switch (is_logged_in)
			{
			case 0:
				strcpy(character, "[ERROR]: Please sign in first!\n");
				strcpy(digit, "");
				break;

			case 1:
				bool is_string_valid = separate_num_str(buffer, character, digit);
				if (!is_string_valid)
				{
					printf("\n[ERROR]: Invalid string!\n");
					strcpy(character, "contain invalid characters");
					strcpy(digit, "contain invalid digits");
				}
				break;
			}

			if (count % 2 == 0 && count >= 2)
			{
				sendto(sockfd, (char *)character, strlen(character), MSG_CONFIRM, (const struct sockaddr *)&client_addrs[1], sizeof(client_addrs[1]));
				sendto(sockfd, (char *)digit, strlen(digit), MSG_CONFIRM, (const struct sockaddr *)&client_addrs[1], sizeof(client_addrs[1]));

				printf("\n[SUCCESS]: Successfully sent `%s` and `%s` to %s:%d\n", character, digit, inet_ntoa(client_addrs[1].sin_addr), ntohs(client_addrs[1].sin_port));
			}
			else if (count % 2 == 1 && count >= 2)
			{
				sendto(sockfd, (char *)character, strlen(character), MSG_CONFIRM, (const struct sockaddr *)&client_addrs[0], sizeof(client_addrs[0]));
				sendto(sockfd, (char *)digit, strlen(digit), MSG_CONFIRM, (const struct sockaddr *)&client_addrs[0], sizeof(client_addrs[0]));

				printf("\n[SUCCESS] Successfully sent `%s` and `%s` to %s:%d\n", character, digit, inet_ntoa(client_addrs[1].sin_addr), ntohs(client_addrs[1].sin_port));
			}
			break;
		case 3:
			strcpy(message, "[SUCCESS]: Bye!\n");
			sendto(sockfd, message, strlen(message), MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr));
			printf("\n[SUCCESS]: Successfully sent confirmation message to %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			break;
		}
	}
	return 0;
}

bool separate_num_str(char buffer[], char character[], char digit[])
{
	int buffer_len = strlen(buffer);
	int character_len = 0;
	int digit_len = 0;
	for (int i = 2; i < buffer_len; i++)
	{
		if ((buffer[i] >= 'a' && buffer[i] <= 'z') || (buffer[i] >= 'A' && buffer[i] <= 'Z'))
		{
			character[character_len] = buffer[i];
			character_len++;
		}
		else if (buffer[i] >= '0' && buffer[i] <= '9')
		{
			digit[digit_len] = buffer[i];
			digit_len++;
		}
		else
		{
			return false;
		}
	}

	if (character_len != 0)
	{
		character[character_len] = '\0';
	}
	else
	{
		strcpy(character, "no characters");
	}

	if (digit_len != 0)
	{
		digit[digit_len] = '\0';
	}
	else
	{
		strcpy(digit, "no digits");
	}

	return true;
}

void extract_option(char buffer[], int *choice)
{
	*choice = buffer[0] - '0';
}

void extract_username_passwd(char buffer[], char username[], char password[])
{
	char *token;

	token = strtok(buffer, "-");

	token = strtok(NULL, "-");
	strcpy(username, token);

	token = strtok(NULL, "-");
	strcpy(password, token);
}

void extract_username_passwd_newpasswd(char buffer[], char username[], char password[], char newpasswd[])
{
	char *token;

	token = strtok(buffer, "-");

	token = strtok(NULL, "-");
	strcpy(username, token);

	token = strtok(NULL, "-");
	strcpy(password, token);

	token = strtok(NULL, "-");
	strcpy(newpasswd, token);
}