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
        if (ret == 0)
        {
            return -1;
        }
        total = total + ret;
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
    if (ret == -1)
    {
        return -1;
    }
    char file_path[MD5_LEN] = "../netdisk/";
    strcat(file_path, file_md5);
    int fd = open(file_path, O_RDONLY);
    if (fd == -1)
    {
        return -2;
    }
    data.data_len = strlen(file_size) + 1;
    strcpy(data.buf, file_size);
    ret = send_cycle(client_fd, (char*)&data, data.data_len + sizeof(int));        //send file size
    if (ret == -1)
    {
        close(fd);
        return -1;
    }
    while ((data.data_len = read(fd, data.buf, sizeof(data.buf))) > 0)
    {
        ret = send_cycle(client_fd, (char*)&data, data.data_len + sizeof(int));
        if (ret == -1)
        {
            close(fd);
            return -1;
        }
    }
    data.data_len = 0;
    ret = send_cycle(client_fd, (char*)&data, sizeof(int));
    if (ret == -1)
    {
        close(fd);
        return -1;
    }
    close(fd);
    close(client_fd);
    return 0;
}

int recv_file(int client_fd, const char* user_name, const char* cur_dir_id)
{
    int ret;
    DataPackage data;
    char file_name[FILE_NAME_LEN];
    long file_size;
    char file_md5[MD5_LEN];

    char path_name[RESULT_LEN] = "../netdisk/tmp_";
    strcat(path_name, user_name);
    int fd = open(path_name, O_CREAT|O_RDWR, 0666);
    if (fd == -1)
    {
        return -2;
    }

    ret = recv_cycle(client_fd, (char*)&data.data_len, sizeof(int));      //recv filename;
    if (ret == -1)
    {
        remove(path_name);
        close(fd);
        close(client_fd);
        return -1;
    }
    ret = recv_cycle(client_fd, data.buf, data.data_len);
    if (ret == -1)
    {
        remove(path_name);
        close(fd);
        close(client_fd);
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
        close(client_fd);
        return -1;
    }
    ret = recv_cycle(client_fd, data.buf, data.data_len);
    if (ret == -1)
    {
        remove(path_name);
        close(fd);
        close(client_fd);
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
            close(client_fd);
            return -1;
        }
        if (data.data_len > 0)
        {
            recv_cycle(client_fd, data.buf, data.data_len);
            write(fd, data.buf, data.data_len);
            transfered += data.data_len;
        }
        else
        {
            lseek(fd, 0, SEEK_SET);
            compute_file_md5(fd, file_md5);
            char new_pathname[RESULT_LEN] = "../netdisk/";
            strcat(new_pathname, file_md5);
            rename(path_name, new_pathname);

            //databse ops
            MYSQL* conn;
            ret = sql_connect(&conn);
            if (ret == -1)
            {
                remove(path_name);
                close(fd);
                close(client_fd);
                return -1;
            }
            ret = sql_insert_file(conn, user_name, cur_dir_id, 1, file_name, file_size, file_md5);
            if (ret)
            {
                mysql_close(conn);
#ifdef _DEBUG
                printf("database closed\n");
#endif
                remove(path_name);
                close(fd);
                close(client_fd);
                return -1;      //insert failed
            }
            mysql_close(conn);
#ifdef _DEBUG
            printf("database closed\n");
#endif
            close(fd);
            close(client_fd);
            return 0;
        }
    }
}
