#include "../include/cmd.h"

void get_file_name(char* file_name, const char* cmd_path)
{
    int len = strlen(cmd_path);
    while (cmd_path[len] != '/' && len != -1)
    {
        len--;
    }
    len++;
    int i = 0;
    while (cmd_path[len] != '\0')
    {
        file_name[i++] = cmd_path[len++];
    }
    file_name[i] = '\0';
}

char* convert_path(const char* path, MYSQL* conn, const char* root_id, const char* cur_dir_id)
{
    char* abs_path = (char*)malloc(RESULT_LEN);
    MYSQL_RES* res;
    MYSQL_ROW row;
    if (path[0] == '/')         //strat with user root dir
    {
        res = sql_select(conn, "file", "id", root_id, 0);
        row = mysql_fetch_row(res);
        mysql_free_result(res);
        strcpy(abs_path, row[5]);
        if (strcmp(path, "/") == 0)
        {
            return abs_path;
        }
        strcat(abs_path, path);
        return abs_path;
    }
    if (path[0] == '.' && path[1] == '.')       //start with parent dir
    {
        res = sql_select(conn, "file", "id", cur_dir_id, 0);
        if (res == NULL)
        {
            return NULL;
        }
        row = mysql_fetch_row(res);
        mysql_free_result(res);
        res = sql_select(conn, "file", "id", row[1], 0);
        if (res == NULL)
        {
            return NULL;
        }
        row = mysql_fetch_row(res);
        mysql_free_result(res);
        if (atoi(row[1]) == -1)
        {
            free(abs_path);
            return NULL;
        }
        strcpy(abs_path, row[5]);
        if (strcmp(path, "..") == 0 || strcmp(path, "../") == 0)
        {
            return abs_path;
        }
        if (path[2] == '/')
        {
            char new_path[RESULT_LEN];
            int len = strlen(path);
            for (int i = 0; i + 3 <= len; i++)
            {
                new_path[i] = path[i + 3];
            }
            convert_path(new_path, conn, root_id, row[0]);
        }
    }
    else        //start with cur dir
    {
        res = sql_select(conn, "file", "id", cur_dir_id, 0);
        if (res == NULL)
        {
            return NULL;
        }
        row = mysql_fetch_row(res);
        mysql_free_result(res);
        if (strcmp(path, "./") == 0 || strcmp(path, ".") == 0)
        {
            strcpy(abs_path, row[5]);
            return abs_path;
        }
        if (path[0] == '.' && path[1] == '/')
        {
            char new_path[RESULT_LEN];
            int len = strlen(path);
            for (int i = 0; i + 2 <= len; i++)
            {
                new_path[i] = path[i + 2];
            }
            sprintf(abs_path, "%s/%s", row[5], new_path);
            return abs_path;
        }
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
        res = sql_select(conn, "file", "dir_id", cur_dir_id, 0);
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
        if (abs_path == NULL)
        {
            return -1;
        }
        res = sql_select(conn, "file","file_path", abs_path, 0);
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
            res = sql_select(conn, "file", "dir_id", row[0], 0);
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
            sprintf((*result)[i], "%-20s%-20s%-20s", row[3], "dir", row[7]);
        }
        else
        {
            sprintf((*result)[i], "%-20s%-20s%-20s", row[3], row[4], row[7]);
        }
    }
    mysql_free_result(res);
    return 1;
}

int resolve_pwd(char*** result, int *n, MYSQL* conn, const char* cur_dir_id, int name_len)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    res = sql_select(conn, "file", "id",  cur_dir_id, 0);
    row = mysql_fetch_row(res);
    *n = 1;
    *result = (char**)malloc(sizeof(char*));
    (*result)[0] = (char*)malloc(RESULT_LEN);
    if (row[5][8 + name_len + 1] == '\0')
    {
        mysql_free_result(res);
        strcpy((*result)[0], "/");
        return 1;
    }
    else
    {
        for (int i = 8 + name_len + 1; row[5][i - 1] != '\0'; i++)
        {
            (*result)[0][i - 8 - name_len - 1] = row[5][i];
        }
        mysql_free_result(res);
    return 1;
    }
}

int resolve_cd(char*** result, int *n, const char* cmd_path, MYSQL* conn, char* cur_dir_id, const char* root_id)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    char* abs_path;
    abs_path = convert_path(cmd_path, conn, root_id, cur_dir_id);
    if (abs_path == NULL)
    {
        return -2;
    }
    res = sql_select(conn, "file", "file_path", abs_path, 0);
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
        int ret;
        ret = resolve_ls(result, n, "", conn, cur_dir_id, root_id);
        return ret;
    }
    else    //is file
    {
        return -2;
    }
}

