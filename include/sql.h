#ifndef __SQL_H__
#define __SQL_H__
#include "head.h"

#define QUERY_LEN 1000

int sql_connect(MYSQL** conn);

MYSQL_RES* sql_select_by_dirid(MYSQL* conn, int dir_id);

MYSQL_RES* sql_select_by_path(MYSQL* conn, char* file_path);

#endif
