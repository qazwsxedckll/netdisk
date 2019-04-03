#include "../include/cmd.h"

char* convert_path(const char* path, MYSQL* conn, const char* root_id, const char* cur_dir_id)
{
    char* abs_path = (char*)malloc(RESULT_LEN);
    MYSQL_RES* res;
    MYSQL_ROW row;
    if (path[0] == '/')         //strat with user root dir
    {
        res = sql_select(conn, "file", "id", root_id);
        row = mysql_fetch_row(res);
        mysql_free_result(res);
        if (strcmp(path, "/") == 0)
        {
            strcpy(abs_path, row[5]);
            return abs_path;
        }
        else
        {
            strcpy(abs_path, row[5]);
            strcat(abs_path, path);
            return abs_path;
        }
    }
    else        //start with cur dir
    {
        res = sql_select(conn, "file", "id", cur_dir_id);
        row = mysql_fetch_row(res);
        mysql_free_result(res);
        sprintf(abs_path, "%s/%s", row[5], path);
        return abs_path;
    }
}

int resolve_ls(char*** result, int *n, const char* path, MYSQL* conn, const char* cur_dir_id, const char* root_id)
{
    int i;
    char* abs_path;
    MYSQL_RES* res;
    MYSQL_ROW row;
    if (strlen(path) == 0)   //only ls
    {
        res = sql_select(conn, "file", "dir_id", cur_dir_id);
        if (res == NULL)
        {
            *n = 0;
            *result = NULL;
            return 1;
        }
    }
    else        //ls [FILE]
    {
        abs_path = convert_path(path, conn, root_id, cur_dir_id);       //free after use
        res = sql_select(conn, "file","file_path", abs_path);
        free(abs_path);
        abs_path = NULL;
        if (res == NULL)
        {
            return -1;
        }

        row = mysql_fetch_row(res);
        if (atoi(row[2]) == 0)  //is dir
        {
            mysql_free_result(res);
            res = sql_select(conn, "file", "dir_id", row[0]);
            if (res == NULL)        //empty dir
            {
                *n = 0;
                *result = NULL;
                return 1;
            }
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
    res = sql_select(conn, "file", "id",  cur_dir_id);
    row = mysql_fetch_row(res);
    *n = 1;
    *result = (char**)malloc(sizeof(char*));
    (*result)[0] = (char*)malloc(RESULT_LEN);
    strcpy((*result)[0], row[5]);
    mysql_free_result(res);
    return 1;
}

int resolve_cd(char*** result, int *n, const char* cmd_path, MYSQL* conn, char* cur_dir_id, const char* root_id)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    char* abs_path;
    if (strcmp(cmd_path, "..") == 0)
    {
        res = sql_select(conn, "file", "id", cur_dir_id);   //get row of current dir
        row = mysql_fetch_row(res);
        mysql_free_result(res);
        if (atoi(row[1]) == 5)     //curent dir is user root dir
        {
            return -2;
        }
        else
        {
            res = sql_select(conn, "file", "id", row[1]);   //get row of parent dir
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
        abs_path = convert_path(cmd_path, conn, root_id, cur_dir_id);
        res = sql_select(conn, "file", "file_path", abs_path);
        free(abs_path);
        abs_path = NULL;
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

int resolve_gets(char* file_md5, char* file_name, char* file_size, const char* path, MYSQL* conn, const char* root_id, const char* cur_dir_id)
{
    char* abs_path;
    MYSQL_RES* res;
    MYSQL_ROW row;

    abs_path = convert_path(path, conn, root_id, cur_dir_id);
    res = sql_select(conn, "file", "file_path", abs_path);
    free(abs_path);
    abs_path = NULL;
    if (res == NULL)
    {
        return -1;
    }

    row = mysql_fetch_row(res);
    mysql_free_result(res);
    if (atoi(row[2]) == 0)          //is dir
    {
        //to be complete
        return -1;
    }
    else        //is file
    {
        strcpy(file_size, row[4]);
        strcpy(file_name, row[3]);
        strcpy(file_md5, row[6]);
        return 1;
    }
}

int resolve_puts(const char* cmd_path, MYSQL* conn, const char* root_id, const char* cur_dir_id)
{
    MYSQL_RES* res;
    char* abs_path;
    char file_name[FILE_NAME_LEN];

    int len = strlen(cmd_path);
    while (cmd_path[len] != '/' && len != -1)
    {
        len--;
    }
    len++;
    int i;
    while (cmd_path[len] != '\0')
    {
        file_name[i++] = cmd_path[len++];
    }
    file_name[i] = '\0';

    abs_path = convert_path(file_name, conn, root_id, cur_dir_id);
    res = sql_select(conn, "file", "file_path", abs_path);
    free(abs_path);
    abs_path = NULL;
    if (res == NULL)
    {
        return 0;
    }
    else
    {
        mysql_free_result(res);
#ifdef _DEBUG
        printf("insert file failed: file exist\n");
#endif
        return -1;          //file exist
    }
}

void cmd_interpret(const char*cmd, char* prefix, char* cmd_path)
{
    int i = 0, j, k;
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
        for (k = 0; k < len; k++, i++)
        {
            cmd_path[k] = cmd[i + 1];
        }
    }
    else
    {
        strcpy(prefix, cmd);
        cmd_path[0] = '\0';
    }
#ifdef _DEBUG
    printf("prefix: %s\n", prefix);
    printf("cmd_path: %s\n", cmd_path);
#endif
}
