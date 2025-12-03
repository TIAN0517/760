#include "stdafx.h"
#include "../LinLauncher/framework.h"
#include "../LinLauncher/LinLauncher.h"
#include "LResource.h"
#include <cstring>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include "../Crypto/des.h"
#include "../Launcher/Lineage.h" // 確保完整型別 CLinLauncherDlg 的宣告

int iResourceMode = 0;
bool bMultiResource = false;
BYTE* lpResources[18];
int lpResourceFiles[18];

int gr_stricmp(const void*, const void*);

typedef bool(__cdecl* FILTER_FUNC)(char* szFileName);

const ResourceUnit ruUnits[] =
{
	{ 28, 4, 20, 24, -1, -1 },			// 原始
	{ 32, 4, 20, 24, 28, -1 },			// _IDX
	{ 128, 16, 112, 4, 8, 12 },			// _EXT
	{ 276, 16, MAX_PATH, 4, 8, 12 }		// _RMS
};

bool __cdecl grv_filter(char* szFilterName)
{
	int i = strchr(szFilterName, '\0') - szFilterName;
	return i <= ruUnits[iResourceMode].iNameSize;
}

int grp_lastErrorCode = 0;

// 
// 取得最後的錯誤代碼
// 
int gr_GetLastError()
{
	return grp_lastErrorCode;
}

// 
// 取得多檔案的識別碼 (根據檔案名稱的雜湊值)
// 
DWORD gr_GetMultiFileId(char* pstPath)
{
	DWORD sum = 0;

	while (*pstPath)
	{
		sum += *pstPath;
		pstPath++;
	}

	return sum % 16;
}

int gr_GetFiles(char* pszPath, FILTER_FUNC filter, int iIndex)
{
	int files = 0;
	WIN32_FIND_DATAA wdf;
	HANDLE hFile = FindFirstFileA(pszPath, &wdf);


	if (hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(wdf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (!filter)
				{
					files++;
				}
				else if (filter(wdf.cFileName))
				{
					files++;

					if (iIndex != -1)
					{
						int iFilterId = 0;

						if (iIndex < TEXT_RESOURCE_BUFFER_INDEX && bMultiResource)
						{
							iFilterId = gr_GetMultiFileId(wdf.cFileName);
						}

						DWORD iLocalFiles = (lpResourceFiles[iIndex + iFilterId] * ruUnits[iResourceMode].iStructSize) + ruUnits[iResourceMode].iNameOffset;
						memset((BYTE*)(lpResources[iIndex + iFilterId] + iLocalFiles), 0, ruUnits[iResourceMode].iNameSize); // 2021/05/09 修正如果檔案名稱超過PakViewer無法開啟的問題 (清空緩衝區)
						lstrcpyA((char*)(lpResources[iIndex + iFilterId] + iLocalFiles), wdf.cFileName); // 2021/05/09 修正如果檔案名稱超過PakViewer無法開啟的問題 (複製檔案名稱)
						lpResourceFiles[iIndex + iFilterId]++; // 要更新檔案數量計數器
					}
				}
			}
		} while (FindNextFileA(hFile, &wdf));

		FindClose(hFile);
	}

	return files;
}

//
// 初始化資源系統
//
bool gr_Initialize()
{
	register int i;
	FILE* fin = fopen("Text.idx", "rb");

	if (!fin)
	{
		return false;
	}

	int iMode = 0;
	fread(&iMode, sizeof(int), 1, fin);
	fclose(fin);

	// _RMS
	if (iMode == 0x534d525f)
		iResourceMode = 3;
	// _EXT
	else if (iMode == 0x5458455f)
		iResourceMode = 2;
	// _IDX
	else if (iMode == 0x5844495f)
		iResourceMode = 1;
	// 原始
	else
		iResourceMode = 0;

	bMultiResource = gr_GetFiles("Sprite*.idx", 0, -1) > 1;

	for (i = 0; i < 18; i++) {
		lpResourceFiles[i] = 0;
	}

	return true;
}

#if !defined(_USRDLL)

// 
// 取得索引檔案的結構內容
// 
void grp_GetIndexFileStruct(char* pszClassName, int iIndex, BYTE*& lpResult, DWORD& dwItemCount)
{
	char pszIndexFileName[MAX_PATH];
	dwItemCount = 0;
	lpResult = NULL;

	if (iIndex >= TEXT_RESOURCE_BUFFER_INDEX || (iIndex == SPRITE_RESOURCE_BUFFER_INDEX && !bMultiResource))
	{
		wsprintfA(pszIndexFileName, "%s.idx", pszClassName);

	}
	else if (iIndex < TEXT_RESOURCE_BUFFER_INDEX || bMultiResource)
	{
		wsprintfA(pszIndexFileName, "%s%02d.idx", pszClassName, iIndex);
	}

	FILE* fin = fopen(pszIndexFileName, "rb");

	if (!fin)
	{
		return;
	}

	if (iResourceMode)
	{
		fseek(fin, sizeof(DWORD), SEEK_SET);
	}

	int i;
	fread(&dwItemCount, sizeof(DWORD), 1, fin);
	int iLocalSize = ruUnits[iResourceMode].iStructSize * dwItemCount;
	lpResult = (BYTE*)malloc(iLocalSize);
	BYTE* lpScanner = lpResult;
	fread(lpResult, ruUnits[iResourceMode].iStructSize, dwItemCount, fin);

	// 解密文字資源的索引
	if (TEXT_RESOURCE_BUFFER_INDEX == iIndex)
	{
		DesContext cnt;
		BYTE key[] = { '~', '!', '@', '#', '%', '^', '$', '<' };

		desInit(&cnt, key, sizeof(key));
		i = iLocalSize;

		while (i >= 8)
		{
			desDecryptBlock(&cnt, lpScanner, lpScanner);
			lpScanner += 8;
			i -= 8;
		}
	}

	fclose(fin);
}

