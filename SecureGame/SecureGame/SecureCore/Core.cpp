#include "Global.h"


// test client private key
const char* enclave_private_key_str =
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDbB5LxD3EY0foo\n"
"/0ILKs/E2wJa8+LOz+F4oNPjNe3P8ncZ/dxbpd7/efCFyG8pHnJf+bpYYYq2HiYQ\n"
"EjYb9dyR9ZHbwqtVGhlVyO2cIrK5C2MYQJwC+YU90djZSje222RISFpEOdH66Mxf\n"
"DRMq3a0VRXhQHFQP+y6/m96mBGTYK6dKUUiybRvn58cL8LtAF9ROpx1bBybVyUdQ\n"
"bfHuhjxca52SjydQpoPYTAlKhqBbd0kgVaDM4z7UjvhLeG+k4auJd3PCebuj2uqX\n"
"8uNvWSUV/Hubyf2NCN9wInNeEu7Pujko2HeiLkUTDHg5a0Dy1oUYYaw59iXU1u0A\n"
"Do+795jFAgMBAAECggEAF2Wj+fm5Dlpx01rPGoPIfYIaqo4Wapq0vm7Jq6IJZRcE\n"
"x4NBfI3HuDk1qwjZ7QXKWHvD/uhuiZ1HWhyj6JE3LmVR3yyYokbfMXV9AlqHhnJT\n"
"s9XFBKFUbSHjPPTrZi5h9EG8WMDncBrZqAasxz22XUkWPNKF3U4q9H9pRYk/H4M5\n"
"qv+A1Kbwifz5rcY1b+Bc3KibTxdd2MWgSJDy1LhrVq6rIrnWjcvZjckXg0PjiD0Z\n"
"g5om3TdjXcZtqyG2CQUb1hP5g2KDnL8I6ecNAOiiUchCXmeQ19O0tw0RghkXOuSU\n"
"M9QU75Lm67w9uyXpEdWcFKPkFbMh9z2WFgEHDBpdwQKBgQDwwSmrZjwWY0G/JdDn\n"
"UsBMH9e3pJIid3xRTBmzCc9eKXh1fyXtc+oUqUTUpqWbNpKpdGcetoPHX3kt+elB\n"
"JcJWa86dxvJ6TT7KvjnoFr8yjQ6Yn46V135eIzNgXtjNt75Vq1J0WURXIqPawQ75\n"
"cXUleig6GdFEXzaR2h+YMx/PQQKBgQDo5jsoKoxznc51kvSknWKfGctLNlrM6J5E\n"
"T2AbQJARU2mRnQ+uJFtobEOmY3y5Ct7WOROBrkrk9kIaYYaCT1wmRnLGz+lUU1vu\n"
"9kYixsiLWoNXeWewPqYtDtBIPgmL5hqMCIry7J+R0g078IQhQ94EWHa5h4o2XV7/\n"
"erFboiHshQKBgQDkIOTvOMyvGhZundK5nzv/5hxuM08VwcrW7hlcAWuxefJew8CY\n"
"pEGmmk5SgiZiUO/gCiC8hY2RGfKlki1oQfNIGJeMAbw6D3/0dRRBy2wY5nhyPp7J\n"
"dYyUfx5rrvQY2odMfi5/eSa8umIxIsahrtSmUn5Tr6sP8niu89UET01RAQKBgQDf\n"
"Km2ZCVxYFOiWhOBjNGQh7Ad+5HCRVH0DG8QNmQnzcCgCXat+xFvKbaNNNpt2fFIW\n"
"l853PtmJF+czfCv1hbHZAzxMAUGlidLX4ahG/o9/6JVeJnkSypVVK5KtBrUDFtRt\n"
"RmpOaCpnAg2oV8lkTrHZHCN3l8b6XPIAgEKeeXdkOQKBgCIml9H1Q45Lu6GyTz/l\n"
"QoAR00vF8Ik2pGIJ9juUyz0ldzccLNQfjOlAA/BadRCPoiwPUPHboeBhp8ccr4kb\n"
"xxtoOpv0blmmJfbU5Q+J99FrZhSxfFwtonOPqGx2ztqiiWxYvFME52xEDAryWnns\n"
"RVo/V+K8ISNFpB5A7RtuFKYS\n"
"-----END PRIVATE KEY-----\n";

