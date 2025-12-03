#pragma once

#define RC6_BLOCK_SIZE 16

#define RC6_MAX_KEY_SIZE 32

#define RC6_NB_ROUNDS 20

typedef struct
{
	DWORD l[RC6_MAX_KEY_SIZE / 4];
	DWORD s[2 * RC6_NB_ROUNDS + 4];
} Rc6Context;

int rc6Init(Rc6Context* context, const BYTE *key, size_t keyLen);
void rc6EncryptBlock(Rc6Context* context, const BYTE* input, BYTE* output);
void rc6DecryptBlock(Rc6Context* context, const BYTE* input, BYTE* output);