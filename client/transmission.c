#include "transmission.h"

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
        total = total + ret;
    }
    return 0;
}

void user_signup(int* socketFd, const char* ip, const char* port, char* user_name, DataPackage* data)
{
    int ret;
    int flag = -1, err;
    char invi_code[5];
    while (flag == -1)
    {
        system("clear");
        if (err == -1)
        {
            printf("connection failed, try again later\n");
            err = 0;
        }
        if (err == -2)
        {
            printf("wrong invitation code\n");
            err = 0;
        }
        if (err == -3)
        {
            printf("unknown error occured\n");
            err = 0;
        }
        printf("Enter invitation code: ");
        fflush(stdout);
        ret = read(STDIN_FILENO, invi_code, sizeof(invi_code));
        invi_code[ret - 1] = '\0';

        //connect to server
        ret = connect_server(socketFd, ip, port);
        if (ret == -1)
        {
            err = -1;
        }
        data->data_len = 4;
        send_cycle(*socketFd, (char*)data, sizeof(int));        //4 for invitation code
        strcpy(data->buf, invi_code);
        data->data_len = strlen(data->buf) + 1;
        send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send code

        recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));         //recv comfirmation
        if (data->data_len == 0)
        {
            flag = 0;
            close(*socketFd);
        }
        else if (data->data_len == -1)
        {
            err = -2;
            close(*socketFd);
        }
        else
        {
            err = -3;
            close(*socketFd);
        }
    }

    flag = -1;
    while (flag == -1)
    {
        //input name
        flag = -2;
        while (flag == -2)
        {
            flag = 0;
            system("clear");
            printf("Enter invitation code: %s\n", invi_code);
            if (err == -1)
            {
                printf("username too long!\n");
                err = 0;
            }
            printf("Enter username: ");
            fflush(stdout);
            while (1)
            {
                ret = read(STDIN_FILENO, user_name, USER_LEN);
                if (ret >= 20)
                {
                    err = -1;       //too long err
                    flag = -1;
                }
                else
                {
                    break;
                }
            }
        }
        user_name[ret - 1] = '\0';

        //comfirm name
        if (err == -1)
        {
            printf("connection failed, try again later\n");
            err = 0;
        }
        if (err == -2)
        {
            printf("username already used\n");
            err = 0;
        }
        if (err == -3)
        {
            printf("unknown error occured\n");
            err = 0;
        }

        //connect to server
        ret = connect_server(socketFd, ip, port);
        if (ret == -1)
        {
            err = -1;
        }
        data->data_len = 5;
        send_cycle(*socketFd, (char*)data, sizeof(int));        //5 for regi name
        strcpy(data->buf, user_name);
        data->data_len = strlen(data->buf) + 1;
        send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send username

        recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));         //recv comfirmation
        if (data->data_len == 0)
        {
            flag = 0;
            close(*socketFd);
        }
        else if (data->data_len == -1)
        {
            err = -2;
            close(*socketFd);
        }
        else
        {
            err = -3;
            close(*socketFd);
        }
    }

    flag = -1;
    while (flag == -1)
    {
        //input password
        char* password;
        flag = -2;
        while (flag == -2)
        {
            flag = 0;
            system("clear");
            printf("Enter invitation code: %s\n", invi_code);
            printf("Enter username: %s\n", user_name);
            if (err == -1)
            {
                printf("password too long!\n");
                err = 0;
            }
            password = getpass("Enter password: ");
            while (1)
            {
                if (strlen(password) >= 20)
                {
                    err = -1;       //too long err
                    flag = -1;
                }
                else
                {
                    break;
                }
            }
        }

        if (err == -1)
        {
            printf("connection failed, try again later\n");
            err = 0;
        }
        if (err == -2)
        {
            printf("unknown error occured\n");
            err = 0;
        }

        //connect to server
        ret = connect_server(socketFd, ip, port);
        if (ret == -1)
        {
            err = -1;
        }
        data->data_len = 6;
        send_cycle(*socketFd, (char*)data, sizeof(int));        //6 for regi password
        strcpy(data->buf, user_name);
        data->data_len = strlen(data->buf) + 1;
        send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send user_name
        strcpy(data->buf, password);
        data->data_len = strlen(data->buf) + 1;
        send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send password

        recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));         //recv comfirmation
        if (data->data_len == 0)
        {
            flag = 0;
            close(*socketFd);
        }
        else if (data->data_len == -1)
        {
            err = -2;
            close(*socketFd);
        }
    }
}

