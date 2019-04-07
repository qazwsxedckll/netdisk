#ifndef __TRANSMISSION_H__
#define __TRANSMISSION_H__
#include "head.h"
#include "md5.h"
#include "sql.h"
#include "crypto.h"

typedef struct
{
    int data_len;
    char buf[RESULT_LEN];
}DataPackage;

int send_cycle(int fd, const char* data, int send_len);

int recv_cycle(int fd, char* data, int recv_len);

int recv_nonce(int fd, DataPackage* data, char* user_name);

int send_file(int client_fd, const char* file_name, const char* file_md5, const char* file_size);

int recv_file(int client_fd, const char* user_name, const char* cur_dir_id);

#endif
