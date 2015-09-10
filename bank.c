//
//  main.c
//  client
//
//  Created by Kirsten on 4/6/15.
//  Copyright (c) 2015 Kirsten Rauffer. All rights reserved.
//

#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<pthread.h>
#include	"bank.h"

Account accounts[20];
int position;
char *account_name; //name of account currently being served (if being served) or name of account to create
int current_account = -1;

int CheckAccountExists(char *account_name)
{
    int i;

    if(account_name == NULL) return -1;
    if(position == 0) return -1;

    for(i = 0; i < position; i++)
    {
        if(strcmp(accounts[i]->name, account_name) == 0) return i;
    }

    return -1;
}

int CreateAccount(char *account_name)
{
    if(position == 20) return 0;
    else if(account_name == NULL || account_name[0] == '\n') return -1;
    else if(CheckAccountExists(account_name) != -1) return -2;
    else
    {
        Account new = malloc(sizeof(Account));
        new->name = account_name;
        new->balance = 0;
        new->insessionflag = 0;
        accounts[position] = new;
        position++;
        return 1;
    }
}

Account GetAccount(char *accountname)
{
	int i;
	for(i=0; i<position; i++){
		if(strcmp(accounts[i]->name, accountname)==0){
			return accounts[i];
		}
	}
	return NULL;
}

char *GetSubstring(char *command)
{
	int i,
        pos = 0,
        space = 0;
    char *account = NULL;

    for(i = 0; i < strlen(command); i++)
    {
        if(space == 1 &&  pos < 100)
        {
            account[pos] = command[i];
            pos++;
        }
        else if(command[i] == ' ' && space == 0)
        {
        	if(strlen(command)-1 == i){
        		return account;
        	}else{
        		int j;
        		int nullstring = 1; // 0 if not a null string  1 if null string
        		for(j=i+1; j < strlen(command); j++){
        			if(command[j] != NULL){
        				nullstring = 0;
        				break;
        			}
        		}
        		if(nullstring == 1){
        			return account;
        		}
        	}
            space = 1;
            account = malloc(sizeof(char)*(strlen(command)-i));
        }
    }
    if(space != 0){
    	account[pos] = '\0';
    }
    return account; //ADD IN ERROR CHECKING FOR NULL STRING
}

int GetAmount(char *substring)
{
    int i, ret;

    if(substring == NULL)
    {
        return -1;
    }

    for(i = 0; i < strlen(substring); i++)
    {
        if((substring[i] < '0' || substring[i] > '9') && substring[i] != '\n')
        {
            return -1;
        }
    }

    ret = atoi(substring);

    return ret;
}


