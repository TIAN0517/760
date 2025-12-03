#include "stdafx.h"
#include "aes.h"
#include "../Crypto/aes.h"

// AES 加密/解密函數的實現
void aes_encrypt(const unsigned char *key, const unsigned char *input, unsigned char *output)
{
    // 調用 Crypto 目錄中的實現
    ::aes_encrypt(key, input, output);
}

void aes_decrypt(const unsigned char *key, const unsigned char *input, unsigned char *output)
{
    // 調用 Crypto 目錄中的實現
    ::aes_decrypt(key, input, output);
}