int resolve_mkdir(char*** result, int *n, const char* user_name, const char* cmd_path, MYSQL* conn, const char* cur_dir_id, const char* root_id)
{
    int ret;
    MYSQL_RES* res;

    char file_name[FILE_NAME_LEN];
    get_file_name(file_name, cmd_path);

    char* abs_path;
    abs_path = convert_path(cmd_path, conn, root_id, cur_dir_id);
    if (abs_path == NULL)
    {
        return -4;
    }
    res = sql_select(conn, "file", "file_path", abs_path, 0);
    free(abs_path);
    abs_path = NULL;
    if (res == NULL)
    {
        ret = sql_insert_file_trans(conn, user_name, cur_dir_id, 0, file_name, 0, NULL);
        if (ret == -1)
        {
            return -4;
        }
        ret = resolve_ls(result, n, "", conn, cur_dir_id, root_id);
        return ret;
    }
    else
    {
        mysql_free_result(res);
        return -4;
    }
}

int resolve_gets(char* file_md5, char* file_name, char* file_size, const char* path, MYSQL* conn, const char* cur_dir_id, const char* root_id)
{
    char* abs_path;
    MYSQL_RES* res;
    MYSQL_ROW row;

    abs_path = convert_path(path, conn, root_id, cur_dir_id);
    if (abs_path == NULL)
    {
#ifdef _DEBUG
        printf("gets: cannot get: No such file or directory\n");
#endif
        return -1;
    }
    res = sql_select(conn, "file", "file_path", abs_path, 0);
    free(abs_path);
    abs_path = NULL;
    if (res == NULL)
    {
#ifdef _DEBUG
        printf("gets: cannot get: No such file or directory\n");
#endif
        return -1;
    }

    row = mysql_fetch_row(res);
    mysql_free_result(res);
    if (atoi(row[2]) == 0)          //is dir
    {
        strcpy(file_name, row[3]);
        strcpy(file_size, cur_dir_id); //send cur_dir_id via file_size
        strcpy(file_md5, "0");
        return 0;
    }
    else        //is file
    {
        strcpy(file_size, row[4]);
        strcpy(file_name, row[3]);
        strcpy(file_md5, row[6]);
        return 0;
    }
}

int resolve_puts(const char* cmd_path, MYSQL* conn, const char* root_id, const char* cur_dir_id)
{
    MYSQL_RES* res;
    char* abs_path;

    char file_name[FILE_NAME_LEN];
    get_file_name(file_name, cmd_path);

    abs_path = convert_path(file_name, conn, root_id, cur_dir_id);
    if (abs_path == NULL)
    {
#ifdef _DEBUG
        printf("puts: cannot put: root dir\n");
#endif
        return -1;
    }
    res = sql_select(conn, "file", "file_path", abs_path, 0);
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

int resolve_rm(char*** result, int *n, const char* cmd_path, MYSQL* conn, const char* user_name, const char* root_id, const char* cur_dir_id)
{
    char* abs_path;
    int ret, num;
    MYSQL_RES* res;
    MYSQL_RES* md5_res;
    MYSQL_ROW row;
    
    abs_path = convert_path(cmd_path, conn, root_id, cur_dir_id);
    if (abs_path == NULL)
    {
        return -3;
    }

    char regexp[QUERY_LEN] = "^";
    strcat(regexp, abs_path);
    free(abs_path);
    abs_path = NULL;
    res = sql_select(conn, "file", "file_path", regexp, 1);
    if (res == NULL)
    {
        return -3;
    }
    *n = mysql_num_rows(res);
    *result = (char**)malloc(*n * sizeof(char*));
    for (int i = 0; i < *n; i++)
    {
        (*result)[i] = (char*)malloc(RESULT_LEN);
        row = mysql_fetch_row(res);
        if (atoi(row[2]) == 0)
        {
            ret = sql_delete_file(conn, user_name, row[5]);
            if (ret == -1)
            {
                return -3;
            }
            sprintf((*result)[i], "%s is removed", row[3]);
        }
        else
        {
            char file_md5[MD5_LEN];
            strcpy(file_md5, row[6]);
            md5_res = sql_select(conn, "file", "file_md5", file_md5, 0);
            num = mysql_num_rows(md5_res);
            mysql_free_result(md5_res);

            ret = sql_delete_file(conn, user_name, row[5]);
            if (ret == -1)
            {
                return -3;
            }
            sprintf((*result)[i], "%s is removed", row[3]);

            if(num == 1)       //last file
            {
                char path_name[RESULT_LEN] = "netdisk/";
                strcat(path_name, file_md5);
                ret = remove(path_name);
                if (ret == -1)
                {
                    return -3;
                }
#ifdef _DEBUG
                printf("%s is removed from disk\n", path_name);
#endif
            }
        }
    }
    mysql_free_result(res);

    //delete account
    if (strcmp(cmd_path, "/") == 0 || strcmp(cmd_path, "./") == 0)
    {
        char pk_path[RESULT_LEN];
        sprintf(pk_path, "keys/%s_%s.key", user_name, "pub");
        ret = remove(pk_path);
        if (ret)
        {
            return -3;
        }
        ret = sql_delete_user(conn, user_name);
        if (ret)
        {
            return -3;
        }
        else
        {
            return -10;
        }
    }
    return 1;
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
