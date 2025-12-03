#include "stdafx.h"
#include <cstring>
#include "Crypto.h"
#include "rc6.h"

#define P32 0xB7E15163
#define Q32 0x9E3779B9

int rc6Init(Rc6Context *context, const BYTE *key, size_t keyLen)
{
	DWORD c;
	DWORD i;
	DWORD j;
	DWORD s;
	DWORD v;
	DWORD a;
	DWORD b;
	
	//Check parameters
	if(context == NULL || key == NULL)
		return -1;
	
	//Invalid key length?
	if(keyLen > RC6_MAX_KEY_SIZE)
		return -2;
	
	//Convert the secret key from bytes to words
	memset(context->l, 0, RC6_MAX_KEY_SIZE);
	memcpy(context->l, key, keyLen);
	
	//Calculate the length of the key in words
	c = (keyLen > 0) ? (keyLen + 3) / 4 : 1;
	
	//Initialize the first element of S
	context->s[0] = P32;
	
	//Initialize array S to a particular fixed pseudo random bit pattern
	for(i = 1; i < (2 * RC6_NB_ROUNDS + 4); i++)
	{
		context->s[i] = context->s[i - 1] + Q32;
	}
	
	//Initialize variables
	i = 0;
	j = 0;
	a = 0;
	b = 0;
	
	//Number of iterations
	v = 3 * max(c, 2 * RC6_NB_ROUNDS + 4);
	
	//Key expansion
	for(s = 0; s < v; s++)
	{
		context->s[i] += a + b;
		context->s[i] = ROL32(context->s[i], 3);
		a = context->s[i];
	
		context->l[j] += a + b;
		context->l[j] = ROL32(context->l[j], (a + b) % 32);
		b = context->l[j];
	
		if(++i >= (2 * RC6_NB_ROUNDS + 4))
		{
			i = 0;
		}
	
		if(++j >= c)
		{
			j = 0;
		}
	}
	
	// No error to report
	return 0;
 }
	
	
/**
 * @brief Encrypt a 16-byte block using RC6 algorithm
 * @param[in] context Pointer to the RC6 context
 * @param[in] input Plaintext block to encrypt
 * @param[out] output Ciphertext block resulting from encryption
 **/
void rc6EncryptBlock(Rc6Context *context, const BYTE *input, BYTE *output)
{
	DWORD i;
	DWORD t;
	DWORD u;
	
	//Load the 4 working registers with the plaintext
	DWORD a = LOAD32LE(input + 0);
	DWORD b = LOAD32LE(input + 4);
	DWORD c = LOAD32LE(input + 8);
	DWORD d = LOAD32LE(input + 12);
	
	//First, update B and D
	b += context->s[0];
	d += context->s[1];
	
	//Apply 20 rounds
	for(i = 1; i <= RC6_NB_ROUNDS; i++)
	{
		t = (b * (2 * b + 1));
		t = ROL32(t, 5);
	
		u = (d * (2 * d + 1));
		u = ROL32(u, 5);
	
		a ^= t;
		a = ROL32(a, u % 32) + context->s[2 * i];
	
		c ^= u;
		c = ROL32(c, t % 32) + context->s[2 * i + 1];
	
		t = a;
		a = b;
		b = c;
		c = d;
		d = t;
	}
	
	//Update A and C
	a += context->s[2 * RC6_NB_ROUNDS + 2];
	c += context->s[2 * RC6_NB_ROUNDS + 3];
	
	//The resulting value is the ciphertext
	STORE32LE(a, output + 0);
	STORE32LE(b, output + 4);
	STORE32LE(c, output + 8);
	STORE32LE(d, output + 12);
 }
	
	
 /**
	* @brief Decrypt a 16-byte block using RC6 algorithm
	* @param[in] context Pointer to the RC6 context
	* @param[in] input Ciphertext block to decrypt
	* @param[out] output Plaintext block resulting from decryption
	**/
	
void rc6DecryptBlock(Rc6Context *context, const BYTE *input, BYTE *output)
{
	DWORD i;
	DWORD t;
	DWORD u;
	
	// Load the 4 working registers with the ciphertext
	DWORD a = LOAD32LE(input + 0);
	DWORD b = LOAD32LE(input + 4);
	DWORD c = LOAD32LE(input + 8);
	DWORD d = LOAD32LE(input + 12);
	
	//First, update C and A
	c -= context->s[2 * RC6_NB_ROUNDS + 3];
	a -= context->s[2 * RC6_NB_ROUNDS + 2];
	
	//Apply 20 rounds
	for(i = RC6_NB_ROUNDS; i > 0; i--)
	{
		t = d;
		d = c;
		c = b;
		b = a;
		a = t;
		
		u = (d * (2 * d + 1));
		u = ROL32(u, 5);
		
		t = (b * (2 * b + 1));
		t = ROL32(t, 5);
		
		c -= context->s[2 * i + 1];
		c = ROR32(c, t % 32) ^ u;
		
		a -= context->s[2 * i];
		a = ROR32(a, u % 32) ^ t;
	}
	
	// Update D and B
	d -= context->s[1];
	b -= context->s[0];
	
	// The resulting value is the plaintext
	STORE32LE(a, output + 0);
	STORE32LE(b, output + 4);
	STORE32LE(c, output + 8);
	STORE32LE(d, output + 12);
}