#pragma once

// AES 加密/解密函數聲明
void aes_encrypt(const unsigned char *key, const unsigned char *input, unsigned char *output);
void aes_decrypt(const unsigned char *key, const unsigned char *input, unsigned char *output);
