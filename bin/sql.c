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

MYSQL_RES* sql_select_by_dirid(MYSQL* conn, int dir_id)
{
    MYSQL_RES* res = NULL;
    char query[QUERY_LEN];
    sprintf(query, "SELECT * FROM file WHERE dir_id = %d", dir_id);
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    int t = mysql_query(conn, query);
    if (t)
    {
#ifdef _DEBUG
        printf("Error making query:%s\n", mysql_error(conn));
#endif
        return NULL;
    }
    else
    {
        res = mysql_store_result(conn);
        return res;
    }
}

MYSQL_RES* sql_select_by_path(MYSQL* conn, char* file_path)
{
    MYSQL_RES* res = NULL;
    char query[QUERY_LEN];
    sprintf(query, "SELECT * FROM file WHERE file_path = '%s'", file_path);
#ifdef _DEBUG
    printf("sql: %s\n", query);
#endif
    int t = mysql_query(conn, query);
    if (t)
    {
#ifdef _DEBUG
        printf("Error making query:%s\n", mysql_error(conn));
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

