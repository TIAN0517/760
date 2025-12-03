#include "stdafx.h"
#include "base64.h"
#include "../Crypto/base64.h"

// Base64 編碼/解碼函數的實現
int base64_encode(char *dst, const char *src, int len)
{
    return ::base64_encode(dst, src, len);
}

int base64_decode(char *dst, const char *src)
{
    return ::base64_decode(dst, src);
}

int base64_encode_len(int len)
{
    return ::base64_encode_len(len);
}

int base64_decode_len(const char *src)
{
    return ::base64_decode_len(src);
}
