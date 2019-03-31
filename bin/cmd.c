#include "../include/cmd.h"

int resolve_ls(char*** result, int *n, const char* path, MYSQL* conn, const char* cur_dir_id, const char* root_id)
{
    int i;
    char abs_path[RESULT_LEN];
    MYSQL_RES* res;
    MYSQL_ROW row;
    if (path == NULL)   //only ls
    {
        res = sql_select(conn, "dir_id", cur_dir_id);
        if (res == NULL)
        {
            *n = 0;
            return 1;
        }
    }
    else        //ls [FILE]
    {
        if (path[0] == '/')         //start with user root dir
        {
            res = sql_select(conn, "id", root_id);
            row = mysql_fetch_row(res);
            mysql_free_result(res);
            if (strcmp(path, "/") == 0)
            {
                strcpy(abs_path, row[5]);
            }
            else
            {
                strcpy(abs_path, row[5]);
                strcat(abs_path, path);
            }
        }
        else        //start with cur dir
        {
            res = sql_select(conn, "id", cur_dir_id);
            row = mysql_fetch_row(res);
            mysql_free_result(res);
            sprintf(abs_path, "%s/%s", row[5], path);
        }
        res = sql_select(conn,"file_path", abs_path);
        if (res == NULL)
        {
            return -1;
        }

        row = mysql_fetch_row(res);
        if (atoi(row[2]) == 0)  //is dir
        {
            mysql_free_result(res);
            res = sql_select(conn, "dir_id", row[0]);
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

int resolve_pwd(char*** result, int *n, MYSQL* conn, const char* cur_dir_id)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    res = sql_select(conn, "id",  cur_dir_id);
    row = mysql_fetch_row(res);
    *n = 1;
    *result = (char**)malloc(sizeof(char*));
    (*result)[0] = (char*)malloc(RESULT_LEN);
    strcpy((*result)[0], row[5]);
    mysql_free_result(res);
    return 1;
}

int resolve_cd(char*** result, int *n, const char* path, MYSQL* conn, char* cur_dir_id, const char* root_id)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    char abs_path[RESULT_LEN];
    if (strcmp(path, "..") == 0)
    {
        res = sql_select(conn, "id", cur_dir_id);   //get row of current dir
        row = mysql_fetch_row(res);
        mysql_free_result(res);
        if (atoi(row[1]) == 5)     //curent dir is user root dir
        {
            return -2;
        }
        else
        {
            res = sql_select(conn, "id", row[1]);   //get row of parent dir
            row = mysql_fetch_row(res);
            strcpy(cur_dir_id, row[0]);
            *n = 1;
            *result = (char**)malloc(sizeof(char*));
            (*result)[0] = (char*)malloc(RESULT_LEN);
            strcpy((*result)[0], row[5]);
            mysql_free_result(res);
            return 2;
        }
    }
    else
    {
        if (path[0] == '/')         //start with user root dir
        {
            res = sql_select(conn, "id", root_id);
            row = mysql_fetch_row(res);
            mysql_free_result(res);
            if (strcmp(path, "/") == 0)
            {
                strcpy(abs_path, row[5]);
            }
            else
            {
                strcpy(abs_path, row[5]);
                strcat(abs_path, path);
            }
        }
        else        //start with cur dir
        {
            res = sql_select(conn, "id", cur_dir_id);
            row = mysql_fetch_row(res);
            mysql_free_result(res);
            sprintf(abs_path, "%s/%s", row[5], path);
        }

        res = sql_select(conn, "file_path", abs_path);
        if (res == NULL)
        {
            return -2;
        }

        row = mysql_fetch_row(res);
        mysql_free_result(res);
        if (atoi(row[2]) == 0)      //is dir
        {
            strcpy(cur_dir_id, row[0]);
            *n = 1;
            *result = (char**)malloc(sizeof(char*));
            (*result)[0] = (char*)malloc(RESULT_LEN);
            strcpy((*result)[0], row[5]);
            return 2;
        }
        else    //is file
        {
            return -2;
        }
    }
}

/*return:
 *  1 for normal cmd
 *  2 for cd result
 *  -1 for ls error
 *  -2 for cd error*/
int cmd_interpret(char*** result, int *n, MYSQL* conn, const char* cmd, char* cur_dir_id, const char* root_id)
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
        ret = resolve_ls(result, n, path, conn, cur_dir_id, root_id);
        return ret;
    }
    else if (strcmp(prefix, "pwd") == 0)
    {
        ret = resolve_pwd(result, n, conn, cur_dir_id);
        return ret;
    }
    else if(strcmp(prefix, "cd") == 0)
    {
        if (path == NULL)
        {
            return -2;
        }
        ret = resolve_cd(result, n, path, conn, cur_dir_id, root_id);
        return ret;
    }
}

