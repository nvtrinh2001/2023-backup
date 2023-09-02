#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

typedef struct Node
{
    char username[50];
    char password[50];
    int status;
    int wrong_count;
    int is_login;
    struct Node *next;
} *Account;

bool check_password(Account head, char *username, char *password);
void change_password(Account *head, char *username, char *newpassword);
void register_account(Account *head);
Account login(Account *head, char *username, char *password);
void search_account(Account *head, int is_login);
void init_accounts(Account *head);
void logout(Account *account);
void print_accounts(Account head, int is_login);
void change_password(Account *head, char *username, char *newpassword);
void print(Account head);
void add(Account *head, char *username, char *password, int status);
Account search(Account head, char *username);
