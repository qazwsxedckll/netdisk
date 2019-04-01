#include "../include/user.h"

int user_verify(MYSQL* conn, const char* user_name, const char* password)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    res = sql_select(conn, "user", "user_name", user_name);
    if (mysql_num_rows(res) == 0)
    {
        mysql_free_result(res);
        return -1;
    }
    mysql_free_result(res);
    row = mysql_fetch_row(res);
    if (strcmp(password, row[2]) == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

char* user_find_root(MYSQL* conn, const char* user_name)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    char path[1000] = "/netdisk/";
    strcat(path, user_name);
    res = sql_select(conn, "file", "file_path", path);
    char* root_dir;
    if (mysql_num_rows(res) == 0)
    {
        //create root dir
    }
    else
    {
        row = mysql_fetch_row(res);
        root_dir = (char*)malloc(strlen(row[0]) + 1);
        strcpy(root_dir, row[0]);
        return root_dir;
    }
}
