#ifndef __CLIENT_H__
#define __CLIENT_H__
#include "head.h"

typedef struct
{
    int data_len;
    char buf[CMD_LEN];
}DataPackage;

int connect_server(int* socketFd, const char* ip, const char* port);

void print_help();

int cmd_interpret(const DataPackage* data);

int login_page(int flag);
#endif


