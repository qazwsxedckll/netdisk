#ifndef __SQL_H__
#define __SQL_H__
#include "head.h"

int sql_connect(MYSQL** conn);

MYSQL_RES* sql_select(MYSQL* conn, int dir_id);

#endif
