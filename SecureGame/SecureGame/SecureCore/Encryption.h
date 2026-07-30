#ifndef __ENCRYPT_H__
#define __ENCRYPT_H__


#include <Windows.h>
#include <bcrypt.h>
#include <stdio.h>
#include <stdint.h>

#include "../SecureGame/Shared.h"


#pragma comment(lib, "Bcrypt.lib")
#pragma warning(disable:4996)

extern const char* test_key;
extern const char* test_iv;

#define RSA_SIGNED_SIZE 256
#define PACKET_MAX 64
#define SYMKEY_SIZE 32
#define IV_SIZE 16
#define HASH_SIZE_BEFORE_SIGN 32

struct RSA_PRIVATE_PARAMS {
    BYTE* mod_p;
    ULONG mod_bytes;
    BYTE* exp_p;
    ULONG exp_bytes;
    BYTE* priv_exp_p;
    ULONG priv_exp_bytes;
    BYTE* p;
    ULONG p_bytes;
    BYTE* q;
    ULONG q_bytes;
    BYTE* dp;
    ULONG dp_bytes;
    BYTE* dq;
    ULONG dq_bytes;
    BYTE* inv;
    ULONG inv_bytes;
};


class Encryption {
private:
    public:
    Encryption();
    ~Encryption();

    // key loader
    bool loadPrivateKey(const char*, BCRYPT_KEY_HANDLE*);
    bool loadPublicKey(const char*, BCRYPT_KEY_HANDLE*);

    // load key helper functions
    bool extractRsaParamsFromDerPublic(const BYTE*, size_t, BYTE**, ULONG*, BYTE**, ULONG*);
    bool ExtractRsaParamsFromDerPrivate(const BYTE*, size_t, RSA_PRIVATE_PARAMS*);

    // base64 decode
    size_t base64Decode(const char*, size_t, BYTE*);

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