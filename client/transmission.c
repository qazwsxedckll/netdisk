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

int user_signup(int* socketFd, const char* ip, const char* port, char* user_name, DataPackage* data)
{
    int ret;
    int flag = -1, err = 0;
    char invi_code[5];
    while (flag == -1)
    {
        system("clear");
        if (err == -1)
        {
            printf("wrong invitation code\n");
            err = 0;
        }
        if (err == -2)
        {
            printf("unknown error occured\n");
            err = 0;
        }
        printf("Enter '0000' to go back\n");
        printf("Enter invitation code: ");
        fflush(stdout);
        ret = read(STDIN_FILENO, invi_code, sizeof(invi_code) - 1);
        invi_code[ret] = '\0';
        //flush stdin
        char ch;
        while (read(STDIN_FILENO, &ch, 1))
        {
            if (ch == '\n')
                break;
        }
        if (strcmp(invi_code, "0000") == 0)
        {
            return -1;
        }
        //connect to server
        ret = connect_server(socketFd, ip, port);
        if (ret)
        {
            continue;
        }
        data->data_len = 4;
        ret = send_cycle(*socketFd, (char*)data, sizeof(int));        //4 for invitation code
        if (ret)
        {
            continue;
        }
        strcpy(data->buf, invi_code);
        data->data_len = strlen(data->buf) + 1;
        ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send code
        if (ret)
        {
            continue;
        }

        ret = recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));         //recv comfirmation
        if (ret)
        {
            continue;
        }

        if (data->data_len == 0)
        {
            flag = 0;
            close(*socketFd);
        }
        else if (data->data_len == -1)
        {
            err = -1;
            close(*socketFd);
        }
        else
        {
            err = -2;
            close(*socketFd);
        }
    }

    flag = -1;
    err = 0;
    while (flag == -1)
    {
        //input name
        flag = -2;
        while (flag == -2)
        {
            flag = 0;
            system("clear");
            printf("Enter '0' to go back\n");
            printf("Enter invitation code: %s\n", invi_code);
            //comfirm name
            if (err == -1)
            {
                printf("username already used\n");
                err = 0;
            }
            if (err == -2)
            {
                printf("unknown error occured\n");
                err = 0;
            }
            if (err == -3)
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
                    err = -3;       //too long err
                    flag = -2;
                }
                else
                {
                    break;
                }
            }
            if (err == 0)
            {
                flag = -1;
            }
        }
        user_name[ret - 1] = '\0';
        if (strcmp(user_name, "0") == 0)
        {
            return -1;
        }

        //connect to server
        ret = connect_server(socketFd, ip, port);
        if (ret)
        {
            continue;
        }
        data->data_len = 5;
        ret = send_cycle(*socketFd, (char*)data, sizeof(int));        //5 for regi name
        if (ret)
        {
            continue;
        }
        strcpy(data->buf, user_name);
        data->data_len = strlen(data->buf) + 1;
        ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send username
        if (ret)
        {
            continue;
        }

        ret = recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));         //recv comfirmation
        if (ret)
        {
            continue;
        }

        if (data->data_len == 0)
        {
            flag = 0;
            close(*socketFd);
        }
        else if (data->data_len == -1)
        {
            err = -1;
            close(*socketFd);
        }
        else
        {
            err = -2;
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
            printf("Enter '0' to go back\n");
            printf("Enter invitation code: %s\n", invi_code);
            printf("Enter username: %s\n", user_name);
            if (err == -1)
            {
                printf("unknown error occured\n");
                err = 0;
            }
            if (err == -2)
            {
                printf("password too long!\n");
                err = 0;
            }
            password = getpass("Enter password: ");
            if (strlen(password) >= 20)
            {
                err = -2;       //too long err
                flag = -2;
            }
            else
            {
                flag = -1;
            }
        }
        if (strcmp(password, "0") == 0)
        {
            return -1;
        }

        //connect to server
        ret = connect_server(socketFd, ip, port);
        if (ret)
        {
            continue;
        }
        data->data_len = 6;
        ret = send_cycle(*socketFd, (char*)data, sizeof(int));        //6 for regi password
        if (ret)
        {
            continue;
        }
        strcpy(data->buf, user_name);
        data->data_len = strlen(data->buf) + 1;
        ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send user_name
        if (ret)
        {
            continue;
        }
        strcpy(data->buf, password);
        data->data_len = strlen(data->buf) + 1;
        ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send password
        if (ret)
        {
            continue;
        }

        ret = recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));         //recv comfirmation
        if (ret)
        {
            continue;
        }

        if (data->data_len == 0)
        {
            flag = 0;
            close(*socketFd);
        }
        else if (data->data_len == -1)
        {
            err = -1;
            close(*socketFd);
        }
    }
    return 0;
}

