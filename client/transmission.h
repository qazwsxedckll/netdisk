#ifndef __TRANSMISSION_H__
#define __TRANSMISSION_H__
#include "head.h"

typedef struct
{
    int data_len;
    char buf[CMD_LEN];
}DataPackage;

int send_cycle(int fd, char* data, int send_len);

int recv_cycle(int fd, char* data, int recv_len);

int tran_cmd(int fd, DataPackage* data);

int tran_authen(int socketFd, char* user_name, DataPackage* data, int err);

void print_help();
#endif
