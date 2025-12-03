#include "AES_TH.h"

// 小端：lowest-order byte 放在 index 0
std::array<uint8_t, 4> to_bytes_le(uint32_t x) noexcept {
	return {
		static_cast<uint8_t>(x & 0xFF),
		static_cast<uint8_t>((x >> 8) & 0xFF),
		static_cast<uint8_t>((x >> 16) & 0xFF),
		static_cast<uint8_t>((x >> 24) & 0xFF)
	};
}

static const uint32_t expand[64] = {
	0xDF532068U, 0x04644152U, 0x51B12408U, 0x6D4F5B3EU,
	0x53356C6FU, 0xC1157A77U, 0x6DBCE04AU, 0x087559D7U,
	0x5B3A3550U, 0x661F5CA4U, 0x1C635149U, 0xB1F3694FU,
	0x3416FE60U, 0xA56EE59AU, 0x38E078B7U, 0x2032EE3FU,
	0x402BBA63U, 0x29BE394EU, 0x75D540AAU, 0xFB5A73ABU,
	0xE480F666U, 0x57E63842U, 0x988689B6U, 0xE953037DU,
	0x6C786BEBU, 0x98E369FDU, 0x58C6A173U, 0x7FFA7F09U,
	0x52B74602U, 0x64B37B67U, 0xE92A5433U, 0xCF566555U,
	0xDC61798BU, 0x4DA46249U, 0xE2E7E121U, 0x86E86842U,
	0xE362DE1EU, 0x0567C45FU, 0x7E32CB0FU, 0x6A17D644U,
	0xE2F1F7A5U, 0xD966D447U, 0x5EA0D791U, 0x797ACA45U,
	0x485D435AU, 0x15ED8791U, 0xCDD9C11DU, 0x7E2209A6U,
	0x82AFFC8DU, 0x6771BCF8U, 0x8C0D58DDU, 0xCA144406U,
	0x6D5A022EU, 0x92AD4B0DU, 0x6B904CE6U, 0x87F03103U,
	0xAF8C504EU, 0x6FB7B874U, 0x7E1BE2BBU, 0x5CFA4BDEU,
	0x3C7CCFECU, 0x80B04445U, 0xBDC0B52FU, 0x937CCB4EU
};

inline void AdditionalEncrypt(uint8_t* data, uint32_t size, const uint8_t* key) {
	uint32_t expend_key[64] = { 0 };
	uint32_t temp = 0;

	for (size_t i = 0; i < 64; i++)
	{
		int r = (i % 4) * 4;
		temp = GETU32LE(key + r);
		expend_key[i] = expand[i] ^ temp;
	}

	for (size_t i = 0, expend_index; i < size; i++)
	{
		size_t r = i % 4;
		if (r == 0) expend_index = (i / 4) % 64;
		uint8_t byte_key = to_bytes_le(expend_key[expend_index])[r];
		data[i] = data[i] ^ byte_key;
	}
}

void AesEncryptExpand(uint32_t* roundkey, const uint8_t* key) {
	roundkey[0] = GETU32(key + 0);
	roundkey[1] = GETU32(key + 4);
	roundkey[2] = GETU32(key + 8);
	roundkey[3] = GETU32(key + 12);

	uint32_t ex_round = 0, round_key_index = 1;
	round_key_index = 1;
	for (int rcon_index = 0; rcon_index < 10; rcon_index++)
	{
		ex_round = roundkey[round_key_index + 2];
		roundkey[round_key_index + 3] =
			(Te3[(ex_round >> 0x08) & 0xFF] & 0x00FF0000) ^
			(Te2[(ex_round >> 0x10) & 0xFF] & 0xFF000000) ^
			(Te1[(ex_round >> 0x18) & 0xFF] & 0x000000FF) ^
			(Te0[ex_round & 0xFF] & 0x0000FF00) ^
			roundkey[round_key_index - 1] ^ rcon[rcon_index];
		roundkey[round_key_index + 4] = roundkey[round_key_index + 3] ^ roundkey[round_key_index];
		roundkey[round_key_index + 5] = roundkey[round_key_index + 4] ^ roundkey[round_key_index + 1];
		roundkey[round_key_index + 6] = roundkey[round_key_index + 5] ^ roundkey[round_key_index + 2];
		round_key_index += 4;
	}
}

