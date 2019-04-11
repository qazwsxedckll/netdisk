#include "../include/transmission.h"

int send_cycle(int fd, const char* data, int send_len)
{
    int total = 0;
    int ret;
    while(total < send_len)
    {
        ret = send(fd, data + total, send_len - total, 0);
        if (ret == -1)
        {
            printf("transmission interrupted\n");
            return -1;
        }
        if (ret == 0)
        {
            printf("transmission closed\n");
            return -1;
        }
        total = total + ret;
    }
    return 0;
}

int recv_cycle(int fd, char* data, int recv_len)
{
    int total = 0;
    int ret;
    while (total < recv_len)
    {
        ret = recv(fd, data + total, recv_len - total, 0);
        if (ret == -1)
        {
            printf("transmission interrupted\n");
            return -1;
        }
        if (ret == 0)
        {
            printf("transmission closed\n");
            return -1;
        }
        total = total + ret;
    }
    return 0;
}

int send_nonce(int fd, DataPackage* data, const char* user_name)
{
    char nonce[15];
    srand((unsigned)(time(NULL)));
    sprintf(nonce, "%d", rand());
    strcpy(data->buf, nonce);
    data->data_len = strlen(data->buf) + 1;
    if (send_cycle(fd, (char*)data, data->data_len + sizeof(int)))
    {
        return -1;
    }
    if (recv_cycle(fd, (char*)&data->data_len, sizeof(int)))
    {
        return -1;
    }
    if (recv_cycle(fd, data->buf, data->data_len))
    {
        return -1;
    }
    char* nonce_tmp;
    nonce_tmp = rsa_verify(data->buf, user_name);
    if (nonce_tmp == NULL)
    {
        return -1;
    }
    if (strcmp(nonce_tmp, nonce) != 0)
    {
        free(nonce_tmp);
        nonce_tmp =NULL;
        printf("nonce verification failed\n");
        return -1;
    }
    free(nonce_tmp);
    nonce_tmp = NULL;
    return 0;
}

int recv_nonce(int fd, DataPackage* data)
{
    if (recv_cycle(fd, (char*)&data->data_len, sizeof(int))) //get nonce
    {
        return -1;
    }
    if (recv_cycle(fd, data->buf, data->data_len))
    {
        return -1;
    }
    char* nonce_tmp;
    nonce_tmp = rsa_sign(data->buf);
    if (nonce_tmp == NULL)
    {
        return -1;
    }
    memcpy(data->buf, nonce_tmp, SER_EN_LEN);  //sign
    free(nonce_tmp);
    nonce_tmp = NULL;
    data->data_len = SER_EN_LEN;
    if (send_cycle(fd, (char*)data, data->data_len + sizeof(int))) //send back
    {
        return -1;
    }
    return 0;
}

