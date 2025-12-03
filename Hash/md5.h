#pragma once

/* MD5 context. */
struct MD5_CTX {
	ULONG state[4];        /* state (ABCD) */
	ULONG count[2];        /* number of bits, modulo 2^64 (lsb first) */
	BYTE buffer[64];       /* input buffer */
	BYTE digest[16];
};

#define MD5_MASK_1	0x556A586E32723575
#define MD5_MASK_2	0x38782F413F442847

void MD5_Init(MD5_CTX *context);
void MD5_Update(MD5_CTX *context, unsigned char *input, unsigned int inputLen);
void MD5_Final(unsigned char digest[16], MD5_CTX *context);

void md5_buffer(_TCHAR* md5, const unsigned char* buffer, size_t len);
bool md5_file(_TCHAR* md5, const _TCHAR* filename);