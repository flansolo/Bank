#ifndef BANK_H
#define BANK_H

#include <stdlib.h>

struct account
{
    char *name;
    int balance;
    int insessionflag; //0 for not in-session  1 for in-session
};
typedef struct account* Account;

//extern Account accounts[20];
//extern int position;

int CheckAccountExists(char *account_name);
Account GetAccount(char *accountname);
int CreateAccount(char *account_name);
char *GetSubstring(char *command);
int GetAmount(char *substring);
char* GetCommand(char* request);

#endif