int send_file(int client_fd, const char* file_name, const char* file_md5, const char* file_size)
{
    int ret;
    DataPackage data;
    data.data_len = strlen(file_name) + 1;
    strcpy(data.buf, file_name);
    ret = send_cycle(client_fd, (char*)&data, data.data_len + sizeof(int));      //send file name
    if (ret)
    {
        return -1;
    }
    char file_path[MD5_LEN] = "netdisk/";
    strcat(file_path, file_md5);
    int fd = open(file_path, O_RDONLY);
    if (fd == -1)
    {
#ifdef _DEBUG
        printf("cannot open file\n");
#endif
        return -2;
    }
    data.data_len = strlen(file_size) + 1;
    strcpy(data.buf, file_size);
    ret = send_cycle(client_fd, (char*)&data, data.data_len + sizeof(int));        //send file size
    if (ret)
    {
        close(fd);
        return -1;
    }
    while ((data.data_len = read(fd, data.buf, sizeof(data.buf))) > 0)
    {
        ret = send_cycle(client_fd, (char*)&data, data.data_len + sizeof(int));
        if (ret)
        {
            close(fd);
            return -1;
        }
    }
    data.data_len = 0;
    ret = send_cycle(client_fd, (char*)&data, sizeof(int));
    if (ret)
    {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int send_dir(int client_fd, char* cur_dir_id, MYSQL* conn)
{
    MYSQL_RES* res;
    MYSQL_ROW row;
    DataPackage data;
    if ((res = sql_select(conn, "file", "dir_id", cur_dir_id, 0)))
    {
        int num = mysql_num_rows(res);
        for (int i = 0; i < num; i++)
        {
            row = mysql_fetch_row(res);
            if (atoi(row[2]) == 1)
            {
                data.data_len = 0;
                if (send_cycle(client_fd, (char*)&data, sizeof(int)))
                {
                    return -1;
                }
                if (send_file(client_fd, row[3], row[6], row[4]))
                {
                    return -1;
                }
            }
            else
            {
                data.data_len = 1;
                if (send_cycle(client_fd, (char*)&data, sizeof(int)))
                {
                    return -1;
                }
                strcpy(data.buf, row[3]);
                data.data_len = strlen(data.buf) + 1;
                if (send_cycle(client_fd, (char*)&data, sizeof(int) + data.data_len))
                {
                    return -1;
                }
                strcpy(cur_dir_id, row[0]);

                if (send_dir(client_fd, cur_dir_id, conn))
                {
                    return -1;
                }
            }
        }
        data.data_len = 2;
        if (send_cycle(client_fd, (char*)&data, sizeof(int)))
        {
            return -1;
        }
        mysql_free_result(res);
        return 0;
    }
    else
    {
        mysql_free_result(res);
        return 0;
    }
}

int send_files(int client_fd, const char* file_name, const char* file_md5, const char* file_size)   //file_size = cur_dir_id when send dir
{
    DataPackage data;
    data.data_len = 0;
    send_cycle(client_fd, (char*)&data, sizeof(int));
    if (strcmp(file_md5, "0"))
    {
        data.data_len = 0;
        if (send_cycle(client_fd, (char*)&data, sizeof(int)))
        {
            return -1;
        }

        if (send_file(client_fd, file_name, file_md5, file_size))
        {
            close(client_fd);
            return -1;
        }

        close(client_fd);
        return 0;
    }
    else
    {
        char cur_dir_id[INT_LEN];
        strcpy(cur_dir_id, file_size);
        data.data_len = 1;
        if (send_cycle(client_fd, (char*)&data, sizeof(int)))
        {
            close(client_fd);
            return -1;
        }

        strcpy(data.buf, file_name);
        data.data_len = strlen(data.buf) + 1;
        if (send_cycle(client_fd, (char*)&data, sizeof(int) + data.data_len))      //send dir name
        {
            close(client_fd);
            return -1;
        }

        MYSQL* conn;
        MYSQL_RES* res;
        MYSQL_ROW row;
        if (sql_connect(&conn))
        {
            close(client_fd);
            return -1;
        }

        res = sql_select(conn, "file", "dir_id", cur_dir_id, 0);
        for (int i = 0; i < (int)mysql_num_rows(res); i++)
        {
            row = mysql_fetch_row(res);
            if (strcmp(file_name, row[3]) == 0)
            {
                strcpy(cur_dir_id, row[0]);
                break;
            }
        }
        mysql_free_result(res);

        if (send_dir(client_fd, cur_dir_id, conn))
        {
            close(client_fd);
            return -1;
        }
        close(client_fd);
        return 0;
    }
}

int recv_file(int client_fd, const char* user_name, const char* cur_dir_id, MYSQL* conn)
{
    int ret;
    DataPackage data;
    char file_name[FILE_NAME_LEN];
    long file_size;
    char file_md5[MD5_LEN];
    MYSQL_RES* res;
    MYSQL_ROW row;
    ret = recv_cycle(client_fd, (char*)&data.data_len, sizeof(int));      //recv md5;
    if (ret)
    {
        return -1;
    }
    ret = recv_cycle(client_fd, data.buf, data.data_len);
    if (ret)
    {
        return -1;
    }
    strcpy(file_md5, data.buf);
    res = sql_select(conn, "file", "file_md5", file_md5, 0);
    if (res == NULL)        //file not exist
    {
        mysql_free_result(res);
        data.data_len = 0;
        ret = send_cycle(client_fd, (char*)&data, sizeof(int));
        if (ret)
        {
            return -1;
        }
    }
    else            //file already exist
    {
        data.data_len = 1;
        ret = send_cycle(client_fd, (char*)&data, sizeof(int));
        if (ret)
        {
            return -1;
        }
        row = mysql_fetch_row(res);
        mysql_free_result(res);
        char file_name[RESULT_LEN];
        strcpy(file_name, row[3]);
        ret = sql_insert_file_trans(conn, user_name, cur_dir_id, 1, file_name, atol(row[4]), file_md5);
        if (ret)
        {
            data.data_len = -1;
            ret = send_cycle(client_fd, (char*)&data, sizeof(int));
#ifdef _DEBUG
            printf("thread database closed\n");
#endif
            return -1;      //insert failed
        }
        data.data_len = 2;      //send trans success
        ret = send_cycle(client_fd, (char*)&data, sizeof(int));
        if (ret)
        {
            return -1;
        }
        close(client_fd);
#ifdef _DEBUG
        printf("database closed\n");
#endif
        return 0;
    }

    char path_name[RESULT_LEN] = "netdisk/";
    strcat(path_name, file_md5);
    int fd = open(path_name, O_CREAT|O_RDWR, 0666);
    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    ret = recv_cycle(client_fd, (char*)&data.data_len, sizeof(int));      //recv filename;
    if (ret == -1)
    {
        remove(path_name);
        close(fd);
        return -1;
    }
    ret = recv_cycle(client_fd, data.buf, data.data_len);
    if (ret == -1)
    {
        remove(path_name);
        close(fd);
        return -1;
    }
    strcpy(file_name, data.buf);
#ifdef _DEBUG
    printf("filename: %s\n", file_name);
#endif

    ret = recv_cycle(client_fd, (char*)&data.data_len, sizeof(int));      //recv filesize;
    if (ret == -1)
    {
        remove(path_name);
        close(fd);
        return -1;
    }
    ret = recv_cycle(client_fd, data.buf, data.data_len);
    if (ret == -1)
    {
        remove(path_name);
        close(fd);
        return -1;
    }
    file_size = atol(data.buf);
#ifdef _DEBUG
    printf("filesize: %ld\n", file_size);
#endif

    int transfered = 0;
    while (1)
    {
        ret = recv_cycle(client_fd, (char*)&data.data_len, sizeof(int));
        if (ret == -1)
        {
            remove(path_name);
            close(fd);
            return -1;
        }
        if (data.data_len > 0)
        {
            recv_cycle(client_fd, data.buf, data.data_len);
            if (ret == -1)
            {
                remove(path_name);
                close(fd);
                return -1;
            }
            ret = write(fd, data.buf, data.data_len);
            if (ret == -1)
            {
                perror("write");
                remove(path_name);
                close(fd);
                return -1;
            }
            transfered += data.data_len;
        }
        else
        {
            //databse ops
            ret = sql_insert_file_trans(conn, user_name, cur_dir_id, 1, file_name, file_size, file_md5);
            if (ret)
            {
                data.data_len = -1;
                send_cycle(client_fd, (char*)&data, sizeof(int));
                remove(path_name);
                close(fd);
                return -1;      //insert failed
            }
            data.data_len = 0;
            ret = send_cycle(client_fd, (char*)&data, sizeof(int));
            if (ret)
            {
                remove(path_name);
                close(fd);
                return -1;
            }
            close(fd);
            return 0;
        }
    }
}

int recv_dir(int client_fd, const char* user_name, char* cur_dir_id, MYSQL* conn)
{
    int ret;
    DataPackage data;
    MYSQL_RES* res;
    MYSQL_ROW row;
    while (1)
    {
        ret = recv_cycle(client_fd, (char*)&data.data_len, sizeof(int));    //0 for dir, 1 for file, 2 for end of dir
        if (ret)
        {
            close(client_fd);
            return -1;
        }

        if (data.data_len == 0)
        {
            ret = recv_cycle(client_fd, (char*)&data.data_len, sizeof(int));
            if (ret)
            {
                close(client_fd);
                return -1;
            }
            ret = recv_cycle(client_fd, data.buf, data.data_len);
            if (ret)
            {
                close(client_fd);
                return -1;
            }
            char dir_name[FILE_NAME_LEN];
            strcpy(dir_name, data.buf);

            if (sql_insert_file_trans(conn, user_name, cur_dir_id, 0, dir_name, 0, NULL))
            {
                return -1;
            }
            res = sql_select(conn, "file", "dir_id", cur_dir_id, 0);
            int num = mysql_num_rows(res);
            for (int i = 0; i < num; i++)
            {
                row = mysql_fetch_row(res);
                if (strcmp(dir_name, row[3]) == 0)
                {
                    char cur_dir_id[INT_LEN];
                    strcpy(cur_dir_id, row[0]);
                    mysql_free_result(res);
                    if (recv_dir(client_fd, user_name, cur_dir_id, conn))
                    {
                        return -1;
                    }
                    break;
                }
            }
        }
        else if (data.data_len == 1)
        {
            if (recv_file(client_fd, user_name, cur_dir_id, conn))
            {
                return -1;
            }
        }
        else if (data.data_len == 2)
        {
            printf("returned\n");
            return 0;
        }
    }
}

int recv_files(int client_fd, const char* user_name, char* cur_dir_id)
{
    int ret;
    DataPackage data;
    data.data_len = 0;
    send_cycle(client_fd, (char*)&data, sizeof(int));

    ret = recv_cycle(client_fd, (char*)&data.data_len, sizeof(int));    //0 for dir, 1 for file
    if (ret)
    {
        close(client_fd);
        return -1;
    }

    MYSQL* conn;
    ret = sql_connect(&conn);
    if (ret)
    {
        close(client_fd);
        return -1;
    }

    if (data.data_len == 1)
    {
        if (recv_file(client_fd, user_name, cur_dir_id, conn))
        {
            mysql_close(conn);
            close(client_fd);
            return -1;
        }
        mysql_close(conn);
        close(client_fd);
        return 0;
    }
    else            //recv dir
    {
        ret = recv_cycle(client_fd, (char*)&data.data_len, sizeof(int));
        if (ret)
        {
            mysql_close(conn);
            close(client_fd);
            return -1;
        }
        ret = recv_cycle(client_fd, data.buf, data.data_len);
        if (ret)
        {
            mysql_close(conn);
            close(client_fd);
            return -1;
        }
        char dir_name[FILE_NAME_LEN];
        strcpy(dir_name, data.buf);

        MYSQL_RES* res;
        MYSQL_ROW row;

        if (sql_insert_file_trans(conn, user_name, cur_dir_id, 0, dir_name, 0, NULL))
        {
            close(client_fd);
            mysql_close(conn);
            return -1;
        }

        res = sql_select(conn, "file", "dir_id", cur_dir_id, 0);
        int num = mysql_num_rows(res);
        for (int i = 0; i < num; i++)
        {
            row = mysql_fetch_row(res);
            if (strcmp(dir_name, row[3]) == 0)
            {
                strcpy(cur_dir_id, row[0]);
                break;
            }
        }
        mysql_free_result(res);

        recv_dir(client_fd, user_name, cur_dir_id, conn);
        close(client_fd);
        mysql_close(conn);
        return 0;
    }
}
