#ifndef __USER_H__
#define __USER_H__
#include "head.h"

#define USER_NAME_LEN 20

typedef struct
{
    int fd;
    char user_name[USERNAME_LENGTH + 1];
    char cur_dir_id[RESULT_LEN];
    char root_id[RESULT_LEN];
}User, *Users;

#endif
