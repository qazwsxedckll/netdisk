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

int send_nonce(int fd, DataPackage* data)
{
    char nonce[15];
    srand((unsigned)(time(NULL)));
    sprintf(nonce, "%d", rand());
    strcpy(data->buf, nonce);
    data->data_len = strlen(data->buf) + 1;
    if (send_cycle(fd, (char*)data, data->data_len + sizeof(int)))   //send nonce
    {
        return -1;
    }
    if (recv_cycle(fd, (char*)&data->data_len, sizeof(int)))        //recv nonce
    {
        return -1;
    }
    if (recv_cycle(fd, data->buf, data->data_len))
    {
        return -1;
    }
    char* nonce_tmp;
    nonce_tmp = rsa_verify(data->buf);         //verify
    if (nonce_tmp == NULL)
    {
        return -1;
    }
    if (strcmp(nonce_tmp, nonce) != 0)
    {
        free(nonce_tmp);
        nonce_tmp = NULL;
        printf("nonce verification failed\n");
        return -1;
    }
    free(nonce_tmp);
    nonce_tmp = NULL;
    return 0;
}

int recv_nonce(int fd, DataPackage* data, const char* user_name)
{
    char* nonce_tmp;
    if (recv_cycle(fd, (char*)&data->data_len, sizeof(int)))        //recv nonce
    {
        return -1;
    }
    if (recv_cycle(fd, data->buf, data->data_len))
    {
        return -1;
    }
    nonce_tmp = rsa_sign(data->buf, user_name);
    if (nonce_tmp == NULL)
    {
        return -1;
    }
    memcpy(data->buf, nonce_tmp, RSA_EN_LEN);
    free(nonce_tmp);
    nonce_tmp = NULL;
    data->data_len = RSA_EN_LEN;
    if (send_cycle(fd, (char*)data, data->data_len + sizeof(int)))
    {
        return -1;
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

    //input password
    char* password;
    flag = -1;
    while (flag == -1)
    {
        flag = 0;
        system("clear");
        printf("Enter '0' to go back\n");
        printf("Enter invitation code: %s\n", invi_code);
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
            flag = -1;
        }
        else
        {
            break;
        }
    }

    if (strcmp(password, "0") == 0)
    {
        return -1;
    }

    ret = rsa_generate_key(user_name);
    if (ret)
    {
        return -1;
    }

    //connect to server
    ret = connect_server(socketFd, ip, port);
    if (ret)
    {
        close(*socketFd);
        return -1;
    }
    data->data_len = 6;
    ret = send_cycle(*socketFd, (char*)data, sizeof(int));        //6 for regi password
    if (ret)
    {
        close(*socketFd);
        return -1;
    }

    strcpy(data->buf, user_name);
    data->data_len = strlen(data->buf) + 1;
    ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send user_name
    if (ret)
    {
        close(*socketFd);
        return -1;
    }

    ret = send_nonce(*socketFd, data);              //send nonce
    if (ret)
    {
        close(*socketFd);
        return -1;
    }
    char pk_path[FILE_NAME_LEN];                    //send pk
    sprintf(pk_path, "%s_rsa_pub.key", user_name);
    int pkfd = open(pk_path, O_RDONLY);
    if (pkfd == -1)
    {
        perror("open");
        close(*socketFd);
        return -1;
    }
    while ((data->data_len = read(pkfd, data->buf, sizeof(data->buf))) > 0)
    {
        ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));
        if (ret == -1)
        {
            close(pkfd);
            return -1;
        }
    }
    close(pkfd);
    //send end of transmission
    data->data_len = 0;
    ret = send_cycle(*socketFd, (char*)data, sizeof(int));
    if (ret)
    {
        close(*socketFd);
        return -1;
    }

    char *en_pass = rsa_encrypt(password);
    free(password);
    password = NULL;
    if (en_pass == NULL)
    {
        close(*socketFd);
        return -1;
    }
    memcpy(data->buf, en_pass, SER_EN_LEN);
    free(en_pass);
    en_pass = NULL;
    data->data_len = SER_EN_LEN;
    ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send password
    if (ret)
    {
        close(*socketFd);
        return -1;
    }

    ret = recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));         //recv comfirmation
    if (ret)
    {
        close(*socketFd);
        return -1;
    }

    if (data->data_len == 0)
    {
        flag = 0;
        close(*socketFd);
    }
    else if (data->data_len == -1)
    {
        close(*socketFd);
        return -1;
    }
    return 0;
}

