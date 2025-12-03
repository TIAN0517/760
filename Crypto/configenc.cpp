#include "stdafx.h"
#include "../WinLicenseSDK/WinLicenseSDK.h"
#include "rc6.h"
#include <cstdio>

namespace
{
void ConfigEncDebugLog(const char* stage, int len)
{
	if(len < 100000)
		return;
	FILE* fp = nullptr;
	if(fopen_s(&fp, "encoder_debug.log", "a+") == 0 && fp)
	{
		SYSTEMTIME st{};
		GetLocalTime(&st);
		fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d.%03d\tconfig_encrypt\t%s\tlen=%d\n",
				st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, stage, len);
		fclose(fp);
	}
}
}

void config_encrypt(const unsigned char* key, unsigned char* buffer, int len)
{
	int i;
	int MyCheckVar;

	ConfigEncDebugLog("start", len);
	VM_START

	unsigned long int next = 1;

	for (i = 0; i < 32; i++)
	{
		next = next * 1103515245 + key[i];
	}

	unsigned char xor_table[256];

	for (i = 0; i < 256; i++)
	{
		next = next * 1103515245 + i;
		xor_table[i] = next % 256;
	}
	ConfigEncDebugLog("xor_table", len);

	for (i = 0; i < 256; i++)
	{
		xor_table[i] ^= key[i % 32];
	}

	for (i = 0; i < len; i++)
	{
		buffer[i] ^= xor_table[i % 256];
	}
	ConfigEncDebugLog("xor_applied", len);

	CHECK_PROTECTION(MyCheckVar, 0x23bc1e6d);

	Rc6Context cnt;
	rc6Init(&cnt, key, 32);
	ConfigEncDebugLog("rc6_init", len);

	int count = len / 16;
	unsigned char* p = buffer;

	for (i = 0; i < count; i++)
	{
		rc6EncryptBlock(&cnt, p, p);
		p += 16;
	}
	ConfigEncDebugLog("rc6_done", len);

	if (MyCheckVar != 0x23bc1e6d) {
		__asm
		{
			rcl edi, 11h
			push edi
			mov ebx, ecx
			shr ecx, 2
			and ebx, 3
			pop ebp
			add esi, 4
			add edi, 4
			mov esp, ebp
			dec ecx
			xor ebp, ebp
			_emit 0xe9
			_emit 0xac
			_emit 0xc0
			_emit 0x44
			_emit 0x98
		}
	}

	VM_END
	ConfigEncDebugLog("end", len);
}

void config_decrypt(const unsigned char* key, unsigned char* buffer, int len)
{
	int MyCheckVar;
	int i;
	VM_START;
	Rc6Context cnt;
	rc6Init(&cnt, key, 32);
	int count = len / 16;
	unsigned char* p = buffer;

	for (i = 0; i < count; i++)
	{
		rc6DecryptBlock(&cnt, p, p);
		p += 16;
	}

	CHECK_PROTECTION(MyCheckVar, 0x404f1755);

	unsigned long int next = 1;

	for (i = 0; i < 32; i++)
	{
		next = next * 1103515245 + key[i];
	}

	unsigned char xor_table[256];

	for (i = 0; i < 256; i++)
	{
		next = next * 1103515245 + i;
		xor_table[i] = next % 256;
	}

	for (i = 0; i < 256; i++)
	{
		xor_table[i] ^= key[i % 32];
	}

	for (i = 0; i < len; i++)
	{
		buffer[i] ^= xor_table[i % 256];
	}

	if (MyCheckVar != 0x404f1755)
	{
		__asm
		{
			rcl edi, 11h
			push edi
			mov ebx, ecx
			shr ecx, 2
			and ebx, 3
			pop ebp
			add esi, 4
			add edi, 4
			mov esp, ebp
			dec ecx
			xor ebp, ebp
			_emit 0xe9
			_emit 0x7f
			_emit 0x87
			_emit 0x9a
			_emit 0xbf
		}
	}

	VM_END
}