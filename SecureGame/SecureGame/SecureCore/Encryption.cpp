
#include "Encryption.h"

Encryption::Encryption() {

}
Encryption::~Encryption() {

}

// key loader
BCRYPT_KEY_HANDLE Encryption::loadPrivateKey(const char* key_pem) {
	DWORD dLen = 0;
	if (!CryptStringToBinaryA(key_pem, 0, CRYPT_STRING_BASE64HEADER, NULL, &dLen, NULL, NULL)) {
		return NULL;
	}

	std::vector<BYTE> buffer(dLen);
	if (!CryptStringToBinaryA(key_pem, 0, CRYPT_STRING_BASE64HEADER, buffer.data(), &dLen, NULL, NULL)) {
		return NULL;
	}

	DWORD blobLen = 0;
	if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY, buffer.data(), dLen, 0, NULL, NULL, &blobLen)) {
		return NULL;
	}

	std::vector<BYTE> blobBuffer(blobLen);
	if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY, buffer.data(), dLen, 0, NULL, blobBuffer.data(), &blobLen)) {
		return NULL;
	}

	BCRYPT_ALG_HANDLE hAlg = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;

	if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RSA_ALGORITHM, NULL, 0) != 0) {
		return NULL;
	}

	if (BCryptImportKeyPair(hAlg, NULL, LEGACY_RSAPRIVATE_BLOB, &hKey, blobBuffer.data(), blobLen, 0) != 0) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return NULL;
	}

	BCryptCloseAlgorithmProvider(hAlg, 0);
	return hKey;
}

BCRYPT_KEY_HANDLE Encryption::loadPublicKey(const char* key_pem) {
	DWORD dLen = 0;
	if (!CryptStringToBinaryA(key_pem, 0, CRYPT_STRING_BASE64HEADER, NULL, &dLen, NULL, NULL)) {
		return NULL;
	}

	std::vector<BYTE> buffer(dLen);
	if (!CryptStringToBinaryA(key_pem, 0, CRYPT_STRING_BASE64HEADER, buffer.data(), &dLen, NULL, NULL)) {
		return NULL;
	}

	DWORD blobLen = 0;
	if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB, buffer.data(), dLen, 0, NULL, NULL, &blobLen)) {
		return NULL;
	}

	std::vector<BYTE> blobBuffer(blobLen);
	if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, RSA_CSP_PUBLICKEYBLOB, buffer.data(), dLen, 0, NULL, blobBuffer.data(), &blobLen)) {
		return NULL;
	}

	BCRYPT_ALG_HANDLE hAlg = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;

	if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RSA_ALGORITHM, NULL, 0) != 0) {
		return NULL;
	}

	if (BCryptImportKeyPair(hAlg, NULL, BCRYPT_RSAPUBLIC_BLOB, &hKey, blobBuffer.data(), blobLen, 0) != 0) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return NULL;
	}

	BCryptCloseAlgorithmProvider(hAlg, 0);
	return hKey;
}

// aes functions
bool Encryption::generateIv(unsigned char* ivBuffer) {
	return BCryptGenRandom(NULL, ivBuffer, IV_SIZE, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
}

int Encryption::AES_encrypt(unsigned char* plaintext, int plaintext_len, const unsigned char* iv, const unsigned char* key, unsigned char* ciphertext) {
	BCRYPT_ALG_HANDLE hAlg = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;

	DWORD cipherLen = 0;
	BYTE localIV[IV_SIZE];
	memcpy(localIV, iv, IV_SIZE);

	if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0) != 0) {
		return -1;
	}

	if (BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0) != 0) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return -1;
	}

	if (BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, (PBYTE)key, SYMKEY_SIZE, 0) != 0) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return -1;
	}

	NTSTATUS status = BCryptEncrypt(hKey, plaintext, plaintext_len, NULL, localIV, sizeof(localIV), ciphertext, PACKET_MAX, &cipherLen, BCRYPT_BLOCK_PADDING);

	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlg, 0);

	return (status == 0) ? (int)cipherLen : -1;

}