void AesDecryptExpand(uint32_t* roundkey, const uint8_t* key) {
	AesEncryptExpand(roundkey, key);

	uint32_t temp;

	// Next, invert the order of the round keys.
	for (uint8_t i = 0, j = 40; i < j; i += 4, j -= 4) {
		temp = roundkey[i + 0];
		roundkey[i + 0] = roundkey[j + 0];
		roundkey[j + 0] = temp;

		temp = roundkey[i + 1];
		roundkey[i + 1] = roundkey[j + 1];
		roundkey[j + 1] = temp;

		temp = roundkey[i + 2];
		roundkey[i + 2] = roundkey[j + 2];
		roundkey[j + 2] = temp;

		temp = roundkey[i + 3];
		roundkey[i + 3] = roundkey[j + 3];
		roundkey[j + 3] = temp;
	}

	// Finally, apply the inverse MixColumn transform to all round keys except the first and last.
	for (uint8_t index = 4; index < 40; index += 4) {
		roundkey[index] =
			Td0[Te4[(roundkey[index] >> 24) & 0xff] & 0xff] ^
			Td1[Te4[(roundkey[index] >> 16) & 0xff] & 0xff] ^
			Td2[Te4[(roundkey[index] >> 8) & 0xff] & 0xff] ^
			Td3[Te4[(roundkey[index] >> 0) & 0xff] & 0xff];
		roundkey[index + 1] =
			Td0[Te4[(roundkey[index + 1] >> 24) & 0xff] & 0xff] ^
			Td1[Te4[(roundkey[index + 1] >> 16) & 0xff] & 0xff] ^
			Td2[Te4[(roundkey[index + 1] >> 8) & 0xff] & 0xff] ^
			Td3[Te4[(roundkey[index + 1] >> 0) & 0xff] & 0xff];
		roundkey[index + 2] =
			Td0[Te4[(roundkey[index + 2] >> 24) & 0xff] & 0xff] ^
			Td1[Te4[(roundkey[index + 2] >> 16) & 0xff] & 0xff] ^
			Td2[Te4[(roundkey[index + 2] >> 8) & 0xff] & 0xff] ^
			Td3[Te4[(roundkey[index + 2] >> 0) & 0xff] & 0xff];
		roundkey[index + 3] =
			Td0[Te4[(roundkey[index + 3] >> 24) & 0xff] & 0xff] ^
			Td1[Te4[(roundkey[index + 3] >> 16) & 0xff] & 0xff] ^
			Td2[Te4[(roundkey[index + 3] >> 8) & 0xff] & 0xff] ^
			Td3[Te4[(roundkey[index + 3] >> 0) & 0xff] & 0xff];
	}
}