int tran_authen(int* socketFd, const char* ip, const char* port, char* user_name, DataPackage* data)
{
    int flag, ret, err;
    flag = -1;
    err = 0;

    while (flag == -1)
    {
        //input name
        flag = -2;
        while (flag == -2)
        {
            flag = 0;
            system("clear");
            if (err == -1)
            {
                printf("username too long!\n");
                err = 0;
            }
            if (err == -2)
            {
                printf("wrong username or password\n");
                err = 0;
            }
            printf("Enter '0' to go back\n");
            printf("Enter username: ");
            fflush(stdout);
            while (1)
            {
                ret = read(STDIN_FILENO, user_name, USER_LEN);
                if (ret >= 20)
                {
                    err = -1;       //too long err
                    flag = -2;
                }
                else
                {
                    break;
                }
            }
            if (err == 0)
            {
                flag = -1;
            }
        }
        user_name[ret - 1] = '\0';
        if (strcmp(user_name, "0") == 0)
        {
            return -1;
        }

        //input password
        char* password;
        flag = -2;
        while (flag == -2)
        {
            flag = 0;
            system("clear");
            printf("Enter '0' to go back\n");
            printf("Enter username: %s\n", user_name);
            if (err == -1)
            {
                printf("password too long!\n");
                err = 0;
            }
            password = getpass("Enter password: ");
            if (strlen(password) >= 20)
            {
                err = -1;       //too long err
                flag = -2;
            }
            else
            {
                flag = -1;
            }
        }
        if (strcmp(password, "0") == 0)
        {
            return -1;
        }

        //connect to server
        ret = connect_server(socketFd, ip, port);
        if (ret)
        {
            continue;
        }
        data->data_len = 0;
        ret = send_cycle(*socketFd, (char*)data, sizeof(int));        //0 for login
        if (ret)
        {
            continue;
        }
        strcpy(data->buf, user_name);
        data->data_len = strlen(data->buf) + 1;
        ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send user_name
        if (ret)
        {
            continue;
        }
        strcpy(data->buf, password);
        data->data_len = strlen(data->buf) + 1;
        ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send password
        if (ret)
        {
            continue;
        }

        ret = recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));         //recv comfirm
        if (ret)
        {
            continue;
        }

        if (data->data_len == -1)
        {
            err = -2;
            continue;
        }

        ret = recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));         //recv token
        if (ret)
        {
            continue;
        }
        ret = recv_cycle(*socketFd, data->buf, data->data_len);
        if (ret)
        {
            continue;
        }
        break;
    }
    return 0;
}

int tran_cmd(int socket_fd, DataPackage* data)
{
    int ret;
    send_cycle(socket_fd, (char*)data, data->data_len + 4);
    system("clear");
    printf("-----$ %s\n", data->buf);
    while (1)
    {
        ret = recv_cycle(socket_fd, (char*)&data->data_len, sizeof(int));
        if (ret)
        {
            break;
        }
        if (data->data_len == 0)
        {
            break;
        }
        ret = recv_cycle(socket_fd, data->buf, data->data_len);
        if (ret)
        {
            break;
        }
        printf("%s\n", data->buf);
    }
    return 0;
}