char* GetCommand(char* request)
{

    char *command = request,
         *string_amount,
         *menu = "create [accountname]\nserve [accountname]\ndeposit [amount]\nwithdraw [amount]\nquery\nend\nquit\n",
         *commands[7],
    	 *message = NULL;

    int amount;

    commands[0] = "create";
    commands[1] = "serve";
    commands[2] = "deposit";
    commands[3] = "withdraw";
    commands[4] = "query";
    commands[5] = "end";
    commands[6] = "quit";

  //  while(1)
    //{
        if(command[0] == 'c')
        {
        	char *tempc = (char*)malloc(sizeof(char)*7);
        	strncpy(tempc, command, 6);
            if(strcmp(tempc, commands[0]) == 0)
            {
            	free(tempc);
                if(current_account < 0)
                {
                    account_name = GetSubstring(command);
                    int test = CreateAccount(account_name);
                    if(test != 1)
                    {
                        char *buf;
                        size_t sz;
                        sz = snprintf(NULL, 0, "Error making the account: %d\n", test);
                        buf = (char *)malloc(sz + 1); /* make sure you check for != NULL in real code */
                        snprintf(buf, sz+1, "Error making the account: %d\n", test);
                        message = (char*)buf;
                    }else{
                        char *buf;
                        size_t sz;
                        sz = snprintf(NULL, 0, "Successfully created account: %s\n", account_name);
                        buf = (char *)malloc(sz + 1);
                        snprintf(buf, sz+1, "Successfully created account: %s\n", account_name);
                        message = buf;
                    }
                }
                else
                {
                    char out[]= "You must end your customer session before creating a new account\n";
                    message = out;
                }
            }
            else{
            	char out[] = "Please enter one of the following commands:\n";
            	message = out;
            }
        }
        else if(command[0] == 's')
        {
        	char *tempc = (char*)malloc(sizeof(char)*6);
        	strncpy(tempc, command, 5);
            if(strcmp(tempc, commands[1]) == 0)
            {
            	free(tempc);
                if(current_account < 0){
                    account_name = GetSubstring(command);

                    current_account = CheckAccountExists(account_name);

                    if(current_account > -1){
                        char *buf;
                        size_t sz;
                        sz = snprintf(NULL, 0, "Welcome, %s\n", account_name);
                        buf = (char *)malloc(sz + 1);
                        snprintf(buf, sz+1, "Welcome, %s\n", account_name);
                        message = buf;

                        Account curraccount= GetAccount(account_name);
                        curraccount->insessionflag = 1;
                    }
                    else
                    {
                        char out[]= "Account does not exist\n";
                        message = out;
                        current_account = -1;
                    }
                }
                else{
                	char out[]= "You can only be in one customer session at a time\n";
                	message = out;
                }
            }
            else{
            	char out[] = "Please enter one of the following commands:\n";
            	message = out;
            }

        }
        else if(command[0] == 'd')
        {
        	char *tempc = (char*)malloc(sizeof(char)*8);
        	strncpy(tempc, command, 7);
            if(strcmp(tempc, commands[2]) == 0)
            {
                if(current_account < 0){
                    char out[] = "You need to start a customer session to deposit\n";
                    message = out;
                }
                else
                {
                    string_amount = GetSubstring(command);
                    amount = GetAmount(string_amount);

                    if(amount < 0){
                        char out[]= "Please try the command again, but enter a positive integer amount to deposit\n";
                        message = out;
                    }
                    else{
                    	accounts[current_account]->balance += amount;
                        char *buf;
                        size_t sz;
                        sz = snprintf(NULL, 0, "Successfully deposited: $%d\n", amount);
                        buf = (char *)malloc(sz + 1);
                        snprintf(buf, sz+1, "Successfully deposited: $%d\n", amount);
                        message = buf;
                    }
                }
            }
            else{
            	char out[] = "Please enter one of the following commands:\n";
            	message = out;
            }
        }
        else if(command[0] == 'w')
        {
        	char *tempc = (char*)malloc(sizeof(char)*9);
        	strncpy(tempc, command, 8);
            if(strcmp(tempc, commands[3]) == 0)
            {
                if(current_account < 0){
                	char out[]= "You need to start a customer session to withdraw\n";
                	message = out;
                }
                else
                {
                    string_amount = GetSubstring(command);
                    amount = GetAmount(string_amount);
                    if(amount < 0){
                    	char out[]= "Please try the command again, but enter a positive integer amount to deposit\n";
                    	message = out;
                    }
                    else{
                    	accounts[current_account]->balance -= amount;
                        char *buf;
                        size_t sz;
                        sz = snprintf(NULL, 0, "Successfully withdrew: $%d\n", amount);
                        buf = (char *)malloc(sz + 1);
                        snprintf(buf, sz+1, "Successfully withdrew: $%d\n", amount);
                        message = buf;
                    }
                }
            }
            else{
            	char out[] = "Please enter one of the following commands:\n";
            	message = out;
            }
        }
        else if(command[0] == 'e')
        {
            if(strcmp(command, commands[5]) == 0){
                if(current_account < 0){
                	char out[] = "You need to start a customer session to end a customer session\n";
                	message = out;
                }else{
                	current_account = -1;
                	char out[] = "Customer session has been ended\n";
                	message = out;
                }
            }
            else{
            	char out[] = "Please enter one of the following commands:\n";
            	message = out;
            }
        }
        else if(command[0] == 'q')
        {
            if(strcmp(command, commands[6]) == 0){
            	char out[] = "quit";
            	message = out;
            }
            else if(strcmp(command, commands[4]) == 0)
            {
                if(current_account < 0){
                	char out[] = "You need to start a customer session to place a query\n";
                	message = out;
                }
                else{
                    char *buf;
                    size_t sz;
                    sz = snprintf(NULL, 0, "Current balance: $%d\n",accounts[current_account]->balance);
                    buf = (char *)malloc(sz + 1);
                    snprintf(buf, sz+1, "Current balance: $%d\n",accounts[current_account]->balance);
                    message = buf;
                }
            }
            else{
            	char out[] = "Please enter one of the following commands:\n";
            	message = out;
            }
        }
        else{
        	char out[] = "Please enter one of the following commands:\n";
        	message = out;
        }
  //  }
    return message;
}
