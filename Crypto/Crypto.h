#pragma once

#define ROL28(a, n) ((((a) << (n)) | ((a) >> (28 - (n)))) & 0x0FFFFFFF)
#define ROL32(a, n) (((a) << (n)) | ((a) >> (32 - (n))))

#define ROR32(a, n) (((a) >> (n)) | ((a) << (32 - (n))))

#define LOAD32BE(p) ( \
	((DWORD)(((BYTE *)(p))[0]) << 24) | \
	((DWORD)(((BYTE *)(p))[1]) << 16) | \
	((DWORD)(((BYTE *)(p))[2]) << 8) | \
	((DWORD)(((BYTE *)(p))[3]) << 0))

#define LOAD32LE(p) ( \
	((DWORD)(((BYTE *)(p))[0]) << 0) | \
	((DWORD)(((BYTE *)(p))[1]) << 8) | \
	((DWORD)(((BYTE *)(p))[2]) << 16) | \
	((DWORD)(((BYTE *)(p))[3]) << 24))

//Store unaligned 32-bit integer (big-endian encoding)
#define STORE32BE(a, p) \
	((BYTE *)(p))[0] = ((DWORD)(a) >> 24) & 0xFFU, \
	((BYTE *)(p))[1] = ((DWORD)(a) >> 16) & 0xFFU, \
	((BYTE *)(p))[2] = ((DWORD)(a) >> 8) & 0xFFU, \
	((BYTE *)(p))[3] = ((DWORD)(a) >> 0) & 0xFFU
	
 #define STORE32LE(a, p) \
	((BYTE *)(p))[0] = ((DWORD)(a) >> 0) & 0xFFU, \
	((BYTE *)(p))[1] = ((DWORD)(a) >> 8) & 0xFFU, \
	((BYTE *)(p))[2] = ((DWORD)(a) >> 16) & 0xFFU, \
	((BYTE *)(p))[3] = ((DWORD)(a) >> 24) & 0xFFU