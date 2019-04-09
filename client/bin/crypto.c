#include "../include/crypto.h"

int rsa_generate_key()
{
    int ret;
    ret = access("client_rsa.key", F_OK);
    if (ret)
    {
        system("openssl genrsa -out client_rsa.key 2048");
    }
    ret = access("client_rsa.key", F_OK);
    if (ret)
    {
        printf("key generation fail, check if openssl is installed\n");
        return -1;
    }
    ret = access("client_rsa_pub.key", F_OK);
    if (ret)
    {
        system("openssl rsa -in client_rsa.key -pubout -out client_rsa_pub.key");
    }
    ret = access("client_rsa_pub.key", F_OK);
    if (ret)
    {
        printf("key generation fail, check if openssl is installed\n");
        return -1;
    }
    return 0;
}

char* rsa_encrypt(char* str)
{
    int ret;
    char* en_str;;
    FILE* fp;

    fp = fopen("server_rsa_pub.key", "rb");
    if (fp == NULL)
    {
        printf("server_rsa_pub.key not found\n");
        return NULL;
    }

    RSA* rsa;

    rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
    if (rsa == NULL)
    {
        printf("rsa_pub_key read failed\n");
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }

    en_str = (char*)calloc(SER_EN_LEN, sizeof(char));
    ret = RSA_public_encrypt(RSA_EN_LEN, (unsigned char*)str, (unsigned char*)en_str, rsa, RSA_PKCS1_PADDING);
    if (ret == -1)
    {
        printf("rsa_encrypt failed\n");
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }

    fclose(fp);
    RSA_free(rsa);
    return en_str;
}

char* rsa_sign(char* str)
{
    int ret;
    char* en_str;
    FILE* fp;

    fp = fopen("client_rsa.key", "rb");
    if (fp == NULL)
    {
        printf("client_rsa.key not found\n");
        return NULL;
    }

    RSA* rsa;

    rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
    if (rsa == NULL)
    {
        printf("rsa_private_key read failed\n");
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }

    int len = strlen(str);
    en_str = (char*)calloc(RSA_EN_LEN, sizeof(char));
    ret = RSA_private_encrypt(len, (unsigned char*)str, (unsigned char*)en_str, rsa, RSA_PKCS1_PADDING);
    if (ret == -1)
    {
        printf("rsa sign failed\n");
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }

    fclose(fp);
    RSA_free(rsa);
    return en_str;
}

char* rsa_decrypt(char* str)
{
    int ret;
    char* de_str;
    FILE* fp;

    fp = fopen("client_rsa.key", "rb");
    if (fp == NULL)
    {
        printf("client_rsa.key not found\n");
        return NULL;
    }

    RSA* rsa;

    rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
    if (rsa == NULL)
    {
        printf("rsa_private_key read failed\n");
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }

    de_str = (char*)calloc(RSA_DE_LEN, sizeof(char));
    ret = RSA_private_decrypt(RSA_EN_LEN, (unsigned char*)str, (unsigned char*)de_str, rsa, RSA_PKCS1_PADDING);
    if (ret == -1)
    {
        printf("rsa sign failed\n");
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }

    fclose(fp);
    RSA_free(rsa);
    return de_str;
}

char* rsa_verify(char* str)
{
    int ret;
    char* de_str;

    FILE* fp = fopen("server_rsa_pub.key", "rb");
    if (fp == NULL)
    {
        printf("server_rsa_pub.key not found\n");
        return NULL;
    }

    RSA* rsa;
    rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
    if (rsa == NULL)
    {
        printf("server_pub_key read failed\n");
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }

    de_str = (char*)calloc(SER_DE_LEN, sizeof(char));
    ret = RSA_public_decrypt(SER_EN_LEN, (unsigned char*)str, (unsigned char*)de_str, rsa, RSA_PKCS1_PADDING);
    if (ret == -1)
    {
        printf("decrption failed\n");
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }

    fclose(fp);
    RSA_free(rsa);
    return de_str;
}
