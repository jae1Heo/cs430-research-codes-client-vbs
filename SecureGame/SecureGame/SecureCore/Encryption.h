#ifndef __ENCRYPT_H__
#define __ENCRYPT_H__


#include <Windows.h>
#include <bcrypt.h>
#include <wincrypt.h>
#include <cstdint>
#include <vector>
#include <iostream>

#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "crypt32.lib")
#pragma warning(disable:4996)

extern const char* test_key;
extern const char* test_iv;

#define RSA_SIGNED_SIZE 256
#define PACKET_MAX 64
#define SYMKEY_SIZE 32
#define IV_SIZE 16
#define HASH_SIZE_BEFORE_SIGN 32

#pragma pack(push, 1)
typedef struct {
    unsigned char symkey[RSA_SIGNED_SIZE];
    unsigned char iv[IV_SIZE];
    unsigned char packet[PACKET_MAX];
    uint16_t packet_len;
    unsigned char hash_signed[RSA_SIGNED_SIZE];
}envelope;
#pragma pack(pop)

class Encryption {
private:
    public:
    Encryption();
    ~Encryption();

    // key loader
    BCRYPT_KEY_HANDLE loadPrivateKey(const char*);
    BCRYPT_KEY_HANDLE loadPublicKey(const char*);

    // aes functions
    bool generateIv(unsigned char*);
    int AES_encrypt(unsigned char*, int, const unsigned char*, const unsigned char*, unsigned char*);
    int AES_decrypt(unsigned char*, int, const unsigned char*, const unsigned char*, unsigned char*);

    // rsa functions
    bool RSA_encrypt_aes_key(BCRYPT_KEY_HANDLE, const unsigned char*, size_t, unsigned char*, size_t*);
    bool RSA_decrypt_aes_key(BCRYPT_KEY_HANDLE, const unsigned char*, size_t, unsigned char*, size_t*);

    // hash generation
    bool generateSignedHash(BCRYPT_KEY_HANDLE, envelope*);
    bool verifySignedHash(BCRYPT_KEY_HANDLE, const envelope*);

    // envelope functions
    bool buildEnvelope(unsigned char*, int, BCRYPT_KEY_HANDLE, BCRYPT_KEY_HANDLE, envelope*);
    bool resolveEnvelope(const envelope*, BCRYPT_KEY_HANDLE, BCRYPT_KEY_HANDLE, unsigned char*, size_t);
    
};




#endif