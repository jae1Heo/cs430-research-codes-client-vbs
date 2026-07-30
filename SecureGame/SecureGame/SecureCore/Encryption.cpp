#include "Encryption.h"

const char* test_key = "abcdefghijklmnopqrstuvwxyzzyxwvu";

Encryption::Encryption() {

}
Encryption::~Encryption() {

}


// key loader
bool Encryption::loadPrivateKey(const char* key_pem, BCRYPT_KEY_HANDLE* key) {
	NTSTATUS status;
	BCRYPT_ALG_HANDLE hAlg = NULL;

	size_t pemLen = strlen(key_pem);
	HANDLE heap = GetProcessHeap();

	char* b64Clean = (char*)HeapAlloc(heap, HEAP_ZERO_MEMORY, pemLen + 1);
	if (!b64Clean) {
		return false;
	}

	size_t cleanIndex = 0;
	for (size_t i = 0; i < pemLen; i++) {
		// ignore header/footer 
		if (key_pem[i] == '-') {
			while (i < pemLen && key_pem[i] != '\n') {
				i++;
			}
			continue;
		}
		char c = key_pem[i];
		// takes only base64 encoded daa
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
			b64Clean[cleanIndex++] = c;
		}
	}

	BYTE* derbytes = (BYTE*)HeapAlloc(heap, HEAP_ZERO_MEMORY, cleanIndex);
	if (!derbytes) {
		HeapFree(heap, 0, b64Clean);
		return false;
	}

	size_t derLen = this->base64Decode(b64Clean, cleanIndex, derbytes);
	HeapFree(heap, 0, b64Clean);

	if (derLen == 0) {
		HeapFree(heap, 0, derbytes);
		return false;
	}

	RSA_PRIVATE_PARAMS params = { 0 };
	if (!ExtractRsaParamsFromDerPrivate(derbytes, derLen, &params)) {
		HeapFree(heap, 0, derbytes);
		return false;
	}

	// blob length: header + e + n + p + q
	ULONG blobLen = sizeof(BCRYPT_RSAKEY_BLOB) + params.exp_bytes + params.mod_bytes + params.p_bytes + params.dq_bytes;
	BCRYPT_RSAKEY_BLOB* header = (BCRYPT_RSAKEY_BLOB*)HeapAlloc(heap, HEAP_ZERO_MEMORY, blobLen);

	header->Magic = BCRYPT_RSAPRIVATE_MAGIC;
	header->BitLength = params.mod_bytes * 8;
	header->cbPublicExp = params.exp_bytes;
	header->cbModulus = params.mod_bytes;
	header->cbPrime1 = params.p_bytes;
	header->cbPrime2 = params.q_bytes;

	BYTE* offset = ((BYTE*)header) + sizeof(BCRYPT_RSAKEY_BLOB);
	memcpy(offset, params.exp_p, params.exp_bytes);
	offset += params.exp_bytes;
	memcpy(offset, params.mod_p, params.mod_bytes);
	offset += params.mod_bytes;
	memcpy(offset, params.p, params.p_bytes);
	offset += params.p_bytes;
	memcpy(offset, params.q, params.q_bytes);

	HeapFree(heap, 0, derbytes);

	status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RSA_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);

	if (status) {
		HeapFree(heap, 0, header);
		return false;
	}

	BCryptImportKeyPair(hAlg, NULL, BCRYPT_RSAPRIVATE_BLOB, key, (PUCHAR)header, blobLen, 0);
	HeapFree(heap, 0, header);
	BCryptCloseAlgorithmProvider(hAlg, 0);
	return true;
}

