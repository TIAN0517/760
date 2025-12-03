#include "stdafx.h"
#include "md5.h"

#include <cstring>

namespace
{
	constexpr BYTE MD5_PADDING[64] = { 0x80 };

	inline void Encode(BYTE* output, const ULONG* input, unsigned int len)
	{
		for (unsigned int i = 0, j = 0; j < len; ++i, j += 4)
		{
			output[j] = static_cast<BYTE>(input[i] & 0xff);
			output[j + 1] = static_cast<BYTE>((input[i] >> 8) & 0xff);
			output[j + 2] = static_cast<BYTE>((input[i] >> 16) & 0xff);
			output[j + 3] = static_cast<BYTE>((input[i] >> 24) & 0xff);
		}
	}

	inline void Decode(ULONG* output, const BYTE* input, unsigned int len)
	{
		for (unsigned int i = 0, j = 0; j < len; ++i, j += 4)
		{
			output[i] = static_cast<ULONG>(input[j]) |
						 (static_cast<ULONG>(input[j + 1]) << 8) |
						 (static_cast<ULONG>(input[j + 2]) << 16) |
						 (static_cast<ULONG>(input[j + 3]) << 24);
		}
	}

	inline ULONG F(ULONG x, ULONG y, ULONG z) { return (x & y) | (~x & z); }
	inline ULONG G(ULONG x, ULONG y, ULONG z) { return (x & z) | (y & ~z); }
	inline ULONG H(ULONG x, ULONG y, ULONG z) { return x ^ y ^ z; }
	inline ULONG I(ULONG x, ULONG y, ULONG z) { return y ^ (x | ~z); }

	inline ULONG ROTATE_LEFT(ULONG value, ULONG bits)
	{
		return (value << bits) | (value >> (32 - bits));
	}

	inline void STEP(ULONG& a, ULONG b, ULONG c, ULONG d, ULONG x, ULONG s, ULONG ac, ULONG(*func)(ULONG, ULONG, ULONG))
	{
		a += func(b, c, d) + x + ac;
		a = ROTATE_LEFT(a, s);
		a += b;
	}

	void Transform(ULONG state[4], const BYTE block[64])
	{
		ULONG x[16];
		Decode(x, block, 64);

		ULONG a = state[0];
		ULONG b = state[1];
		ULONG c = state[2];
		ULONG d = state[3];

		STEP(a, b, c, d, x[0], 7, 0xd76aa478, F);
		STEP(d, a, b, c, x[1], 12, 0xe8c7b756, F);
		STEP(c, d, a, b, x[2], 17, 0x242070db, F);
		STEP(b, c, d, a, x[3], 22, 0xc1bdceee, F);
		STEP(a, b, c, d, x[4], 7, 0xf57c0faf, F);
		STEP(d, a, b, c, x[5], 12, 0x4787c62a, F);
		STEP(c, d, a, b, x[6], 17, 0xa8304613, F);
		STEP(b, c, d, a, x[7], 22, 0xfd469501, F);
		STEP(a, b, c, d, x[8], 7, 0x698098d8, F);
		STEP(d, a, b, c, x[9], 12, 0x8b44f7af, F);
		STEP(c, d, a, b, x[10], 17, 0xffff5bb1, F);
		STEP(b, c, d, a, x[11], 22, 0x895cd7be, F);
		STEP(a, b, c, d, x[12], 7, 0x6b901122, F);
		STEP(d, a, b, c, x[13], 12, 0xfd987193, F);
		STEP(c, d, a, b, x[14], 17, 0xa679438e, F);
		STEP(b, c, d, a, x[15], 22, 0x49b40821, F);

		STEP(a, b, c, d, x[1], 5, 0xf61e2562, G);
		STEP(d, a, b, c, x[6], 9, 0xc040b340, G);
		STEP(c, d, a, b, x[11], 14, 0x265e5a51, G);
		STEP(b, c, d, a, x[0], 20, 0xe9b6c7aa, G);
		STEP(a, b, c, d, x[5], 5, 0xd62f105d, G);
		STEP(d, a, b, c, x[10], 9, 0x02441453, G);
		STEP(c, d, a, b, x[15], 14, 0xd8a1e681, G);
		STEP(b, c, d, a, x[4], 20, 0xe7d3fbc8, G);
		STEP(a, b, c, d, x[9], 5, 0x21e1cde6, G);
		STEP(d, a, b, c, x[14], 9, 0xc33707d6, G);
		STEP(c, d, a, b, x[3], 14, 0xf4d50d87, G);
		STEP(b, c, d, a, x[8], 20, 0x455a14ed, G);
		STEP(a, b, c, d, x[13], 5, 0xa9e3e905, G);
		STEP(d, a, b, c, x[2], 9, 0xfcefa3f8, G);
		STEP(c, d, a, b, x[7], 14, 0x676f02d9, G);
		STEP(b, c, d, a, x[12], 20, 0x8d2a4c8a, G);

		STEP(a, b, c, d, x[5], 4, 0xfffa3942, H);
		STEP(d, a, b, c, x[8], 11, 0x8771f681, H);
		STEP(c, d, a, b, x[11], 16, 0x6d9d6122, H);
		STEP(b, c, d, a, x[14], 23, 0xfde5380c, H);
		STEP(a, b, c, d, x[1], 4, 0xa4beea44, H);
		STEP(d, a, b, c, x[4], 11, 0x4bdecfa9, H);
		STEP(c, d, a, b, x[7], 16, 0xf6bb4b60, H);
		STEP(b, c, d, a, x[10], 23, 0xbebfbc70, H);
		STEP(a, b, c, d, x[13], 4, 0x289b7ec6, H);
		STEP(d, a, b, c, x[0], 11, 0xeaa127fa, H);
		STEP(c, d, a, b, x[3], 16, 0xd4ef3085, H);
		STEP(b, c, d, a, x[6], 23, 0x04881d05, H);
		STEP(a, b, c, d, x[9], 4, 0xd9d4d039, H);
		STEP(d, a, b, c, x[12], 11, 0xe6db99e5, H);
		STEP(c, d, a, b, x[15], 16, 0x1fa27cf8, H);
		STEP(b, c, d, a, x[2], 23, 0xc4ac5665, H);

		STEP(a, b, c, d, x[0], 6, 0xf4292244, I);
		STEP(d, a, b, c, x[7], 10, 0x432aff97, I);
		STEP(c, d, a, b, x[14], 15, 0xab9423a7, I);
		STEP(b, c, d, a, x[5], 21, 0xfc93a039, I);
		STEP(a, b, c, d, x[12], 6, 0x655b59c3, I);
		STEP(d, a, b, c, x[3], 10, 0x8f0ccc92, I);
		STEP(c, d, a, b, x[10], 15, 0xffeff47d, I);
		STEP(b, c, d, a, x[1], 21, 0x85845dd1, I);
		STEP(a, b, c, d, x[8], 6, 0x6fa87e4f, I);
		STEP(d, a, b, c, x[15], 10, 0xfe2ce6e0, I);
		STEP(c, d, a, b, x[6], 15, 0xa3014314, I);
		STEP(b, c, d, a, x[13], 21, 0x4e0811a1, I);
		STEP(a, b, c, d, x[4], 6, 0xf7537e82, I);
		STEP(d, a, b, c, x[11], 10, 0xbd3af235, I);
		STEP(c, d, a, b, x[2], 15, 0x2ad7d2bb, I);
		STEP(b, c, d, a, x[9], 21, 0xeb86d391, I);

		state[0] += a;
		state[1] += b;
		state[2] += c;
		state[3] += d;

		std::memset(x, 0, sizeof(x));
	}
} // namespace

