#include "stdafx.h"
#include "configenc.h"
#include "../Crypto/configenc.h"

// 配置加密/解密函數的實現
void config_encrypt(const unsigned char* key, unsigned char* buffer, int len)
{
    // 調用 Crypto 目錄中的實現
    ::config_encrypt(key, buffer, len);
}

void config_decrypt(const unsigned char* key, unsigned char* buffer, int len)
{
    // 調用 Crypto 目錄中的實現
    ::config_decrypt(key, buffer, len);
}
