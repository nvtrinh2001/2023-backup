#include "accounts.h"

void logout(Account *account)
{
    if (!(*account)->is_login)
    {
        printf("[accounts.c]: Authentication failed! You need to login first!\n\n");
        return;
    }
    (*account)->is_login = 0;
    printf("[accounts.c]: Bye, have a good day! :)\n\n");
    return;
}

bool check_password(Account head, char *username, char *password)
{
    bool result = false;
    Account node = NULL;
    for (node = head; node != NULL; node = node->next)
    {
        if (strcmp(node->username, username) == 0)
        {
            if (strcmp(node->password, password) == 0)
            {
                result = true;
            }
            break;
        }
    }
    free(node);
    return result;
}

void init_accounts(Account *head)
{
    FILE *fp = fopen("account.txt", "r+");
    char username[50], password[50];
    int status;

    while (!feof(fp))
    {
        fscanf(fp, "%s ", username);
        fscanf(fp, "%s ", password);
        fscanf(fp, "%d\n", &status);
        add(head, username, password, status);
    }
    fclose(fp);
    printf("[accounts.c]: Account list initialized!\n\n");
    return;
}

void print_accounts(Account head, int is_login)
{
    if (!is_login)
    {
        printf("Authentication failed! You need to login first!\n");
        return;
    }

    print(head);
    return;
}

void search_account(Account *head, int is_login)
{
    if (!is_login)
    {
        printf("Authentication failed! You need to login first!\n");
        return;
    }
    char username[50];
    printf("Input a username: ");
    scanf("%s", username);

    Account node = search(*head, username);
    if (node == NULL)
    {
        printf("Account not exists!\n");
        return;
    }
    printf("Account:\n");
    printf("\tusername: %s\n", node->username);
    printf("\tstatus: %d\n", node->status);
    return;
}

Account login(Account *head, char *username, char *password)
{
    Account node = search(*head, username);
    if (node == NULL)
    {
        printf("[accounts.c]: Username does not exist!\n\n");
        return node;
    }

    if (!node->status)
    {
        printf("[accounts.c]: Account is blocked!\n\n");
        return node;
    }

    if (strcmp(node->password, password) == 0)
    {
        printf("[accounts.c]: Welcome back!\n\n");
        node->is_login = 1;
        node->wrong_count = 0;
        return node;
    }

    node->wrong_count++;
    if (node->wrong_count <= 3)
    {
        printf("[accounts.c]: Wrong password! Try again.\n\n");
        return node;
    }

    printf("[accounts.c]: Authentication failed! your account is blocked!\n\n");
    FILE *fp;
    fp = fopen("account.txt", "w+");
    for (Account temp = *head; temp != NULL; temp = temp->next)
    {
        if (strcmp(temp->username, username) == 0)
            temp->status = 0;

        fprintf(fp, "%s %s %d\n", temp->username, temp->password, temp->status);
    }
    fclose(fp);

    return node;
}

void change_password(Account *head, char *username, char *newpassword)
{
    FILE *fp;
    fp = fopen("account.txt", "w");
    ftruncate(fileno(fp), 0);
    Account node = NULL;
    for (node = *head; node != NULL; node = node->next)
    {
        if (strcmp(node->username, username) == 0)
        {
            fprintf(fp, "%s %s %d\n", username, newpassword, node->status);
        }
        else
        {
            fprintf(fp, "%s %s %d\n", node->username, node->password, node->status);
        }
    }
    free(node);
    fclose(fp);
}

void register_account(Account *head)
{
    char username[50];
    char password[50];

    printf("What's your new username: ");
    scanf("%s", username);
    printf("What's your password: ");
    scanf("%s", password);

    Account node = search(*head, username);
    if (node != NULL)
    {
        printf("Account already exists!\n");
        return;
    }

    add(head, username, password, 1);

    FILE *fp;
    fp = fopen("account.txt", "a");
    fprintf(fp, "%s %s %d\n", username, password, 1);
    fclose(fp);

    free(node);
    printf("New account registered!\n");
    return;
}