int Encryption::AES_decrypt(unsigned char* ciphertext, int ciphertext_len, const unsigned char* iv, const unsigned char* key, unsigned char* plaintext) {
	BCRYPT_ALG_HANDLE hAlg = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;
	DWORD plainLen = 0;
	BYTE localIV[IV_SIZE];
	memcpy(localIV, iv, IV_SIZE);

	if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0) != 0) return -1;

	if (BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0) != 0) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return -1;
	}

	if (BCryptGenerateSymmetricKey(hAlg, &hKey, NULL, 0, (PBYTE)key, SYMKEY_SIZE, 0) != 0) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		return -1;
	}

	NTSTATUS status = BCryptDecrypt(hKey, ciphertext, ciphertext_len, NULL, localIV, sizeof(localIV), plaintext, PACKET_MAX, &plainLen, BCRYPT_BLOCK_PADDING);

	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlg, 0);

	return (status == 0) ? (int)plainLen : -1;
}

// rsa function
bool Encryption::RSA_encrypt_aes_key(BCRYPT_KEY_HANDLE hPubKey, const unsigned char* symKey, size_t symKey_len, unsigned char* enc_sym, size_t* key_len) {
	BCRYPT_OAEP_PADDING_INFO oaepInfo = { 0 };
	oaepInfo.pszAlgId = BCRYPT_SHA256_ALGORITHM;

	DWORD cbResult = 0;
	NTSTATUS status = BCryptEncrypt(hPubKey, (PUCHAR)symKey, (ULONG)symKey_len, &oaepInfo, NULL, 0, enc_sym, RSA_SIGNED_SIZE, &cbResult, BCRYPT_PAD_OAEP);

	if (status == 0) {
		if (key_len) *key_len = cbResult;
		return true;
	}
	return false;
}

bool Encryption::RSA_decrypt_aes_key(BCRYPT_KEY_HANDLE hPrivKey, const unsigned char* enc_sym, size_t encsym_size, unsigned char* sym_key, size_t* symkey_size) {
	BCRYPT_OAEP_PADDING_INFO oaepInfo = { 0 };
	oaepInfo.pszAlgId = BCRYPT_SHA256_ALGORITHM;

	DWORD cbResult = 0;
	NTSTATUS status = BCryptDecrypt(hPrivKey, (PUCHAR)enc_sym, (ULONG)encsym_size, &oaepInfo, NULL, 0, sym_key, SYMKEY_SIZE, &cbResult, BCRYPT_PAD_OAEP);

	if (status == 0) {
		if (symkey_size) *symkey_size = cbResult;
		return true;
	}
	return false;
}

// hash generation
bool Encryption::generateSignedHash(BCRYPT_KEY_HANDLE hPrivKey, envelope* env) {
	BCRYPT_ALG_HANDLE hHashAlg = NULL;
	BCRYPT_HASH_HANDLE hHash = NULL;

	if (BCryptOpenAlgorithmProvider(&hHashAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0) != 0) return false;
	if (BCryptCreateHash(hHashAlg, &hHash, NULL, 0, NULL, 0, 0) != 0) {
		BCryptCloseAlgorithmProvider(hHashAlg, 0);
		return false;
	}

	BCryptHashData(hHash, env->symkey, RSA_SIGNED_SIZE, 0);
	BCryptHashData(hHash, env->iv, IV_SIZE, 0);
	BCryptHashData(hHash, env->packet, env->packet_len, 0);

	BYTE hashValue[HASH_SIZE_BEFORE_SIGN];
	DWORD hashLen = sizeof(hashValue);
	BCryptFinishHash(hHash, hashValue, hashLen, 0);

	BCRYPT_PKCS1_PADDING_INFO paddingInfo = { BCRYPT_SHA256_ALGORITHM };
	DWORD resultLen = 0;

	NTSTATUS status = BCryptSignHash(hPrivKey, &paddingInfo, hashValue, hashLen, env->hash_signed, RSA_SIGNED_SIZE, &resultLen, BCRYPT_PAD_PKCS1);

	BCryptDestroyHash(hHash);
	BCryptCloseAlgorithmProvider(hHashAlg, 0);

	return (status == 0);
}

