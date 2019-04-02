#ifndef __USER_H__
#define __USER_H__
#include "head.h"
#include "sql.h"

#define USER_NAME_LEN 20
#define USER_PWD_LEN 20

typedef struct
{
    int fd;
    char user_name[USER_NAME_LEN + 1];
    char cur_dir_id[RESULT_LEN];
    char root_id[RESULT_LEN];
    char token[TOKEN_LEN];
}User, *Users;

int user_verify(MYSQL* conn, const char* user_name, const char* password);

char* user_find_root(MYSQL* conn, const char* user_name);
#endif
