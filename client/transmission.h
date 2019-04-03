#ifndef __TRANSMISSION_H__
#define __TRANSMISSION_H__
#include "head.h"

typedef struct
{
    int data_len;
    char buf[CMD_LEN];
}DataPackage;

int tran_cmd(int fd, DataPackage* data);

int tran_authen(int* socketFd, const char* ip, const char* port, char* user_name, DataPackage* data, int err);

int cmd_interpret(const DataPackage* data);

void* get_files(void* p);

void* put_files(void* p);

void print_help();
#endif
