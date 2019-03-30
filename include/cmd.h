#ifndef __CMD_H__
#define __CMD_H__
#include "head.h"
#include "sql.h"

int cmd_interpret(char*** result, int* n, const char* cmd, MYSQL* conn, int dir_id);

#endif