bool Encryption::loadPublicKey(const char* key_pem, BCRYPT_KEY_HANDLE* key) {
	NTSTATUS status;
	BCRYPT_ALG_HANDLE hAlg = NULL;

	size_t pemLen = strlen(key_pem);
	HANDLE heap = GetProcessHeap();

	char* b64Clean = (char*)HeapAlloc(heap, HEAP_ZERO_MEMORY, pemLen + 1);
	if (!b64Clean){ 	
		return false;
	}

	size_t cleanIndex = 0;
	for (size_t i = 0; i < pemLen; i++) {
		if (key_pem[i] == '-') {
			while (i < pemLen && key_pem[i] != '\n') i++;
			continue;
		}
		char c = key_pem[i];
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
			b64Clean[cleanIndex++] = c;
		}
	}

	BYTE* derbytes = (BYTE*)HeapAlloc(heap, HEAP_ZERO_MEMORY, cleanIndex);
	if (!derbytes) {
		HeapFree(heap, 0, b64Clean);
		return false;
	}

	size_t derLen = this->base64Decode(b64Clean, cleanIndex, derbytes);
	HeapFree(heap, 0, b64Clean);

	if (derLen == 0) {
		HeapFree(heap, 0, derbytes);
		return false;
	}

	BYTE* srcmod = NULL;
	BYTE* srcexp = NULL;
	ULONG modules = 0;
	ULONG exponent = 0;

	if (!this->extractRsaParamsFromDerPublic(derbytes, derLen, &srcmod, &modules, &srcexp, &exponent)) {
		HeapFree(heap, 0, derbytes);
		return STATUS_INVALID_PARAMETER;
	}

	// BCrypt Blob Layout: BCRYPT_RSAKEY_BLOB + Exponent + Modulus
	ULONG blobLen = sizeof(BCRYPT_RSAKEY_BLOB) + exponent + modules;
	BCRYPT_RSAKEY_BLOB* header = (BCRYPT_RSAKEY_BLOB*)HeapAlloc(heap, HEAP_ZERO_MEMORY, blobLen);

	header->Magic = BCRYPT_RSAPUBLIC_MAGIC;
	header->BitLength = modules * 8;
	header->cbPublicExp = exponent;
	header->cbModulus = modules;

	BYTE* destExp = ((BYTE*)header) + sizeof(BCRYPT_RSAKEY_BLOB);
	BYTE* destMod = destExp + exponent;

	memcpy(destExp, srcexp, exponent);
	memcpy(destMod, srcmod, modules);

	HeapFree(heap, 0, derbytes);

	status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RSA_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0);
	if (status) {
		HeapFree(heap, 0, header);
		return false;
	}

	BCryptImportKeyPair(hAlg, NULL, BCRYPT_RSAPUBLIC_BLOB, key, (PUCHAR)header, blobLen, 0);

	HeapFree(heap, 0, header);
	BCryptCloseAlgorithmProvider(hAlg, 0);
	return true;
}

bool Encryption::extractRsaParamsFromDerPublic(const BYTE* der, size_t derLen, BYTE** modules_pp, ULONG* modules_pcb, BYTE** exponent_pp, ULONG* exponent_pcb) {
	if (!der || derLen < 64) return false;

	size_t index = 0;

	auto parseLength = [&der, &index, derLen](ULONG* pLength) -> BOOL {
		if (index >= derLen) return false;
		BYTE lenByte = der[index++];
		if (lenByte & 0x80) {
			BYTE numBytes = lenByte & 0x7F;
			if (numBytes == 0 || numBytes > 4 || index + numBytes > derLen) return false;
			ULONG len = 0;
			for (BYTE i = 0; i < numBytes; i++) {
				len = (len << 8) | der[index++];
			}
			*pLength = len;
		}
		else {
			*pLength = lenByte;
		}
		return true;
		};

	ULONG len = 0;

	// tkae outer sequence (0x30)
	if (index >= derLen || der[index++] != 0x30 || !parseLength(&len)) return false;

	// take algorithmIdentifier sequence (0x30)
	if (index >= derLen || der[index++] != 0x30) return false;
	ULONG algLen = 0;
	if (!parseLength(&algLen)) return false;
	index += algLen; // skip algorithm details OID

	// take bit string tag (0x03)
	if (index >= derLen || der[index++] != 0x03) return false;
	ULONG bitStrLen = 0;
	if (!parseLength(&bitStrLen)) return false;

	// skip bit string padding byte (0x00)
	if (index >= derLen || der[index++] != 0x00) return false;

	// take inner RSAPublicKey sequence (0x30)
	if (index >= derLen || der[index++] != 0x30) return false;
	ULONG rsaSeqLen = 0;
	if (!parseLength(&rsaSeqLen)) return false;

	// take modulus INTEGER Tag (0x02)
	if (index >= derLen || der[index++] != 0x02) return false;
	ULONG modLen = 0;
	if (!parseLength(&modLen)) return false;

	if (index + modLen > derLen) return false;
	BYTE* pMod = (BYTE*)&der[index];
	index += modLen;

	// strip leading 0x00 byte (257 bytes -> 256 bytes for 2048-bit modulus)
	if (modLen > 0 && pMod[0] == 0x00) {
		pMod++;
		modLen--;
	}

	// take exponent integer Tag (0x02)
	if (index >= derLen || der[index++] != 0x02) return false;
	ULONG expLen = 0;
	if (!parseLength(&expLen)) return false;

	if (index + expLen > derLen) return false;
	BYTE* pExp = (BYTE*)&der[index];

	if (expLen > 0 && pExp[0] == 0x00) {
		pExp++;
		expLen--;
	}

	*modules_pp = pMod;
	*modules_pcb = modLen;   // 256 bytes
	*exponent_pp = pExp;
	*exponent_pcb = expLen;  // 3 bytes

	return true;
}

