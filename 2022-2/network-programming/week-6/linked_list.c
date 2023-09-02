#include "accounts.h"

void print(Account head)
{
    Account node = NULL;
    for (node = head; node != NULL; node = node->next)
    {
        printf("Username: %s\n", node->username);
        printf("Password: %s\n", node->password);
        printf("Status: %d\n", node->status);
        printf("Wrong Authentication Count: %d\n\n", node->wrong_count);
    }
    free(node);
    return;
}

void add(Account *head, char *username, char *password, int status)
{
    Account node = (Account)malloc(sizeof(struct Node));
    strcpy(node->username, username);
    strcpy(node->password, password);
    node->status = status;
    node->wrong_count = 0;
    node->is_login = 0;
    node->next = *head;
    *head = node;
    return;
}

Account search(Account head, char *username)
{
    Account node = NULL;
    for (node = head; node != NULL; node = node->next)
    {
        if (strcmp(node->username, username) == 0)
            return node;
    }
    free(node);
    return NULL;
}