void AesEncrypt(const uint32_t* roundkey, const uint8_t plaintext[AES_BLOCK_SIZE], uint8_t ciphertext[AES_BLOCK_SIZE]) {
	uint32_t s0, s1, s2, s3, t0, t1, t2, t3;

	s0 = GETU32(plaintext + 0x00) ^ roundkey[0];
	s1 = GETU32(plaintext + 0x04) ^ roundkey[1];
	s2 = GETU32(plaintext + 0x08) ^ roundkey[2];
	s3 = GETU32(plaintext + 0x0C) ^ roundkey[3];

	t0 = Te0[(s0 >> 0x18) & 0xFF] ^ Te1[(s1 >> 0x10) & 0xFF] ^ Te2[(s2 >> 0x08) & 0xFF] ^ Te3[s3 & 0xFF] ^ roundkey[4];//ebp-C
	t1 = Te0[(s1 >> 0x18) & 0xFF] ^ Te1[(s2 >> 0x10) & 0xFF] ^ Te2[(s3 >> 0x08) & 0xFF] ^ Te3[s0 & 0xFF] ^ roundkey[5];//ebp-4
	t2 = Te0[(s2 >> 0x18) & 0xFF] ^ Te1[(s3 >> 0x10) & 0xFF] ^ Te2[(s0 >> 0x08) & 0xFF] ^ Te3[s1 & 0xFF] ^ roundkey[6];//ebp-8
	t3 = Te0[(s3 >> 0x18) & 0xFF] ^ Te1[(s0 >> 0x10) & 0xFF] ^ Te2[(s1 >> 0x08) & 0xFF] ^ Te3[s2 & 0xFF] ^ roundkey[7];//ebx

	s0 = Te0[(t0 >> 0x18) & 0xFF] ^ Te1[(t1 >> 0x10) & 0xFF] ^ Te2[(t2 >> 0x08) & 0xFF] ^ Te3[t3 & 0xFF] ^ roundkey[8];//ebp-10
	s1 = Te0[(t1 >> 0x18) & 0xFF] ^ Te1[(t2 >> 0x10) & 0xFF] ^ Te2[(t3 >> 0x08) & 0xFF] ^ Te3[t0 & 0xFF] ^ roundkey[9];//ebp-14
	s2 = Te0[(t2 >> 0x18) & 0xFF] ^ Te1[(t3 >> 0x10) & 0xFF] ^ Te2[(t0 >> 0x08) & 0xFF] ^ Te3[t1 & 0xFF] ^ roundkey[10];//ebp-4
	s3 = Te0[(t3 >> 0x18) & 0xFF] ^ Te1[(t0 >> 0x10) & 0xFF] ^ Te2[(t1 >> 0x08) & 0xFF] ^ Te3[t2 & 0xFF] ^ roundkey[11];//edx

	t0 = Te0[(s0 >> 0x18) & 0xFF] ^ Te1[(s1 >> 0x10) & 0xFF] ^ Te2[(s2 >> 0x08) & 0xFF] ^ Te3[s3 & 0xFF] ^ roundkey[12];//ebp-C
	t1 = Te0[(s1 >> 0x18) & 0xFF] ^ Te1[(s2 >> 0x10) & 0xFF] ^ Te2[(s3 >> 0x08) & 0xFF] ^ Te3[s0 & 0xFF] ^ roundkey[13];//ebp-8
	t2 = Te0[(s2 >> 0x18) & 0xFF] ^ Te1[(s3 >> 0x10) & 0xFF] ^ Te2[(s0 >> 0x08) & 0xFF] ^ Te3[s1 & 0xFF] ^ roundkey[14];//ebp-4
	t3 = Te0[(s3 >> 0x18) & 0xFF] ^ Te1[(s0 >> 0x10) & 0xFF] ^ Te2[(s1 >> 0x08) & 0xFF] ^ Te3[s2 & 0xFF] ^ roundkey[15];//ebx

	s0 = Te0[(t0 >> 0x18) & 0xFF] ^ Te1[(t1 >> 0x10) & 0xFF] ^ Te2[(t2 >> 0x08) & 0xFF] ^ Te3[t3 & 0xFF] ^ roundkey[16];//ebp-10
	s1 = Te0[(t1 >> 0x18) & 0xFF] ^ Te1[(t2 >> 0x10) & 0xFF] ^ Te2[(t3 >> 0x08) & 0xFF] ^ Te3[t0 & 0xFF] ^ roundkey[17];//ebp-14
	s2 = Te0[(t2 >> 0x18) & 0xFF] ^ Te1[(t3 >> 0x10) & 0xFF] ^ Te2[(t0 >> 0x08) & 0xFF] ^ Te3[t1 & 0xFF] ^ roundkey[18];//ebp-8
	s3 = Te0[(t3 >> 0x18) & 0xFF] ^ Te1[(t0 >> 0x10) & 0xFF] ^ Te2[(t1 >> 0x08) & 0xFF] ^ Te3[t2 & 0xFF] ^ roundkey[19];//edx

	t0 = Te0[(s0 >> 0x18) & 0xFF] ^ Te1[(s1 >> 0x10) & 0xFF] ^ Te2[(s2 >> 0x08) & 0xFF] ^ Te3[s3 & 0xFF] ^ roundkey[20];//ebp-C
	t1 = Te0[(s1 >> 0x18) & 0xFF] ^ Te1[(s2 >> 0x10) & 0xFF] ^ Te2[(s3 >> 0x08) & 0xFF] ^ Te3[s0 & 0xFF] ^ roundkey[21];//ebp-4
	t2 = Te0[(s2 >> 0x18) & 0xFF] ^ Te1[(s3 >> 0x10) & 0xFF] ^ Te2[(s0 >> 0x08) & 0xFF] ^ Te3[s1 & 0xFF] ^ roundkey[22];//ebp-8
	t3 = Te0[(s3 >> 0x18) & 0xFF] ^ Te1[(s0 >> 0x10) & 0xFF] ^ Te2[(s1 >> 0x08) & 0xFF] ^ Te3[s2 & 0xFF] ^ roundkey[23];//ebx

	s0 = Te0[(t0 >> 0x18) & 0xFF] ^ Te1[(t1 >> 0x10) & 0xFF] ^ Te2[(t2 >> 0x08) & 0xFF] ^ Te3[t3 & 0xFF] ^ roundkey[24];//ebp-10
	s1 = Te0[(t1 >> 0x18) & 0xFF] ^ Te1[(t2 >> 0x10) & 0xFF] ^ Te2[(t3 >> 0x08) & 0xFF] ^ Te3[t0 & 0xFF] ^ roundkey[25];//ebp-14
	s2 = Te0[(t2 >> 0x18) & 0xFF] ^ Te1[(t3 >> 0x10) & 0xFF] ^ Te2[(t0 >> 0x08) & 0xFF] ^ Te3[t1 & 0xFF] ^ roundkey[26];//ebp-4
	s3 = Te0[(t3 >> 0x18) & 0xFF] ^ Te1[(t0 >> 0x10) & 0xFF] ^ Te2[(t1 >> 0x08) & 0xFF] ^ Te3[t2 & 0xFF] ^ roundkey[27];//edx

	t0 = Te0[(s0 >> 0x18) & 0xFF] ^ Te1[(s1 >> 0x10) & 0xFF] ^ Te2[(s2 >> 0x08) & 0xFF] ^ Te3[s3 & 0xFF] ^ roundkey[28];//ebp-C
	t1 = Te0[(s1 >> 0x18) & 0xFF] ^ Te1[(s2 >> 0x10) & 0xFF] ^ Te2[(s3 >> 0x08) & 0xFF] ^ Te3[s0 & 0xFF] ^ roundkey[29];//ebp-18
	t2 = Te0[(s2 >> 0x18) & 0xFF] ^ Te1[(s3 >> 0x10) & 0xFF] ^ Te2[(s0 >> 0x08) & 0xFF] ^ Te3[s1 & 0xFF] ^ roundkey[30];//ebp-4
	t3 = Te0[(s3 >> 0x18) & 0xFF] ^ Te1[(s0 >> 0x10) & 0xFF] ^ Te2[(s1 >> 0x08) & 0xFF] ^ Te3[s2 & 0xFF] ^ roundkey[31];//ebx

	s0 = Te0[(t0 >> 0x18) & 0xFF] ^ Te1[(t1 >> 0x10) & 0xFF] ^ Te2[(t2 >> 0x08) & 0xFF] ^ Te3[t3 & 0xFF] ^ roundkey[32];//ebp-8
	s1 = Te0[(t1 >> 0x18) & 0xFF] ^ Te1[(t2 >> 0x10) & 0xFF] ^ Te2[(t3 >> 0x08) & 0xFF] ^ Te3[t0 & 0xFF] ^ roundkey[33];//ebp-14
	s2 = Te0[(t2 >> 0x18) & 0xFF] ^ Te1[(t3 >> 0x10) & 0xFF] ^ Te2[(t0 >> 0x08) & 0xFF] ^ Te3[t1 & 0xFF] ^ roundkey[34];//ebp-1C
	s3 = Te0[(t3 >> 0x18) & 0xFF] ^ Te1[(t0 >> 0x10) & 0xFF] ^ Te2[(t1 >> 0x08) & 0xFF] ^ Te3[t2 & 0xFF] ^ roundkey[35];//edx

	t0 = Te0[(s0 >> 0x18) & 0xFF] ^ Te1[(s1 >> 0x10) & 0xFF] ^ Te2[(s2 >> 0x08) & 0xFF] ^ Te3[s3 & 0xFF] ^ roundkey[36];//ebp-C
	t1 = Te0[(s1 >> 0x18) & 0xFF] ^ Te1[(s2 >> 0x10) & 0xFF] ^ Te2[(s3 >> 0x08) & 0xFF] ^ Te3[s0 & 0xFF] ^ roundkey[37];//ebp-10
	t2 = Te0[(s2 >> 0x18) & 0xFF] ^ Te1[(s3 >> 0x10) & 0xFF] ^ Te2[(s0 >> 0x08) & 0xFF] ^ Te3[s1 & 0xFF] ^ roundkey[38];//ebp-4
	t3 = Te0[(s3 >> 0x18) & 0xFF] ^ Te1[(s0 >> 0x10) & 0xFF] ^ Te2[(s1 >> 0x08) & 0xFF] ^ Te3[s2 & 0xFF] ^ roundkey[39];//ebp-8

	s0 =
		(Te2[(t0 >> 24) & 0xff] & 0xFF000000) ^
		(Te3[(t1 >> 16) & 0xff] & 0x00FF0000) ^
		(Te0[(t2 >> 8) & 0xff] & 0x0000FF00) ^
		(Te1[(t3 >> 0) & 0xff] & 0x000000FF) ^ roundkey[40];
	PUTU32(ciphertext, s0);
	s1 =
		(Te2[(t1 >> 24) & 0xff] & 0xFF000000) ^
		(Te3[(t2 >> 16) & 0xff] & 0x00FF0000) ^
		(Te0[(t3 >> 8) & 0xff] & 0x0000FF00) ^
		(Te1[(t0 >> 0) & 0xff] & 0x000000FF) ^ roundkey[41];
	PUTU32(ciphertext + 0x04, s1);
	s2 =
		(Te2[(t2 >> 24) & 0xff] & 0xFF000000) ^
		(Te3[(t3 >> 16) & 0xff] & 0x00FF0000) ^
		(Te0[(t0 >> 8) & 0xff] & 0x0000FF00) ^
		(Te1[(t1 >> 0) & 0xff] & 0x000000FF) ^ roundkey[42];
	PUTU32(ciphertext + 0x08, s2);
	s3 =
		(Te2[(t3 >> 24) & 0xff] & 0xFF000000) ^
		(Te3[(t0 >> 16) & 0xff] & 0x00FF0000) ^
		(Te0[(t1 >> 8) & 0xff] & 0x0000FF00) ^
		(Te1[(t2 >> 0) & 0xff] & 0x000000FF) ^ roundkey[43];
	PUTU32(ciphertext + 0x0C, s3);
}

