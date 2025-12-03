#pragma once

// Base64 編碼/解碼函數聲明
int base64_encode(char *dst, const char *src, int len);
int base64_decode(char *dst, const char *src);
int base64_encode_len(int len);
int base64_decode_len(const char *src);
