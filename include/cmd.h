#ifndef __CMD_H__
#define __CMD_H__
#include "head.h"
#include "sql.h"

int cmd_interpret(char*** result, int* n, MYSQL* conn, const char* cmd, char* dir_id, const char* root_id);

#endif