void WINAPI MD5Init(MD5_CTX* lpCtx)
{
	if (lpCtx == nullptr)
	{
		return;
	}

	lpCtx->num[0] = 0;
	lpCtx->num[1] = 0;
	lpCtx->buf[0] = 0x67452301;
	lpCtx->buf[1] = 0xefcdab89;
	lpCtx->buf[2] = 0x98badcfe;
	lpCtx->buf[3] = 0x10325476;
	std::memset(lpCtx->input, 0, sizeof(lpCtx->input));
	std::memset(lpCtx->digest, 0, sizeof(lpCtx->digest));
}

void WINAPI MD5Update(MD5_CTX* lpCtx, const void* lpBuf, unsigned int nLen)
{
	if (lpCtx == nullptr || lpBuf == nullptr || nLen == 0)
	{
		return;
	}

	const BYTE* input = static_cast<const BYTE*>(lpBuf);
	unsigned int index = static_cast<unsigned int>((lpCtx->num[0] >> 3) & 0x3f);

	lpCtx->num[0] += static_cast<ULONG>(nLen) << 3;
	if (lpCtx->num[0] < (static_cast<ULONG>(nLen) << 3))
	{
		lpCtx->num[1]++;
	}
	lpCtx->num[1] += static_cast<ULONG>(nLen) >> 29;

	unsigned int partLen = 64 - index;
	unsigned int i = 0;

	if (nLen >= partLen)
	{
		std::memcpy(&lpCtx->input[index], input, partLen);
		Transform(lpCtx->buf, lpCtx->input);

		for (i = partLen; i + 63 < nLen; i += 64)
		{
			Transform(lpCtx->buf, &input[i]);
		}

		index = 0;
	}

	if (i < nLen)
	{
		std::memcpy(&lpCtx->input[index], &input[i], nLen - i);
	}
}

void WINAPI MD5Final(MD5_CTX* lpCtx)
{
	if (lpCtx == nullptr)
	{
		return;
	}

	BYTE bits[8];
	Encode(bits, lpCtx->num, 8);

	unsigned int index = static_cast<unsigned int>((lpCtx->num[0] >> 3) & 0x3f);
	unsigned int padLen = (index < 56) ? (56 - index) : (120 - index);
	MD5Update(lpCtx, MD5_PADDING, padLen);
	MD5Update(lpCtx, bits, 8);
	Encode(lpCtx->digest, lpCtx->buf, 16);

	std::memset(lpCtx->input, 0, sizeof(lpCtx->input));
}

void md5_buffer(_TCHAR* md5, const unsigned char* buffer, size_t len)
{
	MD5_CTX ctx;
	MD5Init(&ctx);
	MD5Update(&ctx, buffer, len);
	MD5Final(&ctx);

	_TCHAR tmp[32];
	for(int i = 0; i < 16; i++)
	{
		_stprintf(tmp, _T("%02x"), ctx.digest[i]);
		_tcscat(md5, tmp);
	}
}

bool md5_file(_TCHAR* md5, const _TCHAR* filename)
{

	MD5_CTX ctx;
	FILE* fp;

	if(fp = _tfopen(filename, _T("rb")),fp == 0)
		return false;

	MD5Init(&ctx);
	unsigned char buffer[8192];

	fseek(fp, 0, SEEK_SET);
	while(!feof(fp))
	{
		size_t len = fread(buffer, 1, sizeof(buffer), fp);
		if(ferror(fp))
			break;
		MD5Update(&ctx, buffer, len);
	}
	fclose(fp);
	MD5Final(&ctx);
	
	_TCHAR tmp[32];
	for(int i = 0; i < 16; i++)
	{
		_stprintf(tmp, _T("%02x"), ctx.digest[i]);
		_tcscat(md5, tmp);
	}
	return true;
}