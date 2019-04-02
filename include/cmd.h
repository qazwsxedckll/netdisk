#ifndef __CMD_H__
#define __CMD_H__
#include "head.h"
#include "sql.h"

void cmd_interpret(const char*cmd, char* prefix, char* cmd_path); 

int resolve_ls(char*** result, int *n, const char* cmd_path, MYSQL* conn, const char* cur_dir_id, const char* root_id);

int resolve_pwd(char*** result, int *n, MYSQL* conn, const char* cur_dir_id);

int resolve_cd(char*** result, int *n, const char* cmd_path, MYSQL* conn, char* cur_dir_id, const char* root_id);

int resolve_gets(const char* path, MYSQL* conn, const char* root_id, const char* cur_dir_id);

#endif

