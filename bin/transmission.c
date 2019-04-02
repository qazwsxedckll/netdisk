#include "../include/transmission.h"

int send_cycle(int fd, char* data, int send_len)
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

int tran_file(int client_fd, char* file_name, char* file_md5, char* file_size)
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
        return -1;
    }
    while ((data.data_len = read(fd, data.buf, sizeof(data.buf))) > 0)
    {
        ret = send_cycle(client_fd, (char*)&data, data.data_len + sizeof(int));
        if (ret == -1)
        {
            return -1;
        }
    }
    data.data_len = 0;
    ret =send_cycle(client_fd, (char*)&data, sizeof(int));
    if (ret == -1)
    {
        return -1;
    }
    close(fd);
    close(client_fd);
    return 0;
}