bool Encryption::ExtractRsaParamsFromDerPrivate(const BYTE* der, size_t derLen, RSA_PRIVATE_PARAMS* params) {
	if (!der || derLen < 128) {
		return false;
	}

	size_t index = 0;
	auto parseLength = [&der, &index, derLen](ULONG* pLength) -> BOOL {
		if (index >= derLen) {
			return false;
		}
		BYTE lenByte = der[index++];
		if (lenByte & 0x80) {
			BYTE numBytes = lenByte & 0x7F;
			if (numBytes == 0 || numBytes > 4 || index + numBytes > derLen) {
				return false;
			}
			ULONG len = 0;
			for (BYTE i = 0; i < numBytes; i++) {
				len = (len << 8) | der[index++];
			}
			*pLength = len;
		}
		else {
			*pLength = lenByte;
		}
		return true;
		};

	auto parseInteger = [&der, &index, derLen, &parseLength](BYTE** ppVal, ULONG* pCb) -> BOOL {
		if (index >= derLen || der[index++] != 0x02) {
			return false;
		}
		ULONG len = 0;
		if (!parseLength(&len) || index + len > derLen) {
			return false;
		}

		BYTE* pData = (BYTE*)&der[index];
		index += len;

		if (len > 0 && pData[0] == 0x00) {
			pData++;
			len--;
		}

		*ppVal = pData;
		*pCb = len;
		return true;
		};

	ULONG len = 0;
	if (index >= derLen || der[index++] != 0x30 || !parseLength(&len)) {
		return false;
	}

	BYTE* pVer = NULL;
	ULONG cbVer = 0;
	if (!parseInteger(&pVer, &cbVer)) {
		return false;
	}

	if (index >= derLen || der[index++] != 0x30) {
		return false;
	}
	ULONG algLen = 0;
	if (!parseLength(&algLen)) {
		return false;
	}
	index += algLen;

	if (index >= derLen || der[index++] != 0x04) {
		return false;
	}

	ULONG octetLen = 0;
	if (!parseLength(&octetLen)) {
		return false;
	}

	if (index >= derLen || der[index++] != 0x30) {
		return false;
	}

	ULONG rsaSeqLen = 0;
	if (!parseLength(&rsaSeqLen)) {
		return false;
	}

	if (!parseInteger(&pVer, &cbVer)) {
		return false;
	}

	if (!parseInteger(&params->mod_p, &params->mod_bytes)) {
		return false;
	}

	if (!parseInteger(&params->exp_p, &params->exp_bytes)) {
		return false;
	}

	if (!parseInteger(&params->priv_exp_p, &params->priv_exp_bytes)) {
		return false;
	}

	if (!parseInteger(&params->p, &params->p_bytes)) {
		return false;
	}

	if (!parseInteger(&params->q, &params->q_bytes)) {
		return false;
	}

	if (!parseInteger(&params->dp, &params->dp_bytes)) {
		return false;
	}

	if (!parseInteger(&params->dq, &params->dq_bytes)) {
		return false;
	}

	if (!parseInteger(&params->inv, &params->inv_bytes)) {
		return false;
	}

	return true;
}

// base64 decode
size_t Encryption::base64Decode(const char* src, size_t srcLen, BYTE* dest) {
	static const int b64inv[256] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
	52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
	15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
	-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
	41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
	};

	size_t outLen = 0;
	DWORD val = 0;
	int valb = -8;

	for (size_t i = 0; i < srcLen; i++) {
		unsigned char c = (unsigned char)src[i];
		if (c == '=') break;

		int d = b64inv[c];
		if (d == -1) continue;

		val = (val << 6) | d;
		valb += 6;

		if (valb >= 0) {
			dest[outLen++] = (BYTE)((val >> valb) & 0xFF);
			valb -= 8;
		}
	}
	return outLen;
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
