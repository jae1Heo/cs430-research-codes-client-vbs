#ifndef __ENCRYPT_H__
#define __ENCRYPT_H__


#include <Windows.h>
#include <bcrypt.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "Bcrypt.lib")
#pragma warning(disable:4996)

extern const char* test_key;
extern const char* test_iv;

class Encryption {
private:
    public:
    Encryption();
    ~Encryption();
    int AES_encrypt(unsigned char*, int, unsigned char*);
    int AES_decrypt(unsigned char*, int, unsigned char*);
    
};




#endif