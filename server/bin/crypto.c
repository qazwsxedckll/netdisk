#include "../include/crypto.h"

char* rsa_encrypt(char* str, char* user_name)
{
    int ret;
    char* en_str;;

    char pk_path[RESULT_LEN];
    sprintf(pk_path, "keys/%s_%s.key", user_name, "pub");
    FILE* fp = fopen(pk_path, "rb");
    if (fp == NULL)
    {
#ifdef _DEBUG
        printf("user_pub.key not found\n");
#endif
        return NULL;
    }

    RSA* rsa;

    rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
    if (rsa == NULL)
    {
#ifdef _DEBUG
        printf("user_pub_key read failed\n");
#endif
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }

    int len = strlen(str);
    en_str = (char*)calloc(RSA_EN_LEN, sizeof(char));
    ret = RSA_public_encrypt(len, (unsigned char*)str, (unsigned char*)en_str, rsa, RSA_PKCS1_PADDING);
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
    char* en_str;;
    FILE* fp;

    fp = fopen("server_rsa.key", "rb");
    if (fp == NULL)
    {
#ifdef _DEBUG
        printf("server_rsa.key not found\n");
#endif
        return NULL;
    }

    RSA* rsa;

    rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
    if (rsa == NULL)
    {
#ifdef _DEBUG
        printf("rsa_private_key read failed\n");
#endif
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }

    int len = strlen(str);
    en_str = (char*)calloc(SER_EN_LEN, sizeof(char));
    ret = RSA_private_encrypt(len, (unsigned char*)str, (unsigned char*)en_str, rsa, RSA_PKCS1_PADDING);
    if (ret == -1)
    {
#ifdef _DEBUG
        printf("rsa sign failed\n");
#endif
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

    FILE* fp = fopen("server_rsa.key", "rb");
    if (fp == NULL)
    {
#ifdef _DEBUG
        printf("server_rsa.key not found\n");
#endif
        return NULL;
    }

    RSA* rsa;
    rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
    if (rsa == NULL)
    {
#ifdef _DEBUG
        printf("rsa_private_key read failed\n");
#endif
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }
    de_str = (char*)calloc(SER_DE_LEN, sizeof(char));
    ret = RSA_private_decrypt(SER_EN_LEN, (unsigned char*)str, (unsigned char*)de_str, rsa, RSA_PKCS1_PADDING);
    if (ret == -1)
    {
#ifdef _DEBUG
        ERR_print_errors_fp(stdout);
        printf("rsa_decrypt failed\n");
#endif
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }

    fclose(fp);
    RSA_free(rsa);
    return de_str;
}

char* rsa_verify(char* str, char* user_name)
{
    int ret;
    char* de_str;

    char pk_path[RESULT_LEN];
    sprintf(pk_path, "keys/%s_%s.key", user_name, "pub");
    FILE* fp = fopen(pk_path, "rb");
    if (fp == NULL)
    {
#ifdef _DEBUG
        printf("user pub key not found\n");
#endif
        return NULL;
    }

    RSA* rsa;
    rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
    if (rsa == NULL)
    {
#ifdef _DEBUG
        printf("user pub key read failed\n");
#endif
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }
    de_str = (char*)calloc(RSA_DE_LEN, sizeof(char));
    ret = RSA_public_decrypt(RSA_EN_LEN, (unsigned char*)str, (unsigned char*)de_str, rsa, RSA_PKCS1_PADDING);
    if (ret == -1)
    {
#ifdef _DEBUG
        ERR_print_errors_fp(stdout);
        printf("decryption failed\n");
#endif
        fclose(fp);
        RSA_free(rsa);
        return NULL;
    }

    fclose(fp);
    RSA_free(rsa);
    return de_str;
}