int tran_authen(int* socketFd, const char* ip, const char* port, char* user_name, DataPackage* data, TransInfo* trans_info)
{
    int flag, ret, err;
    flag = -1;
    err = 0;
    char* password;

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
            if (err == -3)
            {
                printf("nonce verification failed\n");
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

        char pk_path[FILE_NAME_LEN];
        sprintf(pk_path, "%s_rsa.key", user_name);
        if (access(pk_path, F_OK)  == 0)
        {
            ret = connect_server(socketFd, ip, port);
            if (ret)
            {
                continue;
            }

            data->data_len = 1;
            ret = send_cycle(*socketFd, (char*)data, sizeof(int));        //1 for no pwd login
            if (ret)
            {
                close(*socketFd);
                continue;
            }

            strcpy(data->buf, user_name);
            data->data_len = strlen(data->buf) + 1;
            ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send user_name
            if (ret)
            {
                close(*socketFd);
                continue;
            }

            ret = send_nonce(*socketFd, data);          //send nonce
            if (ret)
            {
                close(*socketFd);
                err = -3;
                continue;
            }

            ret = recv_nonce(*socketFd, data, user_name);
            if (ret)
            {
                close(*socketFd);
                continue;
            }

            ret = recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));   //recv comfirm
            if (ret)
            {
                close(*socketFd);
                continue;
            }
            if (data->data_len == 0)
            {
                break;
            }
        }

        //input password
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
            free(password);
            password = NULL;
            return -1;
        }
        strcpy(trans_info->password, password);

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
            close(*socketFd);
            continue;
        }

        strcpy(data->buf, user_name);
        data->data_len = strlen(data->buf) + 1;
        ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send user_name
        if (ret)
        {
            close(*socketFd);
            continue;
        }

        ret = send_nonce(*socketFd, data);          //send nonce
        if (ret)
        {
            close(*socketFd);
            err = -3;
            continue;
        }

        char *en_pass = rsa_encrypt(password);
        free(password);
        password = NULL;
        if (en_pass == NULL)
        {
            close(*socketFd);
            continue;
        }
        memcpy(data->buf, en_pass, SER_EN_LEN);
        free(en_pass);
        en_pass = NULL;
        data->data_len = SER_EN_LEN;
        ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send password
        if (ret)
        {
            close(*socketFd);
            continue;
        }

        ret = recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));         //recv comfirm
        if (ret)
        {
            close(*socketFd);
            continue;
        }

        if (data->data_len == -1)
        {
            close(*socketFd);
            err = -2;
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

int thread_connect(int* socketFd, DataPackage* data, TransInfo* trans_info, int code, const char* password)
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

    strcpy(data->buf, trans_info->user_name);
    data->data_len = strlen(data->buf) + 1;
    ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int)); //send username
    if (ret)
    {
        return -1;
    }

    char pk_path[FILE_NAME_LEN];
    sprintf(pk_path, "%s_rsa.key", trans_info->user_name);
    ret = access(pk_path, F_OK);
    if (ret)
    {
        data->data_len = -1;
        ret = send_cycle(*socketFd, (char*)data, sizeof(int));        //send no key
        if (ret)
        {
            return -1;
        }
        char *en_pass = rsa_encrypt(password);
        if (en_pass == NULL)
        {
            return -1;
        }
        memcpy(data->buf, en_pass, SER_EN_LEN);
        free(en_pass);
        en_pass = NULL;
        data->data_len = SER_EN_LEN;
        ret = send_cycle(*socketFd, (char*)data, data->data_len + sizeof(int));   //send password
        if (ret)
        {
            return -1;
        }

        ret = recv_cycle(*socketFd, (char*)&data->data_len, sizeof(int));      //recv confirm
        if (ret)
        {
            return -1;
        }
        if (data->data_len == -1)
        {
            printf("verification failed\n");
            return -1;
        }
    }
    else
    {
        data->data_len = 0;
        ret = send_cycle(*socketFd, (char*)data, sizeof(int));        //send with key
        if (ret)
        {
            return -1;
        }
        ret = recv_nonce(*socketFd, data, trans_info->user_name);
        if (ret)
        {
            return -1;
        }
    }

    ret = send_nonce(*socketFd, data);
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
        printf("verification failed\n");
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
    if (data->data_len == 0)
    {
        return 0;
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

int get_file(int socketFd)
{
    int ret;
    DataPackage data;
    ret = recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));       //recv filename
    if (ret)
    {
        return -1;
    }
    ret =recv_cycle(socketFd, data.buf, data.data_len);
    if (ret)
    {
        return -1;
    }
    int fd = open(data.buf, O_WRONLY|O_TRUNC|O_CREAT, 0664);
    if (fd == -1)
    {
        printf("file creation failed\n");
        return -1;
    }

    ret = recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));       //recv size
    if (ret)
    {
        close(fd);
        return -1;
    }
    ret = recv_cycle(socketFd, data.buf, data.data_len);
    if (ret)
    {
        close(fd);
        return -1;
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
            return -1;
        }
        if (data.data_len > 0)
        {
            ret = recv_cycle(socketFd, data.buf, data.data_len);
            if (ret)
            {
                printf("transmission interrupted\n");
                close(fd);
                return -1;
            }
            ret = write(fd, data.buf, data.data_len);
            if (ret == -1)
            {
                perror("write");
                printf("transmission interrupted\n");
                close(fd);
                return -1;
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
    return 0;
}

int get_dir(int socketFd, char* cur_path)
{
    DataPackage data;
    while (1)
    {
        if (recv_cycle(socketFd, (char*)&data.data_len, sizeof(int)))
        {
            return -1;
        }

        if (data.data_len == 0)
        {
            if (get_file(socketFd))
            {
                return -1;
            }
        }
        else if (data.data_len == 1)
        {
            if (recv_cycle(socketFd, (char*)&data.data_len, sizeof(int)))   //recv dir name
            {
                return -1;
            }
            if (recv_cycle(socketFd, data.buf, data.data_len))
            {
                return -1;
            }
            if (mkdir(data.buf, 0755))
            {
                return -1;
            }
            char cur_path[CMD_LEN];
            getcwd(cur_path, sizeof(cur_path));
            chdir(data.buf);
            get_dir(socketFd, cur_path);
        }
        else
        {
            chdir(cur_path);
            return 0;
        }
    }
}

void* get_files(void* p)
{
    int socketFd;
    DataPackage data;
    TransInfo* trans_info = (TransInfo*)p;
    if (thread_connect(&socketFd, &data, trans_info, 2, trans_info->password))
    {
        close(socketFd);
        pthread_exit(NULL);
    }

    mkdir("downloads", 0775);
    char path_name[CMD_LEN];
    sprintf(path_name, "%s/%s", "downloads", trans_info->user_name);
    mkdir(path_name, 0775);

    char main_path[CMD_LEN];
    getcwd(main_path, sizeof(main_path));

    if (recv_cycle(socketFd, (char*)&data.data_len, sizeof(int)))
    {
        close(socketFd);
        pthread_exit(NULL);
    }
    if (data.data_len == 0)             //0 for file
    {
        chdir(path_name);
        get_file(socketFd);
        chdir(main_path);
        close(socketFd);
        pthread_exit(NULL);
    }
    else                                //1 for dir
    {
        if (recv_cycle(socketFd, (char*)&data.data_len, sizeof(int)))   //recv dir name
        {
            close(socketFd);
            pthread_exit(NULL);
        }
        if (recv_cycle(socketFd, data.buf, data.data_len))
        {
            close(socketFd);
            pthread_exit(NULL);
        }

        char file_path[CMD_LEN + 100];
        sprintf(file_path, "downloads/%s/%s", trans_info->user_name, data.buf);
        if (mkdir(file_path, 0755))
        {
            printf("cannot mkdir\n");
            close(socketFd);
            pthread_exit(NULL);
        }

        chdir(file_path);
        char cur_path[CMD_LEN];
        getcwd(cur_path, sizeof(cur_path));
        if (get_dir(socketFd, cur_path))
        {
            chdir(cur_path);
            close(socketFd);
            pthread_exit(NULL);
        }
    printf("All files are downloaded\n");
    chdir(main_path);
    close(socketFd);
    pthread_exit(NULL);
    }
}

int put_file(int socketFd, const char* file_name)
{
    int ret;
    int fd = open(file_name, O_RDONLY);
    DataPackage data;
    if (fd == -1)
    {
        printf("file not exist\n");
        return -1;
    }

    //send md5
    printf("\ruploading... %4.1f%%", 0.0);
    fflush(stdout);
    char file_md5[MD5_LEN] = {0};
    ret = compute_file_md5(fd, file_md5);
    if(ret)
    {
        close(fd);
        return -1;
    }
    strcpy(data.buf, file_md5);
    data.data_len = strlen(data.buf) + 1;
    ret = send_cycle(socketFd, (char*)&data, data.data_len + sizeof(int));
    if (ret)
    {
        close(fd);
        return -1;
    }

    ret = recv_cycle(socketFd, (char*)&data.data_len, sizeof(int));       //recv md5 confirm
    if (ret)
    {
        close(fd);
        return -1;
    }
    ret = recv_cycle(socketFd, data.buf, data.data_len);
    if (ret)
    {
        close(fd);
        return -1;
    }
    if (data.data_len == -1)        //server cannot connect database
    {
        printf("transmission interrupted\n");
        close(fd);
        return -1;
    }
    if (data.data_len == 1)         //file already exist
    {
        printf("\ruploading... 100.0%%\n");
        printf("upload success\n");
        close(fd);
        return -1;
    }
    if (data.data_len == 0)
    {
        lseek(fd, 0, SEEK_SET);
    }

    //send filename
    data.data_len = strlen(file_name) + 1;
    strcpy(data.buf, file_name);
    ret = send_cycle(socketFd, (char*)&data, data.data_len + sizeof(int));
    if (ret)
    {
        close(fd);
        return -1;
    }

    //send file size
    struct stat buf;
    ret = fstat(fd, &buf);
    if (ret)
    {
        perror("fstat");
        close(fd);
        return -1;
    }
    off_t file_size = buf.st_size;
    sprintf(data.buf, "%ld", file_size);
    data.data_len = strlen(data.buf) + 1;
    ret = send_cycle(socketFd, (char*)&data, data.data_len + sizeof(int));
    if (ret)
    {
        close(fd);
        return -1;
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
            return -1;
        }
    }

    //send end of transmission
    data.data_len = 0;
    ret = send_cycle(socketFd, (char*)&data, sizeof(int));
    if (ret)
    {
        close(fd);
        return -1;
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
    return 0;
}

int put_dir(DIR* dp, int socketFd, char* cur_path)
{
    DataPackage data;
    struct dirent *entry;
    struct stat stat_buf;
    while ((entry = readdir(dp)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        stat(entry->d_name, &stat_buf);
        if (S_ISDIR(stat_buf.st_mode))
        {
            data.data_len = 0;
            if (send_cycle(socketFd, (char*)&data, sizeof(int)))
            {
                return -1;
            }

            strcpy(data.buf, entry->d_name);
            data.data_len = strlen(data.buf) + 1;
            if (send_cycle(socketFd, (char*)&data, data.data_len + sizeof(int)))
            {
                return -1;
            }

            DIR* dp = opendir(entry->d_name);
            char cur_path[FILE_NAME_LEN];
            getcwd(cur_path, sizeof(cur_path));
            chdir(entry->d_name);
            if (put_dir(dp, socketFd, cur_path))
            {
                return -1;
            }
        }
        else
        {
            data.data_len = 1;
            if (send_cycle(socketFd, (char*)&data, sizeof(int)))
            {
                return -1;
            }
            if (put_file(socketFd, entry->d_name))
            {
                return -1;
            }
        }
    }
    data.data_len = 2;
    if (send_cycle(socketFd, (char*)&data, sizeof(int)))
    {
        return -1;
    }
    chdir(cur_path);
    closedir(dp);
    return 0;
}

void* put_files(void* p)
{
    int ret, socketFd;
    DataPackage data;
    TransInfo* trans_info = (TransInfo*)p;

    char file_path[CMD_LEN] = { 0 };
    char file_name[FILE_NAME_LEN] = { 0 };
    char file_dir[FILE_NAME_LEN] = { 0 };
    int i = 0;
    while (trans_info->cmd[i + 5] != '\0')
    {
        file_path[i] = trans_info->cmd[i + 5];
        i++;
    }

    i = strlen(file_path);
    while (file_path[i] != '/' && i != -1)
    {
        i--;
    }
    if (i == -1)
    {
        file_dir[0] = '.';
    }
    else
    {
        int j = 0;
        while (j != i)
        {
            file_dir[j] = file_path[j];
            j++;
        }
    }
    i++;
    int k = 0;
    while (file_path[i] != '\0')
    {
        file_name[k++] = file_path[i++];
    }

    //connection
    ret = thread_connect(&socketFd, &data, trans_info, 3, trans_info->password);
    if (ret)
    {
        close(socketFd);
        pthread_exit(NULL);
    }

    char cur_path[FILE_NAME_LEN];
    getcwd(cur_path, sizeof(cur_path));
    struct stat buf;
    if (stat(file_path, &buf))
    {
        printf("file not exist\n");
        pthread_exit(NULL);
    }
    if (S_ISDIR(buf.st_mode))
    {
        DIR* dp = opendir(file_path);

        if (dp == NULL)
        {
            printf("cannot open directory\n");
            close(socketFd);
            pthread_exit(NULL);
        }
        if (chdir(file_dir))
        {
            printf("cannot open directory\n");
            close(socketFd);
            pthread_exit(NULL);
        }

        data.data_len = 0;
        if (send_cycle(socketFd, (char*)&data, sizeof(int)))
        {
            close(socketFd);
            pthread_exit(NULL);
        }

        strcpy(data.buf, file_name);
        data.data_len = strlen(file_name) + 1;
        if (send_cycle(socketFd, (char*)&data, sizeof(int) + data.data_len))    //send dir name
        {
            close(socketFd);
            pthread_exit(NULL);
        }

        if (chdir(file_path))
        {
            printf("cannot open directory\n");
            close(socketFd);
            pthread_exit(NULL);
        }

        put_dir(dp, socketFd, cur_path);
        close(socketFd);
        pthread_exit(NULL);

    }
    else
    {
        if (chdir(file_dir))
        {
            printf("cannot open directory\n");
            close(socketFd);
            pthread_exit(NULL);
        }

        data.data_len = 1;
        if (send_cycle(socketFd, (char*)&data, sizeof(int)))
        {
            close(socketFd);
            pthread_exit(NULL);
        }

        put_file(socketFd, file_name);
        chdir(cur_path);
        close(socketFd);
        pthread_exit(NULL);
    }
}
