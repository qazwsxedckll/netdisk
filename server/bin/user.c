#include "../include/user.h"

int user_verify(MYSQL* conn, const char* user_name, const char* password)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    res = sql_select(conn, "user", "user_name", user_name, 0);
    if (res == NULL)
    {
#ifdef _DEBUG
        printf("cannot find user %s\n", user_name);
#endif
        return -1;
    }
    mysql_free_result(res);
    row = mysql_fetch_row(res);
    unsigned char md[SHA512_DIGEST_LENGTH];
    SHA512((unsigned char*)password, strlen(password), md);
    char sha_password[SHA512_DIGEST_LENGTH * 2 + 1] = { 0 };
    char tmp[3] = { 0 };
    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
    {
        sprintf(tmp, "%02x", md[i]);
        strcat(sha_password, tmp);
    }
    if (strcmp(sha_password, row[2]) == 0)
    {
#ifdef _DEBUG
        printf("verification success\n");
#endif
        return 0;
    }
    else
    {
#ifdef _DEBUG
        printf("verification failed\n");
#endif
        return -1;
    }
}

char* user_find_root(MYSQL* conn, const char* user_name)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    char path[1000] = "/netdisk/";
    strcat(path, user_name);
    res = sql_select(conn, "file", "file_path", path, 0);
    char* root_dir;
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    root_dir = (char*)malloc(strlen(row[0]) + 1);
    strcpy(root_dir, row[0]);
    return root_dir;
}