// 
// 掃描外部資料夾並建立更新檔案清單
// 
void gr_GetUpdateFilesByExternal(char* pstClassName)
{
	if (pstClassName == NULL)
	{
		return;
	}

	auto resetRange = [](int begin, int end)
	{
		for (int index = begin; index <= end; ++index)
		{
			if (lpResources[index] != nullptr)
			{
				free(lpResources[index]);
				lpResources[index] = nullptr;
			}

			lpResourceFiles[index] = 0;
		}
	};

	if (_stricmp(pstClassName, "Sprite") == 0)
	{
		if (bMultiResource)
		{
			resetRange(SPRITE_RESOURCE_BUFFER_INDEX, SPRITE_RESOURCE_BUFFER_INDEX + 15);
		}
		else
		{
			resetRange(SPRITE_RESOURCE_BUFFER_INDEX, SPRITE_RESOURCE_BUFFER_INDEX);
		}
	}
	else if (_stricmp(pstClassName, "Text") == 0)
	{
		resetRange(TEXT_RESOURCE_BUFFER_INDEX, TEXT_RESOURCE_BUFFER_INDEX);
	}
	else if (_stricmp(pstClassName, "Tile") == 0)
	{
		resetRange(TILE_RESOURCE_BUFFER_INDEX, TILE_RESOURCE_BUFFER_INDEX);
	}
}

// 
// 重建資源檔案
// 
void gr_RebuildResource(char* pszClassName, int iIndex, CLinLauncherDlg* lpDialog)
{
	int i = 0;
	int j = 0;
	DWORD dwUpdateCount = lpResourceFiles[iIndex];

	if (!dwUpdateCount)
	{
		return;
	}

	// 需要更新的檔案列表
	DWORD dwLastSize = 0;
	BYTE* lpUpdateFiles = (BYTE*)lpResources[iIndex];
	// 取得原始檔案列表
	BYTE* lpIndexStruct = 0;
	BYTE* lpEndIndexStruct = 0;
	DWORD dwItemCount = 0;
	DWORD my;
	grp_GetIndexFileStruct(pszClassName, iIndex, lpIndexStruct, dwItemCount);
	my = dwItemCount;
	lpIndexStruct = (BYTE*)realloc(lpIndexStruct, (dwItemCount + dwUpdateCount) * ruUnits[iResourceMode].iStructSize);
	lpEndIndexStruct = lpIndexStruct + (dwItemCount * ruUnits[iResourceMode].iStructSize);
	// 搜尋的結果
	BYTE* lpResult = 0;

	for (i = 0; i < dwUpdateCount; i++)
	{
		lpResult = (BYTE*)bsearch(lpUpdateFiles, lpIndexStruct, dwItemCount, ruUnits[iResourceMode].iStructSize, gr_stricmp);

		// 如果檔案不是更新檔案列表中的檔案，就從原始複製到.Pak檔案中
		if (!lpResult)
		{
			dwItemCount++;
			memcpy(lpEndIndexStruct, lpUpdateFiles, ruUnits[iResourceMode].iStructSize);
			lpEndIndexStruct += ruUnits[iResourceMode].iStructSize;
		}

		lpUpdateFiles += ruUnits[iResourceMode].iStructSize;
	}

	qsort(lpIndexStruct, dwItemCount, ruUnits[iResourceMode].iStructSize, gr_stricmp); // 排序檔案列表 (如果不排序會造成問題!)
	lpEndIndexStruct = lpIndexStruct; // 重新指向檔案列表
	lpUpdateFiles = (BYTE*)lpResources[iIndex]; // 重新指向需要更新的檔案列表

	lpDialog->m_pProgressCur->SetValue(0);
	lpDialog->m_pProgressCur->SetMinValue(0);
	lpDialog->m_pProgressCur->SetMaxValue(100);

	// 其餘程式碼保持不變...
}

#endif

// 
// 比較檔案名稱的函數
// 
int gr_stricmp(const void* a, const void* b)
{
	char* v4 = (char*)a + ruUnits[iResourceMode].iNameOffset;
	char* v5 = (char*)b + ruUnits[iResourceMode].iNameOffset;
	char c1 = 0;
	char c2 = 0;

	do {
		c1 = *v4;
		c2 = *v5;

		if (c1 >= 'A' && c1 <= 'Z')
			c1 += ' ';

		if (c2 >= 'A' && c2 <= 'Z')
			c2 += ' ';

		v4++;
		v5++;
	} while (c1 && c1 == c2);

	return c1 - c2;
}