#pragma once

// 配置加密/解密函數聲明
void config_encrypt(const unsigned char* key, unsigned char* buffer, int len);
void config_decrypt(const unsigned char* key, unsigned char* buffer, int len);
