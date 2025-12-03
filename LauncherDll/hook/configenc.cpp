#include "stdafx.h"
#include "stdafx.h"
#include "configenc.h"
#include "rc6.h"
#include "debug_log.h"

// ============================================================================
// config_encrypt - Matches Encoder implementation exactly
// ============================================================================
void config_encrypt(const unsigned char* key, unsigned char* buffer, int len)
{
	// Step 1: Generate XOR table using LCG (Linear Congruential Generator)
	unsigned long int next = 1;
	
	// Seed LCG with key
	for (int i = 0; i < 32; i++)
	{
		next = next * 1103515245 + key[i];
	}
	
	// Generate base XOR table
	unsigned char xor_table[256];
	for (int i = 0; i < 256; i++)
	{
		next = next * 1103515245 + i;
		xor_table[i] = next % 256;
	}
	
	// Step 2: Combine with 32-byte key (XOR each table entry with key bytes)
	for (int i = 0; i < 256; i++)
	{
		xor_table[i] ^= key[i % 32];
	}
	
	// Step 3: Apply XOR table to buffer
	for (int i = 0; i < len; i++)
	{
		buffer[i] ^= xor_table[i % 256];
	}
	
	// Step 4: RC6 encrypt the XORed buffer
	Rc6Context cnt;
	rc6Init(&cnt, key, 32);
	
	int count = len / 16;
	unsigned char* p = buffer;
	for (int i = 0; i < count; i++)
	{
		rc6EncryptBlock(&cnt, p, p);
		p += 16;
	}
}

// ============================================================================
// config_decrypt - Inverse of config_encrypt
// ============================================================================
void config_decrypt(const unsigned char* key, unsigned char* buffer, int len)
{
	DebugDecryptLog("=== config_decrypt START ===");
	DebugDecryptLog("Key (first 16 bytes): %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7],
		key[8], key[9], key[10], key[11], key[12], key[13], key[14], key[15]);
	DebugDecryptLog("Buffer length: %d", len);
	DebugDecryptLog("Buffer first 16 bytes (BEFORE): %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7],
		buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);
	
	// Step 1: RC6 decrypt the buffer
	DebugDecryptLog("Step 1: RC6 Decryption");
	Rc6Context cnt;
	rc6Init(&cnt, key, 32);
	
	int count = len / 16;
	unsigned char* p = buffer;
	for (int i = 0; i < count; i++)
	{
		rc6DecryptBlock(&cnt, p, p);
		p += 16;
	}
	DebugDecryptLog("RC6 decryption completed, processed %d blocks", count);
	DebugDecryptLog("Buffer after RC6 (first 16 bytes): %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7],
		buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);
	
	// Step 2: Generate XOR table using LCG (same as encrypt)
	DebugDecryptLog("Step 2: Generating LCG XOR table");
	unsigned long int next = 1;
	
	// Seed LCG with key
	for (int i = 0; i < 32; i++)
	{
		next = next * 1103515245 + key[i];
	}
	DebugDecryptLog("LCG seed state: 0x%lX", next);
	
	// Generate base XOR table
	unsigned char xor_table[256];
	for (int i = 0; i < 256; i++)
	{
		next = next * 1103515245 + i;
		xor_table[i] = next % 256;
	}
	DebugDecryptLog("XOR table base generated, first 16 values: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		xor_table[0], xor_table[1], xor_table[2], xor_table[3], xor_table[4], xor_table[5], xor_table[6], xor_table[7],
		xor_table[8], xor_table[9], xor_table[10], xor_table[11], xor_table[12], xor_table[13], xor_table[14], xor_table[15]);
	
	// Step 3: Combine with 32-byte key
	DebugDecryptLog("Step 3: Combining XOR table with 32-byte key");
	for (int i = 0; i < 256; i++)
	{
		xor_table[i] ^= key[i % 32];
	}
	DebugDecryptLog("XOR table after key combine, first 16 values: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		xor_table[0], xor_table[1], xor_table[2], xor_table[3], xor_table[4], xor_table[5], xor_table[6], xor_table[7],
		xor_table[8], xor_table[9], xor_table[10], xor_table[11], xor_table[12], xor_table[13], xor_table[14], xor_table[15]);
	
	// Step 4: Apply XOR table to buffer (XOR is self-inverse)
	DebugDecryptLog("Step 4: Applying XOR table to buffer");
	for (int i = 0; i < len; i++)
	{
		buffer[i] ^= xor_table[i % 256];
	}
	DebugDecryptLog("XOR applied to %d bytes", len);
	DebugDecryptLog("Buffer AFTER XOR (first 16 bytes): %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
		buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7],
		buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);
	
	DebugDecryptLog("=== config_decrypt END ===");
}
