#include "../include/sql.h"
#include "../include/config.h"

int sql_connect(MYSQL** conn)
{
    Config configs[50];
    int n = read_conf("../conf/sql.conf", configs);
    char server[CONFIG_LEN];
    char user[CONFIG_LEN];
    char password[CONFIG_LEN];
    char database[CONFIG_LEN];
    get_conf_value(configs, n, "server", server);
    get_conf_value(configs, n, "user", user);
    get_conf_value(configs, n, "password", password);
    get_conf_value(configs, n, "database", database);
    *conn = mysql_init(NULL);
    if (!mysql_real_connect(*conn, server, user, password, database, 0, NULL, 0))
    {
        printf("Error connecting to databse: %s\n", mysql_error(*conn));
        return -1;
    }
    else
    {
#ifdef _DEBUG
        printf("database connected\n");
#endif
        return 0;
    }
}

MYSQL_RES* sql_select(MYSQL* conn, const char* table, const char* field, const char* condition, int reg_flag)
{
    MYSQL_RES* res = NULL;
    char query[QUERY_LEN];
    if (reg_flag == 1)
    {
        sprintf(query, "SELECT * FROM %s WHERE %s REGEXP '%s'", table, field, condition);
    }
    else
    {
        sprintf(query, "SELECT * FROM %s WHERE %s = '%s'", table, field, condition);
    }
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    int ret = mysql_query(conn, query);
    if (ret)
    {
#ifdef _DEBUG
        printf("Error making query: %s\n", mysql_error(conn));
#endif
        return NULL;
    }
    else
    {
        res = mysql_store_result(conn);
        if (mysql_num_rows(res) == 0)
        {
            mysql_free_result(res);
            return NULL;
        }
        return res;
    }
}

int sql_insert_user(MYSQL* conn, const char* user_name, const char* password)
{
    int ret;
    char query[QUERY_LEN];

    sprintf(query, "INSERT INTO user VALUES (default, '%s', '%s')", user_name, password);
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    ret = mysql_query(conn, query);
    if (ret)
    {
#ifdef _DEBUG
        printf("Error making query: %s\n", mysql_error(conn));
#endif
        return -1;
    }
    else
    {
        return 0;
    }
}

int sql_insert_userfile(MYSQL* conn, const char* user_id, const char* file_id)
{
    char query[QUERY_LEN];
    sprintf(query, "INSERT INTO user_file VALUES (default, '%s', '%s')", user_id, file_id);
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    int ret = mysql_query(conn, query);
    if (ret)
    {
#ifdef _DEBUG
        printf("Error making query: %s\n", mysql_error(conn));
#endif
        return -1;
    }
    else
    {
        return 0;
    }
}

int sql_insert_file(MYSQL* conn, const char* user_name, const char* dir_id, int type, const char* file_name,
                    int file_size, const char* file_md5)
{
    int ret;
    char query[QUERY_LEN];
    MYSQL_RES* res;
    MYSQL_ROW row;
    char file_path[RESULT_LEN];

    printf("filename=%s\n",file_name);
    char filename[RESULT_LEN];
    sprintf(filename, "%s", file_name);
    res = sql_select(conn, "file", "id", dir_id, 0);
    if (res == NULL)
    {
        return -1;
    }
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    sprintf(file_path, "%s/%s", row[5], filename);

    if (type == 0)
    {
        sprintf(query, "INSERT INTO file VALUES (default, %s, %d, '%s', NULL, '%s', NULL, default)",
                dir_id, type, filename, file_path);
    }
    else
    {
        sprintf(query, "INSERT INTO file VALUES (default, %s, %d, '%s', %d, '%s', '%s', default)",
            dir_id, type, filename, file_size, file_path, file_md5);
    }
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    ret = mysql_query(conn, query);
    if (ret)
    {
        return -1;
    }

    //insert into table user_file
    res = sql_select(conn, "user", "user_name", user_name, 0);
    if (res == NULL)
    {
        return -1;
    }
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    char user_id[INT_LEN];
    strcpy(user_id, row[0]);
    res = sql_select(conn, "file", "file_path", file_path, 0);
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    char file_id[INT_LEN];
    strcpy(file_id, row[0]);

    ret = sql_insert_userfile(conn, user_id, file_id);
    if (ret)
    {
        return -1;
    }
}