int tran_authen(int* socketFd, const char* ip, const char* port, char* user_name, DataPackage* data, int err)
{
    int ret;
    int flag, err_flag = 0;
    do
    {
        flag = 0;
        system("clear");
        if (err == -1)
        {
            printf("wrong username or password\n");
            err = 0;
        }
        if (err == -2)
        {
            printf("connection failed, try again later\n");
            err = 0;
        }
        if (err_flag == 1)
        {
            printf("username too long!\n");
            err_flag = 0;
        }
        printf("Enter username: ");
        fflush(stdout);
        while (1)
        {
            ret = read(STDIN_FILENO, user_name, USER_LEN);
            if (ret >= 20)
            {
                flag = 1;
                err_flag = 1;       //too long err
            }
            else
            {
                break;
            }
        }
    } while (flag == 1);
    user_name[ret - 1] = '\0';

    char* password;
    err_flag = 0;
    while (1)
    {
        if (err_flag == 1)
        {
            system("clear");
            printf("Enter username: %s\n", user_name);
            printf("password too long!\n");
            err_flag = 0;
        }
        password = getpass("Enter password: ");
        if (strlen(password) > 20)
        {
            err_flag = 1;
        }
        else
        {
            break;
        }
    }

    //connect to server
    ret = connect_server(socketFd, ip, port);
    if (ret == -1)
    {
        return -2;
    }
    data->data_len = 0;
    send_cycle(*socketFd, (char*)data, sizeof(int));        //0 for login
    data->data_len = strlen(user_name) + 1;
    strcpy(data->buf, user_name);
    send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send username
    data->data_len = strlen(password) + 1;
    strcpy(data->buf, password);
    send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send password

    recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));         //recv token
    recv_cycle(*socketFd, data->buf, data->data_len);
    if (data->data_len == -1)
    {
        close(*socketFd);
        return -1;
    }
    else
    {
        return 0;
    }
}

int tran_cmd(int socket_fd, DataPackage* data)
{
    send_cycle(socket_fd, (char*)data, data->data_len + 4);
    system("clear");
    printf("-----$ %s\n", data->buf);
    while (1)
    {
        recv_cycle(socket_fd, (char*)&data->data_len, sizeof(int));
        if (data->data_len == 0)
        {
            break;
        }
        recv_cycle(socket_fd, data->buf, data->data_len);
        printf("%s\n", data->buf);
    }
    return 0;
}

int thread_connect(int* socketFd, DataPackage* data, TransInfo* trans_info, int code)
{
    int ret;
    ret = connect_server(socketFd, trans_info->ip_address, trans_info->port);
    if (ret == -1)
    {
        return -1;
    }
    data->data_len = code;
    send_cycle(*socketFd, (char*)data, sizeof(int));        //2 for gets
    data->data_len = strlen(trans_info->token) + 1;
    strcpy(data->buf, trans_info->token);
    send_cycle(*socketFd, (char*)data, sizeof(int) + data->data_len);//send token
    recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));
    if (data->data_len == -1)
    {
        return -1;
    }

    data->data_len = strlen(trans_info->cmd) + 1;
    strcpy(data->buf, trans_info->cmd);
    send_cycle(*socketFd, (char*)data, sizeof(int) + data->data_len);//send command
    recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));
    if (data->data_len == -1)
    {
        return -1;
    }

    return 0;
}

