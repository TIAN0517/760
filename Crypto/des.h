#pragma once

#include <windows.h>
#include <cstddef>

typedef struct
{
	DWORD ks[32];
} DesContext;

//DES related functions
int desInit(DesContext *context, const BYTE *key, size_t keyLen);
void desEncryptBlock(DesContext *context, const BYTE *input, BYTE *output);
void desDecryptBlock(DesContext *context, const BYTE *input, BYTE *output);