void AesDecrypt(const uint32_t* roundkey, const uint8_t ciphertext[AES_BLOCK_SIZE], uint8_t plaintext[AES_BLOCK_SIZE]) {
	uint32_t s0, s1, s2, s3, t0, t1, t2, t3;

	/*
	 * map byte array block to cipher state
	 * and add initial round key:
	 */
	s0 = GETU32(ciphertext) ^ roundkey[0];
	s1 = GETU32(ciphertext + 4) ^ roundkey[1];
	s2 = GETU32(ciphertext + 8) ^ roundkey[2];
	s3 = GETU32(ciphertext + 12) ^ roundkey[3];

	/* round 1: */
	t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >> 8) & 0xff] ^ Td3[s1 & 0xff] ^ roundkey[4];
	t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >> 8) & 0xff] ^ Td3[s2 & 0xff] ^ roundkey[5];
	t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >> 8) & 0xff] ^ Td3[s3 & 0xff] ^ roundkey[6];
	t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >> 8) & 0xff] ^ Td3[s0 & 0xff] ^ roundkey[7];
	/* round 2: */
	s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >> 8) & 0xff] ^ Td3[t1 & 0xff] ^ roundkey[8];
	s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >> 8) & 0xff] ^ Td3[t2 & 0xff] ^ roundkey[9];
	s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >> 8) & 0xff] ^ Td3[t3 & 0xff] ^ roundkey[10];
	s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >> 8) & 0xff] ^ Td3[t0 & 0xff] ^ roundkey[11];
	/* round 3: */
	t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >> 8) & 0xff] ^ Td3[s1 & 0xff] ^ roundkey[12];
	t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >> 8) & 0xff] ^ Td3[s2 & 0xff] ^ roundkey[13];
	t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >> 8) & 0xff] ^ Td3[s3 & 0xff] ^ roundkey[14];
	t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >> 8) & 0xff] ^ Td3[s0 & 0xff] ^ roundkey[15];
	/* round 4: */
	s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >> 8) & 0xff] ^ Td3[t1 & 0xff] ^ roundkey[16];
	s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >> 8) & 0xff] ^ Td3[t2 & 0xff] ^ roundkey[17];
	s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >> 8) & 0xff] ^ Td3[t3 & 0xff] ^ roundkey[18];
	s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >> 8) & 0xff] ^ Td3[t0 & 0xff] ^ roundkey[19];
	/* round 5: */
	t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >> 8) & 0xff] ^ Td3[s1 & 0xff] ^ roundkey[20];
	t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >> 8) & 0xff] ^ Td3[s2 & 0xff] ^ roundkey[21];
	t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >> 8) & 0xff] ^ Td3[s3 & 0xff] ^ roundkey[22];
	t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >> 8) & 0xff] ^ Td3[s0 & 0xff] ^ roundkey[23];
	/* round 6: */
	s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >> 8) & 0xff] ^ Td3[t1 & 0xff] ^ roundkey[24];
	s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >> 8) & 0xff] ^ Td3[t2 & 0xff] ^ roundkey[25];
	s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >> 8) & 0xff] ^ Td3[t3 & 0xff] ^ roundkey[26];
	s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >> 8) & 0xff] ^ Td3[t0 & 0xff] ^ roundkey[27];
	/* round 7: */
	t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >> 8) & 0xff] ^ Td3[s1 & 0xff] ^ roundkey[28];
	t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >> 8) & 0xff] ^ Td3[s2 & 0xff] ^ roundkey[29];
	t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >> 8) & 0xff] ^ Td3[s3 & 0xff] ^ roundkey[30];
	t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >> 8) & 0xff] ^ Td3[s0 & 0xff] ^ roundkey[31];
	/* round 8: */
	s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >> 8) & 0xff] ^ Td3[t1 & 0xff] ^ roundkey[32];
	s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >> 8) & 0xff] ^ Td3[t2 & 0xff] ^ roundkey[33];
	s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >> 8) & 0xff] ^ Td3[t3 & 0xff] ^ roundkey[34];
	s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >> 8) & 0xff] ^ Td3[t0 & 0xff] ^ roundkey[35];
	/* round 9: */
	t0 = Td0[s0 >> 24] ^ Td1[(s3 >> 16) & 0xff] ^ Td2[(s2 >> 8) & 0xff] ^ Td3[s1 & 0xff] ^ roundkey[36];
	t1 = Td0[s1 >> 24] ^ Td1[(s0 >> 16) & 0xff] ^ Td2[(s3 >> 8) & 0xff] ^ Td3[s2 & 0xff] ^ roundkey[37];
	t2 = Td0[s2 >> 24] ^ Td1[(s1 >> 16) & 0xff] ^ Td2[(s0 >> 8) & 0xff] ^ Td3[s3 & 0xff] ^ roundkey[38];
	t3 = Td0[s3 >> 24] ^ Td1[(s2 >> 16) & 0xff] ^ Td2[(s1 >> 8) & 0xff] ^ Td3[s0 & 0xff] ^ roundkey[39];
	/* round 10: */
	s0 = Td0[t0 >> 24] ^ Td1[(t3 >> 16) & 0xff] ^ Td2[(t2 >> 8) & 0xff] ^ Td3[t1 & 0xff] ^ roundkey[40];
	s1 = Td0[t1 >> 24] ^ Td1[(t0 >> 16) & 0xff] ^ Td2[(t3 >> 8) & 0xff] ^ Td3[t2 & 0xff] ^ roundkey[41];
	s2 = Td0[t2 >> 24] ^ Td1[(t1 >> 16) & 0xff] ^ Td2[(t0 >> 8) & 0xff] ^ Td3[t3 & 0xff] ^ roundkey[42];
	s3 = Td0[t3 >> 24] ^ Td1[(t2 >> 16) & 0xff] ^ Td2[(t1 >> 8) & 0xff] ^ Td3[t0 & 0xff] ^ roundkey[43];

	/*
	 * apply last round and
	 * map cipher state to byte array block:
	 */
	s0 =
		(Td4[(t0 >> 24) & 0xff] & 0xff000000) ^
		(Td4[(t3 >> 16) & 0xff] & 0x00ff0000) ^
		(Td4[(t2 >> 8) & 0xff] & 0x0000ff00) ^
		(Td4[(t1 >> 0) & 0xff] & 0x000000ff) ^
		roundkey[40];
	PUTU32(plaintext, s0);

	s1 =
		(Td4[(t1 >> 24) & 0xff] & 0xff000000) ^
		(Td4[(t0 >> 16) & 0xff] & 0x00ff0000) ^
		(Td4[(t3 >> 8) & 0xff] & 0x0000ff00) ^
		(Td4[(t2 >> 0) & 0xff] & 0x000000ff) ^
		roundkey[41];
	PUTU32(plaintext + 4, s1);

	s2 =
		(Td4[(t2 >> 24) & 0xff] & 0xff000000) ^
		(Td4[(t1 >> 16) & 0xff] & 0x00ff0000) ^
		(Td4[(t0 >> 8) & 0xff] & 0x0000ff00) ^
		(Td4[(t3 >> 0) & 0xff] & 0x000000ff) ^
		roundkey[42];
	PUTU32(plaintext + 8, s2);

	s3 =
		(Td4[(t3 >> 24) & 0xff] & 0xff000000) ^
		(Td4[(t2 >> 16) & 0xff] & 0x00ff0000) ^
		(Td4[(t1 >> 8) & 0xff] & 0x0000ff00) ^
		(Td4[(t0 >> 0) & 0xff] & 0x000000ff) ^
		roundkey[43];
	PUTU32(plaintext + 12, s3);
}