bool Encryption::verifySignedHash(BCRYPT_KEY_HANDLE hPubKey, const envelope* env) {
	BCRYPT_ALG_HANDLE hHashAlg = NULL;
	BCRYPT_HASH_HANDLE hHash = NULL;

	if (BCryptOpenAlgorithmProvider(&hHashAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0) != 0) return false;
	if (BCryptCreateHash(hHashAlg, &hHash, NULL, 0, NULL, 0, 0) != 0) {
		BCryptCloseAlgorithmProvider(hHashAlg, 0);
		return false;
	}

	BCryptHashData(hHash, (PUCHAR)env->symkey, RSA_SIGNED_SIZE, 0);
	BCryptHashData(hHash, (PUCHAR)env->iv, IV_SIZE, 0);
	BCryptHashData(hHash, (PUCHAR)env->packet, env->packet_len, 0);

	BYTE hashValue[HASH_SIZE_BEFORE_SIGN];
	DWORD hashLen = sizeof(hashValue);
	BCryptFinishHash(hHash, hashValue, hashLen, 0);

	BCRYPT_PKCS1_PADDING_INFO paddingInfo = { BCRYPT_SHA256_ALGORITHM };

	NTSTATUS status = BCryptVerifySignature(hPubKey, &paddingInfo, hashValue, hashLen, (PUCHAR)env->hash_signed, RSA_SIGNED_SIZE, BCRYPT_PAD_PKCS1);

	BCryptDestroyHash(hHash);
	BCryptCloseAlgorithmProvider(hHashAlg, 0);

	return (status == 0);
}

// envelope functions
bool Encryption::buildEnvelope(unsigned char* plaintext, int plaintext_len, BCRYPT_KEY_HANDLE hSenderPrivKey, BCRYPT_KEY_HANDLE hReceiverPubKey, envelope* packet_buffer) {
	if (!generateIv(packet_buffer->iv)) return false;

	// Encrypt payload with AES-256-CBC
	int cipher_len = AES_encrypt(plaintext, plaintext_len, packet_buffer->iv, (const unsigned char*)test_key, packet_buffer->packet);
	if (cipher_len < 0) return false;
	packet_buffer->packet_len = (uint16_t)cipher_len;

	// Encrypt AES Symmetric Key with Sender's RSA Public Key
	size_t key_len = 0;
	if (!RSA_encrypt_aes_key(hReceiverPubKey, (const unsigned char*)test_key, SYMKEY_SIZE, packet_buffer->symkey, &key_len)) {
		return false;
	}

	// Sign the whole bundle
	return generateSignedHash(hSenderPrivKey, packet_buffer);
}

bool Encryption::resolveEnvelope(const envelope* env, BCRYPT_KEY_HANDLE hReceiverPrivKey, BCRYPT_KEY_HANDLE hSenderPubKey, unsigned char* dec_data, size_t max_dec_len) {
	// Verify Sender Signature
	if (!verifySignedHash(hSenderPubKey, env)) {
		return false;
	}

	// Decrypt AES Key using Receiver's RSA Private Key
	BYTE decrypted_sym_key[SYMKEY_SIZE];
	size_t sym_len = 0;
	if (!RSA_decrypt_aes_key(hReceiverPrivKey, env->symkey, RSA_SIGNED_SIZE, decrypted_sym_key, &sym_len)) {
		return false;
	}

	// Decrypt AES Payload
	int dec_len = AES_decrypt((unsigned char*)env->packet, env->packet_len, env->iv, decrypted_sym_key, dec_data);
	return (dec_len > 0);
}
