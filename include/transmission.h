#ifndef __TRANSMISSION_H__
#define __TRANSMISSION_H__
#include "head.h"

typedef struct
{
    int data_len;
    char buf[1000];
}DataPackage;

int send_cycle(int, char*, int);

int recv_cycle(int ,char*, int);

#endif