void* get_files(void* p)
{
    int ret, socketFd;
    DataPackage data;
    TransInfo* trans_info = (TransInfo*)p;
    ret = thread_connect(&socketFd, &data, trans_info, 2);
    if (ret == -1)
    {
        close(socketFd);
        pthread_exit(NULL);
    }

    recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));       //recv filename
    recv_cycle(socketFd, data.buf, data.data_len);
    char path_name[CMD_LEN] = "./downloads/";
    strcat(path_name, data.buf);
    int fd = open(path_name, O_CREAT|O_RDWR, 0666);
    if (fd == -1)
    {
        close(socketFd);
        pthread_exit(NULL);
    }

    recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));       //recv size
    recv_cycle(socketFd, data.buf, data.data_len);
    off_t size = atoi(data.buf);

    int transfered = 0;
    time_t start, end;
    start = time(NULL);
    printf("\r%4.1f%%", (float)transfered / size * 100);
    fflush(stdout);
    while (1)
    {
        recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));
        if (data.data_len > 0)
        {
            recv_cycle(socketFd, data.buf, data.data_len);
            write(fd, data.buf, data.data_len);
            transfered += data.data_len;
            end = time(NULL);
            if (end - start >= 1)
            {
                printf("\r%4.1f%%", (float)transfered / size * 100);
                start = end;
                fflush(stdout);
            }
        }
        else
        {
            printf("\r%4.1f%%\n", (float)transfered / size * 100);
            close(fd);
            break;
        }
    }
    close(socketFd);
    pthread_exit(NULL);
}

void* put_files(void* p)
{
    int ret, socketFd;
    DataPackage data;
    TransInfo* trans_info = (TransInfo*)p;
    ret = thread_connect(&socketFd, &data, trans_info, 3);
    if (ret == -1)
    {
        close(socketFd);
        pthread_exit(NULL);
    }

    //open file
    char file_path[CMD_LEN];
    int i = 0;
    while (trans_info->cmd[i] != '\0')
    {
        file_path[i] = trans_info->cmd[i + 5];
        i++;
    }
    file_path[i] = '\0';
    int fd = open(file_path, O_RDONLY);
    if (fd == -1)
    {
        close(socketFd);
        pthread_exit(NULL);
    }

    //send filename
    char file_name[FILE_NAME_LEN];
    i = i + 5;
    while (trans_info->cmd[i] != '/' && trans_info->cmd[i] != ' ')
    {
        i--;
    }
    i++;
    int k = 0;
    while (trans_info->cmd[i] != '\0')
    {
        file_name[k++] = trans_info->cmd[i++];
    }
    file_name[k] = '\0';
    data.data_len = strlen(file_name) + 1;
    strcpy(data.buf, file_name);
    ret = send_cycle(socketFd, (char*)&data, data.data_len + sizeof(int));
    if (ret == -1)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }

    //send file size
    struct stat buf;
    ret = fstat(fd, &buf);
    if (ret == -1)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }
    off_t file_size = buf.st_size;
    sprintf(data.buf, "%ld", file_size);
    data.data_len = strlen(data.buf);
    send_cycle(socketFd, (char*)&data, data.data_len + sizeof(int));
    if (ret == -1)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }

    //send file
    while ((data.data_len = read(fd, data.buf, sizeof(data.buf))) > 0)
    {
        ret = send_cycle(socketFd, (char*)&data, data.data_len + sizeof(int));
        if (ret == -1)
        {
            close(fd);
            close(socketFd);
            pthread_exit(NULL);
        }
    }

    //send end of transmission
    data.data_len = 0;
    ret = send_cycle(socketFd, (char*)&data, sizeof(int));
    if (ret == -1)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }
    printf("upload success\n");
    close(fd);
    close(socketFd);
    pthread_exit(NULL);
}
