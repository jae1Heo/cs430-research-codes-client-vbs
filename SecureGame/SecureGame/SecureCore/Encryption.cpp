
#include "Encryption.h"

Encryption::Encryption(int client_number) {
	this->key = hardcoded_keys[client_number];
	this->iv = hardcoded_ivs[client_number];

	if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&this->hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0))) {
		//fputs("Failed to set the algorithm provider", stderr);
		exit(-1);
	}
		
	if (!BCRYPT_SUCCESS(BCryptSetProperty(this->hAlg, BCRYPT_CHAINING_MODE,
		(PUCHAR)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0))) {
		//fputs("Failed to set chaining mode", stderr);
		exit(-1);
	}

	if (!BCRYPT_SUCCESS(BCryptGenerateSymmetricKey(this->hAlg, &this->hKey, nullptr, 0, (PUCHAR)this->key, AES_KEY_SIZE, 0))) {
		//fputs("Failed to generate symmetric key", stderr);
		exit(-1);
	}

}
Encryption::~Encryption() {
	BCryptDestroyKey(this->hKey);
	BCryptCloseAlgorithmProvider(this->hAlg, 0);
}

int Encryption::encrypt(const void* plaintext, uint16_t plaintext_len, unsigned char** out_ciphertext, ULONG* out_ciphertext_len, unsigned char* out_tag) {
	const unsigned char* key = this->key;
	const unsigned char* iv = this->iv;
	unsigned char tag[AES_GCM_TAG_SIZE];

	BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO gcmInfo;
	BCRYPT_INIT_AUTH_MODE_INFO(gcmInfo);
	gcmInfo.pbNonce = const_cast<PUCHAR>(iv);
	gcmInfo.cbNonce = AES_GCM_IV_SIZE;
	gcmInfo.pbTag = tag;
	gcmInfo.cbTag = AES_GCM_TAG_SIZE;
	// Get ciphertext size
	// Get ciphertext size
	ULONG ciphertext_len = 0;
	if (!BCRYPT_SUCCESS(BCryptEncrypt(this->hKey, (PUCHAR)plaintext, plaintext_len,
		&gcmInfo, nullptr, 0, nullptr, 0, &ciphertext_len, 0)))
		return 0;

	unsigned char* ciphertext = (unsigned char*)HeapAlloc(GetProcessHeap(), 0, ciphertext_len);
	if (!ciphertext) return 0;

	ULONG final_len = 0;
	if (!BCRYPT_SUCCESS(BCryptEncrypt(this->hKey, (PUCHAR)plaintext, plaintext_len,
		&gcmInfo, nullptr, 0, ciphertext, ciphertext_len, &final_len, 0)))
	{
		HeapFree(GetProcessHeap(), 0, ciphertext);
		return 0;
	}

	return 1;
}
int Encryption::decrypt(const unsigned char* ciphertext, uint16_t ciphertext_len, const unsigned char* tag, unsigned char** out_plaintext, ULONG* out_plaintext_len) {
	const unsigned char* key = this->key;
	const unsigned char* iv = this->iv;
	unsigned char _tag[AES_GCM_TAG_SIZE];

	BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO gcmInfo;
	BCRYPT_INIT_AUTH_MODE_INFO(gcmInfo);
	gcmInfo.pbNonce = const_cast<PUCHAR>(iv);
	gcmInfo.cbNonce = AES_GCM_IV_SIZE;
	gcmInfo.pbTag = const_cast<PUCHAR>(tag);
	gcmInfo.cbTag = AES_GCM_TAG_SIZE;

	ULONG _ciphertext_len = ciphertext_len;
	if(!BCRYPT_SUCCESS(BCryptDecrypt(this->hKey, (PUCHAR)ciphertext, _ciphertext_len,
		&gcmInfo, nullptr, 0, nullptr, 0, out_plaintext_len, 0)))
		return 0;

	unsigned char* _ciphertext = (unsigned char*)HeapAlloc(GetProcessHeap(), 0, _ciphertext_len);
	if (!ciphertext) return 0;

	ULONG final_len = 0;
	if(!BCRYPT_SUCCESS(BCryptDecrypt(this->hKey, (PUCHAR)ciphertext, ciphertext_len,
		&gcmInfo, nullptr, 0, *out_plaintext, *out_plaintext_len, &final_len, 0)))
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)ciphertext);
		return 0;
	}

	return 1;
}