// AES-128 ECB Encrypt buffer 
void EncryptData(uint8_t* data, uint32_t size, const uint8_t* key) {
	uint32_t roundkey[44];

	AesEncryptExpand(roundkey, key);

	AdditionalEncrypt(data, size, key);

	for (uint32_t offset = 0; offset < size; offset += AES_BLOCK_SIZE) {
		if (offset + 16 > size) {
			//uint8_t padding = offset + AES_BLOCK_SIZE - size;
			//區塊不足填0
			for (int pi = offset + 16 - 1; pi >= size; pi--) {
				data[pi] = 0;
			}
		}
		AesEncrypt(roundkey, data + offset, data + offset);
	}
}

// AES-128 ECB Decrypt buffer 
void DecryptData(uint8_t* data, uint32_t size, const uint8_t* key) {
	uint32_t roundkey[44];
	uint8_t block[AES_BLOCK_SIZE];
	uint8_t prev_block[AES_BLOCK_SIZE];

	AesDecryptExpand(roundkey, key);

	for (uint32_t offset = 0; offset < size; offset += AES_BLOCK_SIZE) {
		AesDecrypt(roundkey, data + offset, data + offset); // Decrypt the block
	}

	AdditionalEncrypt(data, size, key);
}

void PrintHex(const void* data, size_t byteCount, size_t bytesPerLine) {
	auto p = reinterpret_cast<const unsigned char*>(data);
	std::printf("Memory (%zu bytes):\n", byteCount);
	for (size_t i = 0; i < byteCount; ++i) {
		std::printf("%02X ", p[i]);
		if ((i + 1) % bytesPerLine == 0) std::printf("\n");
	}
	if (byteCount % bytesPerLine) std::printf("\n");
}