// test server public key
const char* server_pub_key_str =
"-----BEGIN PUBLIC KEY----\n"
"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyKDI0s8iK4U / FBNSbJZV\n"
"9dvpxxRQCaKwc/f3aFQDrBLEtpPlKcbi8C08yl2yCPfYYWEqa8EVfVS+CIMJZw3S\n"
"9IKikK8UOqXbGxPqhusMSsyfA7MJPYqGj/2igJvQ/LnZ1HNRLtw6rxztsNNUyKFv\n"
"MX6yjf6LJ0g7pRWEPFwrY8QklC+uXqkDX6ftubp7IRHzyuxa/Y60SZLeEOQAxyDo\n"
"BGfXNaELlyf+c2zri4rbeMXWsHP3YDsAPO96hPFVkwAJdbOzWwo6iRILQcJWn7e+\n"
"4tsE1TG6NspKCWVSrZnt8RASgZiNv5BldomjS+JBgkckjoj8CtqH8sUjaZQHzsG7\n"
"0QIDAQAB\n"
"-----END PUBLIC KEY-----\n";


static BCRYPT_KEY_HANDLE enclavePrivateKey = NULL;
static BCRYPT_KEY_HANDLE serverPublicKey = NULL;
static bool isKeyLoaded = false;

extern "C" __declspec(dllexport) void* CALLBACK GameTick(PVOID context)
{

    if (!context) {
        return nullptr;
    }
	
    EnclaveInput*input  = static_cast<EnclaveInput*>(context);
    Encryption enc = Encryption();

	if(!isKeyLoaded) {
		enc.loadPrivateKey(enclave_private_key_str, &enclavePrivateKey);
		enc.loadPublicKey(server_pub_key_str, &serverPublicKey);
		isKeyLoaded = true;
	}

    if(input->isEncrypt) {
		mv playerMovement;
		memcpy(&playerMovement, input->buffer, sizeof(mv));

		envelope outEnvelope;
		SecureZeroMemory(&outEnvelope, sizeof(envelope));

		bool success = enc.buildEnvelope(reinterpret_cast<unsigned char*>(&playerMovement), sizeof(mv), enclavePrivateKey, serverPublicKey, &outEnvelope);

		SecureZeroMemory(&playerMovement, sizeof(mv));
		SecureZeroMemory(input->buffer, sizeof(envelope));

		if(!success) {
			return nullptr;
		}

		memcpy(input->buffer, &outEnvelope, sizeof(envelope));
		input->isEncrypt = false;
		/*
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
		*/
	}
    else {
		envelope inEnvelope;
		memcpy(&inEnvelope, input->buffer, sizeof(envelope));

		SecureZeroMemory(input->buffer, sizeof(envelope));

		unsigned char decrypted[PACKET_MAX] = {0};

		bool success = enc.resolveEnvelope(&inEnvelope, enclavePrivateKey, serverPublicKey, decrypted, PACKET_MAX);

		SecureZeroMemory(&inEnvelope, sizeof(envelope));

		if(!success) {
			SecureZeroMemory(decrypted, PACKET_MAX);
			return nullptr;
		}

		// if received packet is used for handshaking

		if(decrypted[0] == 's') {
			input->buffer[0] = 's';
			input->buffer[1] = decrypted[1]; // asign side
			SecureZeroMemory(decrypted, PACKET_MAX);
			return nullptr;
		}

		gData* state = reinterpret_cast<gData*>(decrypted);

		EnclaveOutput output = {0};
		output.rects[0] = {(int)state->left_paddle_x, (int)state->left_paddle_y, 10, 60};
		output.rects[1] = {(int)state->right_paddle_x, (int)state->right_paddle_y, 10, 60};
		output.rects[2] = {(int)state->ball_pos_x, (int)state->ball_pos_y, 10, 10};
		output.left_score = state->left_score;
		output.right_score = state->right_score;
		output.valid = true;

		SecureZeroMemory(decrypted, PACKET_MAX);
		memcpy(input->buffer, &output, sizeof(EnclaveOutput));
		
		/*
        unsigned char plaintext[64];
		if (input->cipherLen <= 0) {
            return nullptr;
        }
        int plaintext_len = enc.AES_decrypt((unsigned char*)input->buffer, input->cipherLen, plaintext);
        if (plaintext_len < 0) {
            return nullptr;
        }

        memset(input->buffer, 0, 64);
        memcpy(input->buffer, plaintext, plaintext_len);

		SecureZeroMemory(plaintext, sizeof(char)* 64);
		SecureZeroMemory(ciphertext, sizeof(char) *64);
		*/
    }

    return nullptr;
}
