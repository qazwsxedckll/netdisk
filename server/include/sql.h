#ifndef __SQL_H__
#define __SQL_H__
#include "head.h"

#define QUERY_LEN 1000

int sql_connect(MYSQL** conn);

MYSQL_RES* sql_select(MYSQL* conn, const char* table, const char* field, const char* condition, int reg_flag);

int sql_insert_file_trans(MYSQL* conn, const char* user_name, const char* dir_id, int type, const char* file_name,
                        int file_size, const char* file_md5);

int sql_insert_user_trans(MYSQL* conn, const char* user_name, const char* password, const char* dir_id,
                          int type, const char* file_name, int file_size, const char* file_md5);

int sql_delete_file(MYSQL* conn, const char* user_name, const char* file_path);

int sql_delete_user(MYSQL* conn, const char* user_name);
#endif
