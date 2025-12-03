#pragma once

extern int iResourceMode;

extern bool bMultiResource;

extern int lpResourceFiles[18];

extern BYTE* lpResources[18];

#define SPRITE_RESOURCE_BUFFER_INDEX 0
#define TEXT_RESOURCE_BUFFER_INDEX 16
#define TILE_RESOURCE_BUFFER_INDEX 17

#define GR_ERROR_CREATE_TEMP_RESOURCE	1
#define GR_ERROR_OPEN_RESORCE			2
#define GR_ERROR_CREATE_TEMP_INDEX		3

//
// 初始化資源系統
//
bool gr_Initialize();

#if !defined(_USRDLL)

class CLinLauncherDlg;

// 
// 取得需要更新的檔案
// 
void gr_GetUpdateFilesByExternal(char* pstClassName);

//
// 重建資源檔案
// 
void gr_RebuildResource(char* pszClassName, int iIndex, CLinLauncherDlg* lpDialog);

#endif

// 
// 取得最後的錯誤代碼
// 
int gr_GetLastError();

struct ResourceUnit
{
	int iStructSize;
	int iNameOffset;
	int iNameSize;
	int iLengthOffset;
	int iCompressLengthOffset;
	int iModeOffset;
};