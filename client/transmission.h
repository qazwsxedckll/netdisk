#ifndef __TRANSMISSION_H__
#define __TRANSMISSION_H__
#include "head.h"
#include "client.h"
#include "md5.h"

int tran_cmd(int fd, DataPackage* data);

int user_signup(int* socketFd, const char* ip, const char* port, char* user_name, DataPackage* data);

int tran_authen(int* socketFd, const char* ip, const char* port, char* user_name, DataPackage* data);

void* get_files(void* p);

void* put_files(void* p);

#endif