int sql_insert_user_trans(MYSQL* conn, const char* user_name, const char* password, const char* dir_id,
                          int type, const char* file_name, int file_size, const char* file_md5)
{
    char query[QUERY_LEN];
    int ret;

    //transacton begin
    strcpy(query, "BEGIN");
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    ret = mysql_query(conn, query);
    if (ret)
    {
        return -1;
    }

    //insert into table user
    ret = sql_insert_user(conn, user_name, password);
    if (ret)
    {
        strcpy(query, "ROLLBACK");
#ifdef _DEBUG
        printf("sql: %s\n", query);
#endif
        mysql_query(conn, query);
        return -1;
    }

    //insert into table file and user_file
    ret = sql_insert_file(conn, user_name, dir_id, type, file_name, file_size, file_md5);
    if (ret)
    {
        strcpy(query, "ROLLBACK");
#ifdef _DEBUG
        printf("sql: %s\n", query);
#endif
        mysql_query(conn, query);
        return -1;
    }

    //transaction commit
    strcpy(query, "COMMIT");
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    ret = mysql_query(conn, query);
    return ret;

}

int sql_insert_file_trans(MYSQL* conn, const char* user_name, const char* dir_id, int type, const char* file_name,
                        int file_size, const char* file_md5)
{
    char query[QUERY_LEN];
    int ret;

    //transacton begin
    strcpy(query, "BEGIN");
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    ret = mysql_query(conn, query);
    if (ret)
    {
        return -1;
    }

    //insert into table file and user_file
    ret = sql_insert_file(conn, user_name, dir_id, type, file_name, file_size, file_md5);
    if (ret)
    {
        strcpy(query, "ROLLBACK");
#ifdef _DEBUG
        printf("sql: %s\n", query);
#endif
        mysql_query(conn, query);
        return -1;
    }

    //transaction commit
    strcpy(query, "COMMIT");
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    ret = mysql_query(conn, query);
    return ret;
}

int sql_delete_userfile(MYSQL* conn, const char* user_id, const char* file_id)
{
    char query[QUERY_LEN];
    sprintf(query, "DELETE FROM user_file WHERE user_id = %s AND file_id = %s", user_id, file_id);
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    int ret = mysql_query(conn, query);
    if (ret)
    {
#ifdef _DEBUG
        printf("Error making query: %s\n", mysql_error(conn));
#endif
        return -1;
    }
    return 0;
}

int sql_delete_user(MYSQL* conn, const char* user_name)
{
    int ret;
    char query[QUERY_LEN];

    sprintf(query, "DELETE FROM user WHERE user_name = '%s'", user_name);
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    ret = mysql_query(conn, query);
    if (ret)
    {
#ifdef _DEBUG
        printf("Error making query: %s\n", mysql_error(conn));
#endif
        return -1;
    }
    return 0;
}

int sql_delete_file(MYSQL* conn, const char* user_name, const char* file_path)
{
    int ret;
    char query[QUERY_LEN];
    MYSQL_RES* res;
    MYSQL_ROW row;

    //get user id
    res = sql_select(conn, "user", "user_name", user_name, 0);
    if (res == NULL)
    {
        return -1;
    }
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    char user_id[INT_LEN];
    strcpy(user_id, row[0]);

    //get file id
    res = sql_select(conn, "file", "file_path", file_path, 0);
    if (res == NULL)
    {
        return -1;
    }
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    char file_id[INT_LEN];
    strcpy(file_id, row[0]);

    //begin transaction
    strcpy(query, "BEGIN");
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    ret = mysql_query(conn, query);
    if (ret)
    {
        return -1;
    }

    //delete userfile
    ret = sql_delete_userfile(conn, user_id, file_id);
    if (ret)
    {
        strcpy(query, "ROLLBACK");
#ifdef _DEBUG
        printf("sql: %s\n", query);
#endif
        mysql_query(conn, query);
        return -1;
    }

    //delete table file
    sprintf(query, "DELETE FROM file WHERE id = %s", file_id);
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    ret = mysql_query(conn, query);
    if (ret)
    {
        strcpy(query, "ROLLBACK");
#ifdef _DEBUG
        printf("sql: %s\n", query);
#endif
        mysql_query(conn, query);
        return -1;
    }

    strcpy(query, "COMMIT");
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    ret = mysql_query(conn, query);
    return 0;
}
