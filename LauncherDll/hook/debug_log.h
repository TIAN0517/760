/**
 * LauncherDll DEBUG 日誌工具
 * 用於詳細記錄 BD 解密過程
 */

#pragma once

#include <stdio.h>
#include <time.h>
#include <windows.h>

inline void DebugDecryptLog(const char* format, ...)
{
	// ❌ DISABLED - 取消 launcherdll_decrypt_debug.log 記錄
	/*
	FILE* fp = nullptr;
	if (fopen_s(&fp, "launcherdll_decrypt_debug.log", "a+") == 0 && fp)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] ",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		
		va_list args;
		va_start(args, format);
		vfprintf(fp, format, args);
		va_end(args);
		
		fprintf(fp, "\n");
		fclose(fp);
	}
	*/
}

// 記錄 XOR 表生成過程
inline void DebugLogXorTable(const unsigned char* key, const unsigned char* xor_table)
{
	// ❌ DISABLED - 取消 launcherdll_decrypt_debug.log 記錄
	/*
	FILE* fp = nullptr;
	if (fopen_s(&fp, "launcherdll_decrypt_debug.log", "a+") == 0 && fp)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] XOR TABLE (first 256 bytes):\n",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		
		for (int i = 0; i < 256; i++)
		{
			if (i % 16 == 0) fprintf(fp, "  [%3d-%3d]: ", i, i + 15);
			fprintf(fp, "%02X ", xor_table[i]);
			if ((i + 1) % 16 == 0) fprintf(fp, "\n");
		}
		fprintf(fp, "\n");
		fclose(fp);
	}
	*/
}

// 記錄解密前後的資料對比
inline void DebugLogPayloadChange(const char* stage, const unsigned char* before, const unsigned char* after, int len)
{
	// ❌ DISABLED - 取消 launcherdll_decrypt_debug.log 記錄
	/*
	FILE* fp = nullptr;
	if (fopen_s(&fp, "launcherdll_decrypt_debug.log", "a+") == 0 && fp)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s (first 32 bytes):\n",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, stage);
		fprintf(fp, "  Before: ");
		for (int i = 0; i < 32 && i < len; i++) fprintf(fp, "%02X ", before[i]);
		fprintf(fp, "\n  After:  ");
		for (int i = 0; i < 32 && i < len; i++) fprintf(fp, "%02X ", after[i]);
		fprintf(fp, "\n\n");
		fclose(fp);
	}
	*/
}
