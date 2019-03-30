#include "../include/cmd.h"

int resolve_ls(char*** result, int *n, const char* path, MYSQL* conn, int dir_id)
{
    int i;
    MYSQL_RES* res;
    MYSQL_ROW row;
    if (path == NULL)   //only ls
    {
        res = sql_select_by_dirid(conn, dir_id);
        if (res == NULL)
        {
            return -1;
        }
    }
    else    //ls [absolute path]
    {
        res = sql_select_by_path(conn, path);
        if (res == NULL)
        {
            return -1;
        }

        row = mysql_fetch_row(res);
        if (atoi(row[2]) == 0)  //is dir
        {
            mysql_free_result(res);
            res = sql_select_by_dirid(conn, atoi(row[0]));
        }
        else    //is file
        {
            *n = 1;
            *result = (char**)malloc(sizeof(char*));
            (*result)[0] = (char*)malloc(RESULT_LEN);
            sprintf((*result)[0], "%-30s%s", row[3], row[4]);
            mysql_free_result(res);
            return 1;
        }
    }
    *n = mysql_num_rows(res);
    *result = (char**)malloc(*n * sizeof(char*));
    for (i = 0; i < *n; i++)
    {
        (*result)[i] = (char*)malloc(RESULT_LEN);
        row = mysql_fetch_row(res);
        //whether file is dir
        if (atoi(row[2]) == 0)
        {
            sprintf((*result)[i], "%-30s%s", row[3], "dir");
        }
        else
        {
            sprintf((*result)[i], "%-30s%s", row[3], row[4]);
        }
    }
    mysql_free_result(res);
    return 1;
}

int resolve_pwd(char*** result, int *n, MYSQL* conn, int dir_id)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    res = sql_select_by_id(conn, dir_id);
    row = mysql_fetch_row(res);
    *n = 1;
    *result = (char**)malloc(sizeof(char*));
    (*result)[0] = (char*)malloc(RESULT_LEN);
    strcpy((*result)[0], row[5]);
    mysql_free_result(res);
    return 1;
}

/*return:
 *  1 for normal cmd
 *  -1 for ls error*/
int cmd_interpret(char*** result, int *n,  const char* cmd, MYSQL* conn, int dir_id)
{
    int i = 0, j, k;
    char prefix[10];
    char *path;
    while (cmd[i] != ' ' && cmd[i] != '\0')
    {
        i++;
    }
    if (cmd[i] == ' ')
    {
        strncpy(prefix, cmd, i);
        prefix[i] = '\0';
        j = i;
        while (cmd[j] != '\0')
        {
            j++;
        }
        int len = j - i;
        path = (char*)malloc(len);
        for (k = 0; k < len; k++, i++)
        {
            path[k] = cmd[i + 1];
        }
    }
    else
    {
        strcpy(prefix, cmd);
        path = NULL;
    }
#ifdef _DEBUG
    printf("prefix: %s\n", prefix);
    printf("cmd_path: %s\n", path);
#endif

    int ret;
    //only support current dir and absolute path;
    if (strcmp(prefix, "ls") == 0)
    {
       ret = resolve_ls(result, n, path, conn, dir_id);
       return ret;
    }

    if (strcmp(prefix, "pwd") == 0)
    {
       ret = resolve_pwd(result, n, conn, dir_id);
       return ret;
    }
}

