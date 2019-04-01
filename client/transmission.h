#ifndef __TRANSMISSION_H__
#define __TRANSMISSION_H__
#include "head.h"

typedef struct
{
    int data_len;
    char buf[CMD_LEN];
}DataPackage;

int send_cycle(int fd, char* data, int send_len);

int recv_cycle(int fd, char* data, int send_len);

int tran_cmd(int, DataPackage*);

void print_help();
#endif
