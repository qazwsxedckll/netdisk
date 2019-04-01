#ifndef __TRANSMISSION_H__
#define __TRANSMISSION_H__
#include "head.h"

typedef struct
{
    int data_len;
    char buf[RESULT_LEN];
}DataPackage;

int send_cycle(int fd, char* data, int send_len);

int recv_cycle(int fd, char* data, int recv_len);
#endif
