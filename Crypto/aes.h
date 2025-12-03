#pragma once

#define AES_ENCRYPT 1
#define AES_DECRYPT 0

#define AES_MAXNR 14
#define AES_BLOCK_SIZE 16

#define SWAP(x) (_lrotl(x, 8) & 0x00ff00ff | _lrotr(x, 8) & 0xff00ff00)
#define GETU32(p) SWAP(*((unsigned long *)(p)))
#define PUTU32(ct, st) { *((unsigned long *)(ct)) = SWAP((st)); }

struct aes_key_st {
	unsigned long rd_key[4 * (AES_MAXNR + 1)];
	int rounds;
};

typedef struct aes_key_st AES_KEY;

int AES_set_encrypt_key(const unsigned char *userKey, const int bits, AES_KEY *key);
int AES_set_decrypt_key(const unsigned char *userKey, const int bits, AES_KEY *key);

void AES_encrypt(const unsigned char *in, unsigned char *out, const AES_KEY *key);
void AES_decrypt(const unsigned char *in, unsigned char *out, const AES_KEY *key);
