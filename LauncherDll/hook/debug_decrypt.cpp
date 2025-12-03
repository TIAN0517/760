/**
 * DEBUG Decryption Test Tool
 * 用於測試 BD 檔案解密演算法
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// 模擬 config_decrypt 過程，用於調試
void debug_config_decrypt(const unsigned char* key, unsigned char* buffer, int len)
{
	printf("=== DEBUG config_decrypt ===\n");
	printf("Key (first 16 bytes): ");
	for (int i = 0; i < 16 && i < 32; i++) {
		printf("%02X ", key[i]);
	}
	printf("\n");
	printf("Buffer length: %d\n", len);
	printf("Buffer first 16 bytes (before RC6): ");
	for (int i = 0; i < 16 && i < len; i++) {
		printf("%02X ", buffer[i]);
	}
	printf("\n");
	
	// 模擬 LCG XOR 表生成
	unsigned long int next = 1;
	
	// Seed LCG with key
	printf("\nSeeding LCG with key...\n");
	for (int i = 0; i < 32; i++)
	{
		next = next * 1103515245 + key[i];
	}
	printf("LCG seed state: 0x%lX\n", next);
	
	// Generate base XOR table
	unsigned char xor_table[256];
	printf("\nGenerating XOR table base values...\n");
	for (int i = 0; i < 256; i++)
	{
		next = next * 1103515245 + i;
		xor_table[i] = next % 256;
	}
	printf("XOR table first 16 values: ");
	for (int i = 0; i < 16; i++) {
		printf("%02X ", xor_table[i]);
	}
	printf("\n");
	
	// Step 2: Combine with 32-byte key
	printf("\nCombining XOR table with 32-byte key...\n");
	for (int i = 0; i < 256; i++)
	{
		unsigned char before = xor_table[i];
		xor_table[i] ^= key[i % 32];
		if (i < 16) {
			printf("xor_table[%d]: %02X -> %02X (XOR with key[%d]=%02X)\n", 
				i, before, xor_table[i], i % 32, key[i % 32]);
		}
	}
	printf("XOR table after combining first 16 values: ");
	for (int i = 0; i < 16; i++) {
		printf("%02X ", xor_table[i]);
	}
	printf("\n");
	
	// Apply XOR to first few bytes for debugging
	printf("\nApplying XOR to buffer (first 16 bytes):\n");
	for (int i = 0; i < 16 && i < len; i++)
	{
		unsigned char before = buffer[i];
		buffer[i] ^= xor_table[i % 256];
		printf("buffer[%d]: %02X -> %02X (XOR with xor_table[%d]=%02X)\n",
			i, before, buffer[i], i % 256, xor_table[i % 256]);
	}
}

int main()
{
	printf("Debug Decrypt Test\n");
	printf("==================\n\n");
	
	// Test with known key from recent log
	unsigned char test_key[] = {
		0x54, 0xD9, 0x20, 0x74, 0xEC, 0x55, 0x17, 0x89, 
		0xFC, 0x79, 0xF3, 0x66, 0x56, 0xDD, 0xB3, 0x9B, 
		0x97, 0xB7, 0x8F, 0x13, 0x51, 0x64, 0x38, 0x47, 
		0x69, 0x44, 0x60, 0xB1, 0xBD, 0xCB, 0x7D, 0x84
	};
	
	// Test encrypted payload (from log: first bytes 13 E7)
	unsigned char test_buffer[] = {
		0x13, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	
	debug_config_decrypt(test_key, test_buffer, 16);
	
	printf("\nExpected result (after RC6 + XOR): 78 9C ... (zlib magic)\n");
	printf("Actual result first 2 bytes: %02X %02X\n", test_buffer[0], test_buffer[1]);
	
	return 0;
}