int thread_connect(int* socketFd, DataPackage* data, TransInfo* trans_info, int code)
{
    int ret;
    ret = connect_server(socketFd, trans_info->ip_address, trans_info->port);
    if (ret)
    {
        return -1;
    }
    data->data_len = code;

    ret = send_cycle(*socketFd, (char*)data, sizeof(int));        //2 for gets 3 for puts
    if (ret)
    {
        return -1;
    }
    data->data_len = strlen(trans_info->token) + 1;
    strcpy(data->buf, trans_info->token);
    ret = send_cycle(*socketFd, (char*)data, sizeof(int) + data->data_len);//send token
    if (ret)
    {
        return -1;
    }
    ret = recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));
    if (ret)
    {
        return -1;
    }
    if (data->data_len == -1)
    {
        printf("token verification failed\n");
        return -1;
    }

    data->data_len = strlen(trans_info->cmd) + 1;
    strcpy(data->buf, trans_info->cmd);
    ret = send_cycle(*socketFd, (char*)data, sizeof(int) + data->data_len);//send command
    if (ret)
    {
        return -1;
    }
    ret = recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));
    if (ret)
    {
        return -1;
    }
    if (data->data_len == 1)
    {
        printf("transmission interrupted\n");
        return -1;
    }
    if (data->data_len == 2)
    {
        return 0;
    }
    if (data->data_len == 3)
    {
        return 0;
    }
    if (data->data_len == -1)
    {
        printf("unknown error occurred\n");
        return -1;
    }
    if (data->data_len == -2)
    {
        printf("gets: cannot get: No such file or directory\n");
        return -1;
    }
    if (data->data_len == -3)
    {
        printf("puts: cannot put: File already exist\n");
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
    if (ret)
    {
        close(socketFd);
        pthread_exit(NULL);
    }

    ret = recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));       //recv filename
    if (ret)
    {
        close(socketFd);
        pthread_exit(NULL);
    }
    ret =recv_cycle(socketFd, data.buf, data.data_len);
    if (ret)
    {
        close(socketFd);
        pthread_exit(NULL);
    }
    ret = mkdir("./downloads", 0777);
    if (ret)
    {
        printf("downloads folder already exist\n");
    }
    char path_name[CMD_LEN] = "./downloads/";
    strcat(path_name, data.buf);
    int fd = open(path_name, O_CREAT|O_RDWR, 0666);
    if (fd == -1)
    {
        printf("file creation failed\n");
        close(socketFd);
        pthread_exit(NULL);
    }

    ret = recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));       //recv size
    if (ret)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }
    ret = recv_cycle(socketFd, data.buf, data.data_len);
    if (ret)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }
    off_t size = atoi(data.buf);

    int transfered = 0;
    time_t start, end;
    start = time(NULL);
    printf("\rdownloading... %4.1f%%", (float)transfered / size * 100);
    fflush(stdout);
    while (1)
    {
        ret = recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));
        if (ret)
        {
            close(fd);
            close(socketFd);
            pthread_exit(NULL);
        }
        if (data.data_len > 0)
        {
            ret = recv_cycle(socketFd, data.buf, data.data_len);
            if (ret)
            {
                printf("transmission interrupted\n");
                close(fd);
                close(socketFd);
                pthread_exit(NULL);
            }
            ret = write(fd, data.buf, data.data_len);
            if (ret)
            {
                printf("transmission interrupted\n");
                close(fd);
                close(socketFd);
                pthread_exit(NULL);
            }
            transfered += data.data_len;
            end = time(NULL);
            if (end - start >= 3)
            {
                printf("\rdownloading... %4.1f%%", (float)transfered / size * 100);
                start = end;
                fflush(stdout);
            }
        }
        else
        {
            printf("\rdownloading... %4.1f%%\n", (float)transfered / size * 100);
            printf("download success\n");
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
    if (ret)
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
        printf("file not exist\n");
        close(socketFd);
        pthread_exit(NULL);
    }

    //send md5
    printf("\ruploading...   0.0%%");
    fflush(stdout);
    char file_md5[MD5_LEN] = {0};
    ret = compute_file_md5(fd, file_md5);
    if(ret)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }
    strcpy(data.buf, file_md5);
    data.data_len = strlen(data.buf) + 1;
    ret = send_cycle(socketFd, (char*)&data, data.data_len + sizeof(int));
    if (ret)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }

    ret = recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));       //recv md5 confirm
    if (ret)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }
    ret = recv_cycle(socketFd, data.buf, data.data_len);
    if (ret)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }
    if (data.data_len == -1)        //server cannot connect database
    {
        printf("transmission interrupted\n");
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }
    if (data.data_len == 1)         //file already exist
    {
        printf("\ruploading... 100.0%%\n");
        printf("upload success\n");
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }
    if (data.data_len == 0)
    {
        lseek(fd, 0, SEEK_SET);
    }

    //send filename
    char file_name[FILE_NAME_LEN + 1];
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
    if (ret)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }

    //send file size
    struct stat buf;
    ret = fstat(fd, &buf);
    if (ret)
    {
        perror("fstat");
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }
    off_t file_size = buf.st_size;
    sprintf(data.buf, "%ld", file_size);
    data.data_len = strlen(data.buf) + 1;
    ret = send_cycle(socketFd, (char*)&data, data.data_len + sizeof(int));
    if (ret)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }

    //send file
    int transfered = 0;
    time_t start, end;
    start = time(NULL);
    printf("\ruploading... %4.1f%%", (float)transfered / file_size * 100);
    fflush(stdout);
    while ((data.data_len = read(fd, data.buf, sizeof(data.buf))) > 0)
    {
        ret = send_cycle(socketFd, (char*)&data, data.data_len + sizeof(int));
        transfered += data.data_len;
        end = time(NULL);
        if (end - start >= 3)
        {
            printf("\ruploading... %4.1f%%", (float)transfered / file_size * 100);
            start = end;
            fflush(stdout);
        }
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
    if (ret)
    {
        close(fd);
        close(socketFd);
        pthread_exit(NULL);
    }

    //receive confirmation
    recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));
    if (data.data_len == -1)
    {
        printf("upload failed\n");
    }
    else
    {
        printf("\ruploading... %4.1f%%\n", (float)transfered / file_size * 100);
        printf("upload success\n");
    }
    close(fd);
    close(socketFd);
    pthread_exit(NULL);
}
