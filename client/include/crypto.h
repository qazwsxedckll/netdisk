#ifndef __CRYTO_H__
#define __CRYTO_H__
#include "head.h"
char* rsa_encrypt(char* str);

char* rsa_sign(char* str);

char* rsa_decrypt(char* str);

char* rsa_verify(char* str);

int rsa_generate_key();
#endif