/*

// =======================================================
// AES-GCM Secure Send using hardcoded key/IV
// =======================================================
int Client::secure_send(uint8_t packet_type,
	const void* plaintext,
	uint16_t plaintext_len)
{
	const unsigned char* key = hardcoded_keys[this->clnt_number];
	const unsigned char* iv = hardcoded_ivs[this->clnt_number];

	BCRYPT_ALG_HANDLE hAlg = nullptr;
	BCRYPT_KEY_HANDLE hKey = nullptr;

	if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0)))
		return 0;

	if (!BCRYPT_SUCCESS(BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE,
		(PUCHAR)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0)))
		return 0;

	if (!BCRYPT_SUCCESS(BCryptGenerateSymmetricKey(hAlg, &hKey, nullptr, 0, (PUCHAR)key, AES_KEY_SIZE, 0)))
		return 0;

	unsigned char tag[AES_GCM_TAG_SIZE];

	BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO gcmInfo;
	BCRYPT_INIT_AUTH_MODE_INFO(gcmInfo);
	gcmInfo.pbNonce = const_cast<PUCHAR>(iv);
	gcmInfo.cbNonce = AES_GCM_IV_SIZE;
	gcmInfo.pbTag = tag;
	gcmInfo.cbTag = AES_GCM_TAG_SIZE;

	// Get ciphertext size
	ULONG ciphertext_len = 0;
	if (!BCRYPT_SUCCESS(BCryptEncrypt(hKey, (PUCHAR)plaintext, plaintext_len,
		&gcmInfo, nullptr, 0, nullptr, 0, &ciphertext_len, 0)))
		return 0;

	unsigned char* ciphertext = (unsigned char*)HeapAlloc(GetProcessHeap(), 0, ciphertext_len);
	if (!ciphertext) return 0;

	ULONG final_len = 0;
	if (!BCRYPT_SUCCESS(BCryptEncrypt(hKey, (PUCHAR)plaintext, plaintext_len,
		&gcmInfo, nullptr, 0, ciphertext, ciphertext_len, &final_len, 0)))
	{
		HeapFree(GetProcessHeap(), 0, ciphertext);
		return 0;
	}

	// Build header (payload = TAG + ciphertext)
	Packet_H header{};
	header.packet_type = packet_type;
	header.reserved = 0;
	header.payload_len = htons((uint16_t)(AES_GCM_TAG_SIZE + final_len));

	if (!send_all(&header, sizeof(header))) { HeapFree(GetProcessHeap(), 0, ciphertext); return 0; }
	if (!send_all(tag, AES_GCM_TAG_SIZE)) { HeapFree(GetProcessHeap(), 0, ciphertext); return 0; }
	if (!send_all(ciphertext, final_len)) { HeapFree(GetProcessHeap(), 0, ciphertext); return 0; }

	HeapFree(GetProcessHeap(), 0, ciphertext);
	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlg, 0);

	return 1;
}

// =======================================================
// AES-GCM Secure Receive using hardcoded key/IV
// =======================================================
int Client::secure_recv(uint8_t* out_type,
	unsigned char* out_buf,
	uint16_t* out_len)
{
	const unsigned char* key = hardcoded_keys[this->clnt_number];
	const unsigned char* iv = hardcoded_ivs[this->clnt_number];

	Packet_H header{};
	if (!recv_all(&header, sizeof(header))) return 0;

	*out_type = header.packet_type;
	uint16_t enc_total = ntohs(header.payload_len);

	if (enc_total < AES_GCM_TAG_SIZE) return 0;

	// Read TAG
	unsigned char tag[AES_GCM_TAG_SIZE];
	if (!recv_all(tag, AES_GCM_TAG_SIZE)) return 0;

	// Read ciphertext
	uint16_t ciphertext_len = enc_total - AES_GCM_TAG_SIZE;
	unsigned char* ciphertext = (unsigned char*)HeapAlloc(GetProcessHeap(), 0, ciphertext_len);
	if (!ciphertext) return 0;
	if (!recv_all(ciphertext, ciphertext_len)) { HeapFree(GetProcessHeap(), 0, ciphertext); return 0; }

	// Decrypt
	BCRYPT_ALG_HANDLE hAlg = nullptr;
	BCRYPT_KEY_HANDLE hKey = nullptr;

	if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0))) { HeapFree(GetProcessHeap(), 0, ciphertext); return 0; }
	if (!BCRYPT_SUCCESS(BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0))) { HeapFree(GetProcessHeap(), 0, ciphertext); return 0; }
	if (!BCRYPT_SUCCESS(BCryptGenerateSymmetricKey(hAlg, &hKey, nullptr, 0, (PUCHAR)key, AES_KEY_SIZE, 0))) { HeapFree(GetProcessHeap(), 0, ciphertext); return 0; }

	BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO gcmInfo;
	BCRYPT_INIT_AUTH_MODE_INFO(gcmInfo);
	gcmInfo.pbNonce = const_cast<PUCHAR>(iv);
	gcmInfo.cbNonce = AES_GCM_IV_SIZE;
	gcmInfo.pbTag = tag;
	gcmInfo.cbTag = AES_GCM_TAG_SIZE;

	ULONG plain_len = 0;
	if (!BCRYPT_SUCCESS(BCryptDecrypt(hKey, ciphertext, ciphertext_len,
		&gcmInfo, nullptr, 0, out_buf, *out_len, &plain_len, 0)))
	{
		HeapFree(GetProcessHeap(), 0, ciphertext);
		return 0;
	}

	*out_len = (uint16_t)plain_len;

	HeapFree(GetProcessHeap(), 0, ciphertext);
	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlg, 0);

	return 1;
}


*/