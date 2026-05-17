#include "Global.h"


extern "C" __declspec(dllexport) void* CALLBACK GameTick(PVOID context)
{

    if (!context) {
        return nullptr;
    }
    EnclaveInput*input  = static_cast<EnclaveInput*>(context);
    Encryption enc = Encryption();


    if(input->isEncrypt) {
        unsigned char ciphertext[64];
        unsigned char plaintext[64];
        int plaintext_len = sizeof(playerMV);

		memcpy(plaintext, input->buffer, plaintext_len);

		int ciphertext_len = enc.AES_encrypt(plaintext, plaintext_len, ciphertext);
        if (ciphertext_len < 0) {
            return nullptr;
        }

        memset(input->buffer, 0, 64);
		memcpy(input->buffer, ciphertext, ciphertext_len);
        input->isEncrypt = false;
	}
    else {
        unsigned char plaintext[64];
		if (input->cipherLen <= 0) {
            return nullptr;
        }
        int plaintext_len = enc.AES_decrypt((unsigned char*)input->buffer, input->cipherLen, plaintext);
        if (plaintext_len < 0) {
            return nullptr;
        }

        memset(input->buffer, 0, sizeof(64));
        memcpy(input->buffer, plaintext, plaintext_len);
    }

    return nullptr;
}