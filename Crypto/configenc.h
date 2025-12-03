#pragma once

void config_encrypt(const unsigned char* key, unsigned char* buffer, int len);
void config_decrypt(const unsigned char* key, unsigned char* buffer, int len);