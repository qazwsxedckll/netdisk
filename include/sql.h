#ifndef __SQL_H__
#define __SQL_H__
#include "head.h"

#define QUERY_LEN 1000

int sql_connect(MYSQL** conn);

MYSQL_RES* sql_select(MYSQL* conn, const char* field, const char* condition);
#endif
