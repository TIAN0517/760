#include "stdafx.h"
#include "stdafx.h"
#include "ShareMemory.h"

HANDLE hshm = NULL;
SHARE_INFO* pshm_info = NULL;

SHARE_INFO* get_shm(DWORD pid, bool create)
{
	TCHAR name[MAX_PATH];
	if (pshm_info != NULL)
		return pshm_info;

	_stprintf_s(name, MAX_PATH - 1, _T("{E141066F-C3A9-4FE3-A6F1-ABFD069F1C32}_%d"), pid);

	//���ե��}
	hshm = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);

	//���}����, �i��Ы�
	if (hshm == NULL && create)
		hshm = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 8192, name);
	if (hshm == NULL)
		return NULL;
	pshm_info = (SHARE_INFO*)MapViewOfFile(hshm, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	return pshm_info;
}

void free_shm()
{
	if (pshm_info)
	{
		UnmapViewOfFile(pshm_info);
		pshm_info = NULL;
	}

	if (hshm)
	{
		CloseHandle(hshm);
		hshm = NULL;
	}
}
