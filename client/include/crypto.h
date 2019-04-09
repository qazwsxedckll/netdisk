#ifndef __CRYTO_H__
#define __CRYTO_H__
#include "head.h"
char* rsa_encrypt(char* str);

char* rsa_sign(char* str, const char* user_name);

char* rsa_decrypt(char* str, const char* user_name);

char* rsa_verify(char* str);

int rsa_generate_key(const char* user_name);
#endif
