
#include "Encryption.h"

const char* test_key = "abcdefghijklmnopqrstuvwxyzzyxwvu";
const char* test_iv = "zyxwvutsabcdefgh";

Encryption::Encryption() {

}
Encryption::~Encryption() {

}

int Encryption::AES_encrypt(unsigned char* plaintext, int plaintext_len, unsigned char* ciphertext) {
	BCRYPT_ALG_HANDLE hAlg = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;

	DWORD ciphertextLen = 0;
	unsigned char localIV[16];

	// copy IV to local variable since BCryptEncrypt will modify it
	memcpy(localIV, test_iv, 16);

	// open algorithm provider
	BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);

	// set chaining mode to CBC
	BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);

	// generate symmetric key from the test key
	BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, (PBYTE)test_key, 32, 0);

	// encrypt the plaintext
	// append PKCS#7 padding to the plaintext, so the ciphertext buffer must be large enough to hold the padded plaintext + IV
	if (BCryptEncrypt(hKey, plaintext, plaintext_len, NULL, localIV, 16, ciphertext, plaintext_len + 16, &ciphertextLen, BCRYPT_BLOCK_PADDING) != 0) {
		return -1;
	}

	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlg, 0);

	return (int)ciphertextLen;

}
int Encryption::AES_decrypt(unsigned char* ciphertext, int ciphertext_len, unsigned char* plaintext) {
	BCRYPT_ALG_HANDLE hAlg = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;

	DWORD plaintextLen = 0;
	unsigned char localIV[16];

	// copy IV to local variable since BCryptDecrypt will modify it
	memcpy(localIV, test_iv, 16);

	// open algorithm provider
	BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0);

	// set chaining mode to CBC
	BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
	
	// generate symmetric key from the test key 
	BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, (PBYTE)test_key, 32, 0);

	// decrypt the ciphertext
	// the plaintext buffer must be large enough to hold the decrypted plaintext
	NTSTATUS status = BCryptDecrypt(hKey, ciphertext, ciphertext_len, NULL, localIV, 16, plaintext, ciphertext_len, &plaintextLen, BCRYPT_BLOCK_PADDING);

	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlg, 0);

	if (status == 0) {
		return (int)plaintextLen;
	}
	else {
		return -1;
	}
}