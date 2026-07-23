#include "Global.h"


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
		enclavePrivateKey = enc.loadPrivateKey(/*private pem string -- will be added later*/);
		serverPublicKey = enc.loadPublicKey(/*public pem string -- will be added later*/);
		isKeyLoaded = true;
	}

    if(input->isEncrypt) {
		mv playerMovement;
		memcpy(&playerMovement, input->buffer, sizeof(mv));

		envelope outEnvelope;
		SecureZeroMemory(&outEnvelope, sizeof(envelope));

		bool success = enc.buildEnvelope(reinterpret_cast<unsigned char*>(&playerMovement), sizeof(mv), enclavePrivate, serverPublicKey, &outEnvelope);

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
