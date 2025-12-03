#include "stdafx.h"
#include "Lineage.h"
#include "../Compress/XUnzip.h"
#include <cstring>

TCHAR pstSelfUpdate[] = _T("/self_update");
TCHAR pstHttp[] = _T("http://");
TCHAR pstHttps[] = _T("https://");
TCHAR pstRText[] = _T("");
BOOL bNeedUpdateSalf;

Launcher_Config LauncherConfig;

#ifdef _UNICODE
#define ZIPENTRY ZIPENTRYW
#else
#define ZIPENTRY ZIPENTRY
#endif

//
// 嚙踝蕭嚙緻嚙箠嚙緹嚙磕嚙誶並將嚙踝蕭嚙褕名嚙踝蕭嚙瞇嚙踝蕭嚙踝蕭嚙緩嚙踝蕭嚙踝蕭嚙褕名
// 
// pstFileName
//					嚙緯嚙稷嚙褒迎蕭嚙緝嚙踝蕭
// dwTextSize
//					嚙緯嚙稷嚙褒迎蕭嚙緝嚙踝蕭w嚙編嚙褐塚蕭
// pstExtraName
//					嚙緯嚙踝蕭嚙瞇嚙踝蕭嚙踝蕭嚙褕名嚙磕嚙踝蕭
// 
void GetProcessFileName(TCHAR* pstFileName, DWORD dwTextSize, TCHAR* pstExtraName)
{
	TCHAR* pstExtraFileName;
	GetModuleFileName(GetModuleHandle(0), pstFileName, dwTextSize);
	pstExtraFileName = _tcsrchr(pstFileName, _T('.'));

	if (pstExtraFileName)
	{
		_tcscpy(pstExtraFileName, pstExtraName);
	}
	else
	{
		_tcscat(pstFileName, pstExtraName);
	}
}

// 
// 嚙瞇嚙箴嚙緝嚙踝蕭
// 
char* tchar_to_ansi(const TCHAR* str)
{
#ifdef _UNICODE
	static char buffer[8192];
	memset(buffer, 0 , sizeof(buffer));
	::WideCharToMultiByte(CP_ACP, 0, str, _tcslen(str), buffer, sizeof(buffer), NULL, NULL);
	return buffer;
#else
	return (char*)str;
#endif
}

// 
// 嚙踝蕭嚙踝蕭O嚙稻嚙編嚙箭
// 
bool DirectoryExists(TCHAR* dir)
{
	DWORD attr = GetFileAttributes(dir);
	return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

// 
// 嚙諍立賂蕭嚙?(嚙緣嚙締嚙踝蕭嚙?
// 
void ForceDirectories(_TCHAR* dir)
{
	if (DirectoryExists(dir))
		return;

	for (size_t i = 0; i < _tcslen(dir); i++)
	{
		if(dir[i] == _T('\\') || dir[i] == _T('/'))
		{
			_TCHAR chr = dir[i];
			dir[i] = _T('\0');
			if(!DirectoryExists(dir))
				CreateDirectory(dir, NULL);
			dir[i] = chr;
		}
	}

	CreateDirectory(dir, NULL);
}

TCHAR* ExtractFilePath(const TCHAR* filename)
{
	static TCHAR dir[MAX_PATH];
	memset(dir, 0, sizeof(dir));
	_tcscpy(dir, filename);
	for(size_t i = _tcslen(dir) - 1; i > 0; i --)
	{
		if(dir[i] == _T('\\') || dir[i] == _T('/'))
		{
			dir[i] = _T('\0');
			break;
		}
	}
	return dir;
}

// 
// 嚙踝蕭嚙踝蕭嚙緞嚙踝蕭
// 
BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	HANDLE hToken;
	LUID luid;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return FALSE;
	}

	if(!LookupPrivilegeValue(0, lpszPrivilege, &luid))
	{
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if( bEnablePrivilege )
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;
	if( !AdjustTokenPrivileges(hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES) NULL,
		(PDWORD) NULL) )
	{
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		return FALSE;
	}

	return TRUE;
}

CLinLauncherDlg::CLinLauncherDlg()
{
	m_nVer = 1000;
	m_nServerCount = 0;
	m_nSelectIndex = -1;
	m_vtNeedUpdateFile.clear();
}

CLinLauncherDlg::~CLinLauncherDlg()
{
}

LPCTSTR CLinLauncherDlg::GetWindowClassName() const { return _T("Login"); }
UINT CLinLauncherDlg::GetClassStyle() { return CS_DBLCLKS; }
void CLinLauncherDlg::OnFinalMessage(HWND hWnd) { delete this; }

void CLinLauncherDlg::Init()
{
	int MyCheckVar;

	VM_START;
	TCHAR pstFileName[MAX_PATH];
	int i;

	m_pLabelCaption = static_cast<CLabelUI*>(m_pm.FindControl(_T("label_caption")));
	m_pBtnMin = static_cast<CButtonUI*>(m_pm.FindControl(_T("btn_min")));
	m_pBtnClose = static_cast<CButtonUI*>(m_pm.FindControl(_T("btn_close")));
	m_pBtnStart = static_cast<CButtonUI*>(m_pm.FindControl(_T("btn_start")));
	m_pBtnExit = static_cast<CButtonUI*>(m_pm.FindControl(_T("btn_exit")));
	m_pProgressCur = static_cast<CProgressUI*>(m_pm.FindControl(_T("progress_cur")));
	m_pProgressAll = static_cast<CProgressUI*>(m_pm.FindControl(_T("progress_all")));
	m_pProgressState = static_cast<CProgressUI*>(m_pm.FindControl(_T("progress_status")));
	m_pBtnStart->SetEnabled(false);

	CHECK_PROTECTION(MyCheckVar, 0x29c105d7);

	GetProcessFileName(pstFileName, MAX_PATH, _T(".cfg"));
	BYTE* lpTemp = (BYTE*)&LauncherConfig;

	HANDLE hFile = CreateFile(pstFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	DWORD dwSize = 0;
	DWORD dwReadSize = 0;

	

	if (INVALID_HANDLE_VALUE == hFile)
	{
		ExitProcess(0);
		return;
	}

	while (dwSize)
	{
		if (!ReadFile(hFile, lpTemp, dwSize, &dwReadSize, 0))
		{
			ExitProcess(0);
			return;
		}

		dwSize -= dwReadSize;
		lpTemp += dwReadSize;
	}

	CloseHandle(hFile);
	lpTemp = (BYTE*)&LauncherConfig;
	config_decrypt(CFG_PRIVATE_KEY, lpTemp, sizeof(Launcher_Config));

	m_pLabelCaption->SetText(LauncherConfig.szTitle);
	
	TCHAR tmp[MAX_PATH];
	
	for (i = 0; i < 5; i++)
	{
		wsprintf(tmp, _T("btn_link%d"), i + 1);
		m_pBtnLinks[i] = static_cast<CButtonUI*>(m_pm.FindControl(tmp));
		m_pBtnLinks[i]->SetTag(0xFFFF0001);
			
		if (LauncherConfig.hyperlinks[i].bEnable)
		{
			m_pBtnLinks[i]->SetText(LauncherConfig.hyperlinks[i].swName);
			m_pBtnLinks[i]->SetUserData(LauncherConfig.hyperlinks[i].swUrl);
		}
		else
		{
			m_pBtnLinks[i]->SetUserData(_T(""));
			m_pBtnLinks[i]->SetVisible(false);
		}
	}

	for (i = 0; i < 8; i++)
	{
		wsprintf(tmp, _T("btn_server%d"), i + 1);
		m_pBtnServer[i] = static_cast<CButtonUI*>(m_pm.FindControl(tmp));
		m_pBtnServer[i]->SetTag(0xFFFF0002);
		wsprintf(tmp, _T("%d"), i);
		m_pBtnServer[i]->SetUserData(tmp);
		m_pBtnServer[i]->SetVisible(false);
	}

	m_pWeb = static_cast<CActiveXUI*>(m_pm.FindControl(_T("web")));
	
	if(m_pWeb)
	{
		if (LauncherConfig.bUseWebBowser)
		{
			IWebBrowser2* pWebBrowser = NULL;
			m_pWeb->GetControl(IID_IWebBrowser2, (void**)&pWebBrowser);

			if(pWebBrowser != NULL)
			{
				wchar_t url[MAX_PATH] = {0};
				wsprintf(url, _T("%s?t=%d"), LauncherConfig.szWebBowserUrl, time(NULL));
				pWebBrowser->Navigate(url, 0, 0, 0, 0);
				pWebBrowser->Release();
			}
		}
		else
		{
			m_pWeb->SetVisible(false);
		}
	}

	if (MyCheckVar != 0x29c105d7)
	{
		__asm
		{
			rcl edi, 11h
			push edi
			mov ebx, ecx
			shr ecx, 6
			xor ebx, ebx
			pop ebp
			add esi, 6
			add edi, 9
			mov esp, ebp
			dec ecx
			xor ebp, ebp
			_emit 0xe9
			_emit 0x40
			_emit 0x4f
			_emit 0x17
			_emit 0x55
		}
	}

	VM_END
}

// 
// 嚙踝蕭s嚙褕案迎蕭嚙箠嚙緹
//
DWORD WINAPI GetUpdateListThread(void* p)
{
	CLinLauncherDlg* pDlg = (CLinLauncherDlg*)p;
	pDlg->GetUpdateList();
	return FALSE;
}

void CLinLauncherDlg::OnPrepare()
{
	TCHAR* pstUrl;
	TCHAR pstFileName[MAX_PATH];
	m_pBtnStart->SetEnabled(false);

	// 嚙緯嚙踝蕭嚙踝蕭嚙瞌嚙稻嚙課用改蕭s
	if (LauncherConfig.bUseUpdateList)
	{
		Sleep(1000);
		_tcscpy(m_szUpdateBase, LauncherConfig.szUpdateList);
		pstUrl = _tcsrchr(m_szUpdateBase, _T('/'));
		*pstUrl = _T('\0');
		HANDLE hThread = CreateThread(NULL, 0, GetUpdateListThread, this, 0, NULL);
	}
	else
	{
		RebuildGameResource();
		Sleep(1000);
		GetServerList();
	}
}

void CLinLauncherDlg::Notify(TNotifyUI& msg)
{
	if(msg.sType == _T("windowinit"))
	{
		OnPrepare();
	}
	else if(msg.sType == _T("click"))
	{
		if (msg.pSender == m_pBtnMin)
		{
			PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
		else if(msg.pSender == m_pBtnClose || msg.pSender == m_pBtnExit)
		{
			PostQuitMessage(0);
		}
		else if(msg.pSender == m_pBtnStart)
		{
			if(m_nSelectIndex >= 0 && m_nSelectIndex < 8)
			{
				m_pBtnStart->SetEnabled(false);
				StartGame();
			}
			else
			{
				MessageBox(m_hWnd, _T("Please choose a server!"), 0, MB_ICONINFORMATION);
			}
		}
		else if(msg.pSender->GetTag() == 0xFFFF0001)
		{
			if (msg.pSender->GetUserData() != _T(""))
				ShellExecute(m_hWnd, _T("OPEN"), msg.pSender->GetUserData(), NULL, NULL, SW_SHOWNORMAL);
		}
		else if(msg.pSender->GetTag() == 0xFFFF0002)
		{
			OnSelectServer(msg.pSender);
		}
	}
}

void CLinLauncherDlg::OnSelectServer(CControlUI* pSender)
{
	int i;

	if (pSender == NULL || !pSender->IsVisible())
		return;

	for (i = 0; i < 8; i++)
	{
		if(!m_pBtnServer[i]->IsVisible())
			continue;

		m_pBtnServer[i]->SetNormalImage(_T("server_normal.png"));
	}

	CButtonUI* pBtn = static_cast<CButtonUI*>(pSender);
	pBtn->SetNormalImage(_T("server_hot.png"));
	m_nSelectIndex = _ttoi(pSender->GetUserData());
}

// 
// 嚙罷嚙締嚙瘠嚙踝蕭
// 
void CLinLauncherDlg::StartGame()
{
	// 嚙緬FS嚙誕段嚙踝蕭嚙踝蕭MKernel32.dll嚙衛剁蕭嚙緻LoadLibraryA
	BYTE bFindLoadLibraryA[443] =
	{
		0x68, 0xB7, 0x00, 0x1E, 0x00, 0xE8, 0x02, 0x00, 0x00, 0x00, 0xFF, 0xE0, 0x55, 0x8B, 0xEC, 0x83,
		0xEC, 0x0C, 0x64, 0xA1, 0x30, 0x00, 0x00, 0x00, 0x53, 0x57, 0x8B, 0x40, 0x0C, 0x8B, 0x40, 0x14,
		0x8B, 0x00, 0x8B, 0x00, 0x8B, 0x78, 0x10, 0x8B, 0x47, 0x3C, 0x8B, 0x44, 0x38, 0x78, 0x03, 0xC7,
		0x8B, 0x48, 0x24, 0x8B, 0x58, 0x1C, 0x03, 0xCF, 0x8B, 0x50, 0x20, 0x03, 0xDF, 0x8B, 0x40, 0x18,
		0x03, 0xD7, 0x89, 0x4D, 0xF4, 0x33, 0xC9, 0x89, 0x55, 0xF8, 0x89, 0x45, 0xFC, 0x85, 0xC0, 0x74,
		0x4C, 0x56, 0x8B, 0x14, 0x8A, 0x33, 0xF6, 0x03, 0xD7, 0x8A, 0x02, 0x84, 0xC0, 0x74, 0x1F, 0x69,
		0xF6, 0x01, 0x01, 0x00, 0x00, 0x0F, 0xB6, 0xC0, 0x03, 0xF0, 0xC1, 0xE0, 0x10, 0x33, 0xF0, 0x42,
		0x8A, 0x02, 0x84, 0xC0, 0x75, 0xE9, 0x81, 0xFE, 0x96, 0xE3, 0xC1, 0xEE, 0x74, 0x0B, 0x41, 0x3B,
		0x4D, 0xFC, 0x73, 0x18, 0x8B, 0x55, 0xF8, 0xEB, 0xC9, 0x8B, 0x45, 0xF4, 0x0F, 0xB7, 0x04, 0x48,
		0x8B, 0x04, 0x83, 0x03, 0xC7, 0x74, 0x05, 0xFF, 0x75, 0x08, 0xFF, 0xD0, 0x5E, 0x64, 0xA1, 0x30,
		0x00, 0x00, 0x00, 0x5F, 0x5B, 0x8B, 0x48, 0x08, 0x8B, 0x41, 0x3C, 0x8B, 0x44, 0x08, 0x28, 0x03,
		0xC1, 0x8B, 0xE5, 0x5D, 0xC2, 0x04, 0x00,
	};

	STARTUPINFO sin;
	PROCESS_INFORMATION pi;
	memset(&sin, 0, sizeof(STARTUPINFO));
	sin.cb = sizeof(STARTUPINFO);

	TCHAR pstLinbin[MAX_PATH];
	_tcscpy(pstLinbin, si[m_nSelectIndex].data.wClientName);
	_tcscat(pstLinbin, _T(".bin"));

	// ?岫隞亦恣?甈??萄遣??脩?
	DWORD dwError = 0;
	if (!CreateProcess(pstLinbin, 0, 0, 0, FALSE, CREATE_SUSPENDED, 0, 0, &sin, &pi))
	{
		dwError = GetLastError();
		// 憒??航炊蝣潭 740 (?閬?????嚗蝙??ShellExecuteEx ???
		if (dwError == ERROR_ELEVATION_REQUIRED)
		{
			SHELLEXECUTEINFO sei = { sizeof(sei) };
			sei.lpVerb = _T("runas");  // 隢?蝞∠??⊥???
			sei.lpFile = pstLinbin;
			sei.hwnd = m_hWnd;
			sei.nShow = SW_NORMAL;

			if (ShellExecuteEx(&sei))
			{
				// ??隞亦恣?甈???嚗????亙
				ExitProcess(0);
				return;
			}
			else
			{
				dwError = GetLastError();
			}
		}
	}

	// ?萄遣??脩???
	if (dwError == 0)
	{
		// 嚙踝蕭嚙諉進嚙緹嚙諸別嚙碼嚙諍立共嚙褕記嚙踝蕭嚙踝蕭
		ServerInfo* pShareInfo = get_shm(pi.dwProcessId, TRUE);

		if (pShareInfo)
		{
			pShareInfo->bIsFinish = false;
			memcpy(pShareInfo->szServerHost, si[m_nSelectIndex].szServerHost, 32);
			pShareInfo->dwServerPort = si[m_nSelectIndex].dwServerPort;
			memcpy(&pShareInfo->GameData, &si[m_nSelectIndex].data, sizeof(pShareInfo->GameData));
			pShareInfo->encrypt = si[m_nSelectIndex].encrypt ? 1 : 0;
			pShareInfo->usehelper = si[m_nSelectIndex].usehelper ? 1 : 0;
			pShareInfo->usebd = si[m_nSelectIndex].usebd ? 1 : 0;
			pShareInfo->randenc = si[m_nSelectIndex].randkey ? 1 : 0;
            pShareInfo->encrypt = si[m_nSelectIndex].encrypt ? 1 : 0;
            pShareInfo->usehelper = si[m_nSelectIndex].usehelper ? 1 : 0;
            pShareInfo->usebd = si[m_nSelectIndex].usebd ? 1 : 0;
            pShareInfo->randenc = si[m_nSelectIndex].randkey ? 1 : 0;


			// 嚙窯嚙皚嚙踝蕭嚙磊嚙緹嚙踝蕭dll
			char* pszDllName = (char*)(bFindLoadLibraryA + 183);
			TCHAR pstFileName[MAX_PATH];
			GetProcessFileName(pstFileName, MAX_PATH, _T(".dll"));
			char* pszDllPath = tchar_to_ansi(pstFileName);
			lstrcpyA(pszDllName, pszDllPath);
			LPVOID lpInjectAddress = VirtualAllocEx(pi.hProcess, NULL, sizeof(bFindLoadLibraryA), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); // 嚙踝蕭嚙踝蕭w嚙踝蕭嚙箠嚙緹嚙諉請記嚙踝蕭嚙踝蕭
			*(DWORD*)((DWORD)bFindLoadLibraryA + 1) = ((DWORD)lpInjectAddress) + 183; // 嚙踝蕭s嚙豌數迎蕭嚙瞌嚙踝蕭嚙踝蕭嚙罷

			if (lpInjectAddress != NULL)
			{
				CONTEXT cont;
				ZeroMemory(&cont, sizeof(cont));
				cont.ContextFlags = CONTEXT_INTEGER;
				GetThreadContext(pi.hThread, &cont);
				cont.Eax = (DWORD)lpInjectAddress;
				SetThreadContext(pi.hThread, &cont);
				// 嚙瞇嚙緯嚙窯嚙皚嚙踝蕭DLL嚙踝蕭嚙罵嚙篇嚙皚嚙踝蕭s嚙踝蕭嚙緣嚙踝蕭嚙瞌嚙踝蕭嚙賡中
				WriteProcessMemory(pi.hProcess, lpInjectAddress, bFindLoadLibraryA, sizeof(bFindLoadLibraryA), NULL);
			}
		}

		::ResumeThread(pi.hThread);
		
		if (pShareInfo)
		{
			while (!pShareInfo->bIsFinish)
			{
				Sleep(10);
			}

			// 嚙踝蕭嚙踝蕭嚙瑾嚙褕記嚙踝蕭嚙踝蕭
			free_shm();
		}

		// 嚙踝蕭嚙踝蕭嚙箠嚙緹嚙畿嚙踝蕭
		ExitProcess(0);
	}
	else
	{
		TCHAR stError[MAX_PATH];
		wsprintf(stError, _T("Can't execute %s! ErrorCode:%d."), pstLinbin, dwError);
		MessageBox(m_hWnd, stError, 0, MB_ICONWARNING);
	}

	m_pBtnStart->SetEnabled(TRUE);
}

// 
// 嚙磊嚙踝蕭嚙褓存嚙褕殷蕭
// 
void CLinLauncherDlg::GetHttpFile(const TCHAR* url, const TCHAR* filename, TCHAR* result)
{
	TCHAR* pstScanner;
	TCHAR temp_path[MAX_PATH];
	GetTempPath(MAX_PATH, temp_path);

	if (url)
	{
		pstScanner = (TCHAR*)url;

		while (*pstScanner)
		{
			if (*pstScanner == _T('\\'))
			{
				*pstScanner = _T('/');
			}

			pstScanner++;
		}
	}

	if (filename)
	{
		pstScanner = (TCHAR*)filename;

		while (*pstScanner)
		{
			if (*pstScanner == _T('\\'))
			{
				*pstScanner = _T('/');
			}

			pstScanner++;
		}
	}

	if (!filename)
	{
		filename = _tcsrchr(url, _T('/')) + 1;
	}
	else
	{
		// 嚙褕案名嚙誶開嚙磐嚙踝蕭HTTP嚙踝蕭HTTPS嚙踝蕭
		if (_tcsnicmp(filename, pstHttp, _tcslen(pstHttp)) == 0 || _tcsnicmp(filename, pstHttps, _tcslen(pstHttps)) == 0)
		{
			_tcscpy((TCHAR*)url, (TCHAR*)filename);
			filename = _tcsrchr(filename, _T('/')) + 1; // 嚙踝蕭嚙踝蕭仵蛈W嚙踝蕭
		}
	}
	
	// 嚙踝蕭嚙緻嚙褓存嚙褕賂蕭嚙罵
	wsprintf(result, _T("%s\\%d.tmp"), temp_path, time(NULL));
	// 嚙磋嚙踝蕭嚙褓存嚙踝蕭
	DeleteFile(result);
		
	FILE* fp = _tfopen(result, _T("wb"));
	TCHAR msg[MAX_PATH];

	if (!fp)
	{
		wsprintf(msg, _T("Error writing output file '%s'"), filename);
		m_pProgressState->SetText(msg);
		MessageBox(m_hWnd, msg, 0, MB_ICONERROR);
		ExitProcess(0);
		return;
	}

	const _TCHAR* agent = _T("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36");
	HINTERNET session = NULL, request = NULL;

	do
	{
		session = InternetOpen(agent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

		if (!session)
			break;

		request = InternetOpenUrl(session, url, NULL, 0, 0, 0);
		
		if (!request)
			break;

		DWORD read_length, status_code, status_length = sizeof(DWORD), reserved = 0;

		if (!HttpQueryInfo(request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status_code, &status_length, &reserved))
			break;

		if (status_code != HTTP_STATUS_OK)
			break;

		if (!HttpQueryInfo(request, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &status_code, &status_length, &reserved))
			break;

		int size = status_code;
		m_pProgressCur->SetValue(0);
		m_pProgressCur->SetMaxValue(100);
		wsprintf(msg, _T("Receiving file... %s (0.00%%)"), filename);
		m_pProgressState->SetText(msg);

		if (!size)
		{
			m_pProgressCur->SetValue(100);
			wsprintf(msg, _T("Failed to receive file! '%s'"), filename);
			m_pProgressState->SetText(msg);
			MessageBox(m_hWnd, msg, 0, MB_ICONERROR);
			ExitProcess(0);
			return;
		}

		DWORD tick = GetTickCount(), pos = 0;
		BYTE temp[8192];

		do
		{
			if (!InternetReadFile(request, temp, sizeof(temp), &read_length))
			{
				m_pProgressCur->SetValue(100);
				wsprintf(msg, _T("Failed to receive file! '%s'"), filename);
				m_pProgressState->SetText(msg);
				MessageBox(m_hWnd, msg, 0, MB_ICONERROR);
				ExitProcess(0);
				return;
			}

			if (read_length)
			{
				fwrite(temp, 1, read_length, fp);
			}

			pos += read_length;

			if (GetTickCount() - tick > 100)
			{
				float present = (float)(pos * 100) / size;
				wsprintf(msg, _T("Receiving file... %s (%.2f%%)"), filename, present);
				m_pProgressCur->SetValue((int)present);
				m_pProgressState->SetText(msg);
				tick = GetTickCount();
			}
		} while (read_length != 0);

		wsprintf(msg, _T("Receiving file... %s (100%%)"), filename);
		m_pProgressCur->SetValue(100);
		m_pProgressState->SetText(msg);
	} while(false);

	if (request)
		InternetCloseHandle(request);

	if (session)
		InternetCloseHandle(session);

	if (fp)
	{
		fclose(fp);
		SetFileAttributes(result, (GetFileAttributes(result) | FILE_ATTRIBUTE_HIDDEN) & ~FILE_ATTRIBUTE_READONLY);
	}
}

// 
// 嚙踝蕭嚙緻嚙踝蕭嚙璀嚙踝蕭嚙瘠嚙踝蕭
// 
void CLinLauncherDlg::GetServerList()
{
	TCHAR url[512];
	TCHAR temp_file[MAX_PATH];
	GetProcessFileName(url, 512, _T(".ini"));
	DWORD dwAttrib = GetFileAttributes(url);
		
	// 嚙踝蕭嚙緹嚙踝蕭嚙璀嚙踝蕭嚙瘠嚙踝蕭嚙踝蕭嚙罷嚙踝蕭
	if (LauncherConfig.bUseServerList)
	{
		_tcscpy(url, LauncherConfig.szServerList);
		GetHttpFile(url, 0, temp_file); // 嚙磊嚙踝蕭嚙踝蕭嚙璀嚙踝蕭嚙瘠嚙踝蕭嚙豌暫存嚙踝蕭嚙罵
		LoadServerList(temp_file);
		DeleteFile(temp_file);
	}
	// 嚙褕案如嚙瘦嚙瘩嚙踝蕭嚙踝蕭嚙踝蕭P嚙褕案非嚙踝蕭嚙盤嚙踝蕭嚙豎性殷蕭
	else if ((dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)))
	{
		LoadServerList(url);
	}
	m_pProgressState->SetText(_T("Press 'Start' to play Lineage."));
	m_pBtnStart->SetEnabled(TRUE);
}

//
// 嚙稼嚙踝蕭嚙踝蕭嚙璀嚙踝蕭嚙瘠嚙踝蕭
// 
void CLinLauncherDlg::LoadServerList(const TCHAR* filename)
{
	int i = 0;
	int j = 0;

	TCHAR pswText[32];
	TCHAR pstServerName[66];
	TCHAR pstVal[MAX_PATH];
	int id_pos = 0;

	for (i = 0; i < 8; i++)
	{
		wsprintf(pswText, _T("Server%d"), (i + 1));
		bool bFlag = GetPrivateProfileStruct(pswText, _T("Data"), &si[id_pos], sizeof(ServerInfo), filename);

		if (!bFlag)
		{
			continue;
		}

		config_decrypt(SERVER_LIST_PRIVATE_KEY, (BYTE*)&si[id_pos], sizeof(ServerInfo));

		if (GetPrivateProfileString(pswText, _T("Name"), NULL, pstServerName, 32, filename))
		{
			m_nServerCount++;
			m_pBtnServer[id_pos]->SetText(pstServerName);
			m_pBtnServer[id_pos]->SetVisible(true);
			id_pos++;
		}
		
	}
}

void CLinLauncherDlg::RebuildGameResource()
{
	m_pProgressState->SetText(_T("Checking updated resources..."));
	Sleep(500);
	
	register int i = 0;
	register int pos = 0;
	register int count = 0;
	m_pProgressAll->SetValue(0);
	m_pProgressAll->SetMinValue(0);

	gr_GetUpdateFilesByExternal("Sprite");
	gr_GetUpdateFilesByExternal("Text");
	gr_GetUpdateFilesByExternal("Tile");

	for (i = 0; i < 18; i++) {
		count += lpResourceFiles[i];
	}

	if (count)
	{
		if (bMultiResource)
		{
			m_pProgressAll->SetMaxValue(18);
			m_pProgressState->SetText(_T("Packing sprite files. Please wait."));

			for (i = SPRITE_RESOURCE_BUFFER_INDEX; i < 16; i++)
			{
				m_pProgressAll->SetValue(++pos);
				gr_RebuildResource("Sprite", i, this);
			}
				
			m_pProgressAll->SetValue(++pos);
			m_pProgressState->SetText(_T("Packing text files. Please wait."));
			gr_RebuildResource("Text", TEXT_RESOURCE_BUFFER_INDEX, this);
			m_pProgressAll->SetValue(++pos);
			m_pProgressState->SetText(_T("Packing tile files. Please wait."));
			gr_RebuildResource("Tile", TILE_RESOURCE_BUFFER_INDEX, this);
		}
		else
		{
			m_pProgressAll->SetMaxValue(3);
			m_pProgressAll->SetValue(++pos);
			m_pProgressState->SetText(_T("Packing sprite files. Please wait."));
			gr_RebuildResource("Sprite", SPRITE_RESOURCE_BUFFER_INDEX, this);
			m_pProgressAll->SetValue(++pos);
			m_pProgressState->SetText(_T("Packing text files. Please wait."));
			gr_RebuildResource("Text", TEXT_RESOURCE_BUFFER_INDEX, this);
			m_pProgressAll->SetValue(++pos);
			m_pProgressState->SetText(_T("Packing tile files. Please wait."));
			gr_RebuildResource("Tile", TILE_RESOURCE_BUFFER_INDEX, this);
		}

		Sleep(1000);
	}
	else
	{
		m_pProgressCur->SetMaxValue(10);
		m_pProgressAll->SetMaxValue(10);
		m_pProgressCur->SetValue(10);
		m_pProgressAll->SetValue(10);
	}
}

//
// 嚙罷嚙締嚙踝蕭s嚙褕殷蕭
// 
void CLinLauncherDlg::GetUpdateList()
{
	TCHAR url[512];
	TCHAR temp_file[MAX_PATH];

	if (!LauncherConfig.bUseUpdateList)
	{
		goto __UPDATE_SERVER_LIST;
	}

	wsprintf(url, _T("%s?t=%d"), LauncherConfig.szUpdateList, time(NULL));
	GetHttpFile(url, 0, temp_file);
	LoadUpdateList(temp_file);
	DeleteFile(temp_file);

	// 嚙豎要嚙踝蕭s嚙踝蕭嚙褕案數量嚙篌嚙踝蕭0嚙踝蕭嚙踝蕭嚙緘嚙磊
	if (CheckUpdateFile())
	{
		// 嚙磊嚙踝蕭嚙踝蕭s嚙褕案伐蕭嚙諸殷蕭
		if (!UpdateFile())
		{
			m_pProgressState->SetText(_T("Failed to receive ServerList file! Try later!"));
			MessageBox(NULL, _T("Failed to receive ServerList file! Try later!"), 0, MB_ICONWARNING);
			m_pBtnStart->SetEnabled(true);
			goto __UPDATE_SERVER_LIST;
		}
	}

__UPDATE_SERVER_LIST:

	RebuildGameResource();
	Sleep(1000);
	GetServerList();
}

// 
// 嚙誼查嚙箠嚙瘡嚙踝蕭s嚙踝蕭嚙褕殷蕭
// 
int CLinLauncherDlg::CheckUpdateFile()
{
	int i;
	m_pProgressCur->SetValue(0);
	m_pProgressCur->SetMaxValue(m_nUpdateCount);
	m_pProgressState->SetText(_T("Checking updated files..."));
	TCHAR dir[MAX_PATH], filename[MAX_PATH];
	m_vtNeedUpdateFile.clear();

	for (i = 0; i < m_nUpdateCount; i++)
	{
		Update_Info* UpdateInfo = &m_vtAllFile[i];
		wsprintf(filename, _T("%d"), (i + 1));
		GetPrivateProfileString(_T("update"), filename, _T(""), dir, sizeof(dir) / sizeof(TCHAR), _T("./update.local"));
			
		if (_tcslen(dir) <= 0)
		{
			m_vtNeedUpdateFile.push_back(UpdateInfo);
		}

		m_pProgressCur->SetValue(i + 1);
	}

	return m_vtNeedUpdateFile.size();
}

// 
// 嚙稼嚙踝蕭嚙踝蕭s嚙踝蕭嚙踝蕭嚙諍清嚙踝蕭
// 
// filename
//					嚙踝蕭s嚙瘠嚙踝蕭嚙踝蕭嚙褕案賂蕭嚙罵
void CLinLauncherDlg::LoadUpdateList(const TCHAR* filename)
{
	int i;
	TCHAR app[] = _T("update");
	TCHAR key[32];
	m_nUpdateCount = GetPrivateProfileInt(app, _T("count"), 0, filename);
	m_vtAllFile = new Update_Info[m_nUpdateCount];

	for (i = 0; i < m_nUpdateCount; i++)
	{
		m_vtAllFile[i].iIndex = i + 1;
		wsprintf(key, _T("%d"), m_vtAllFile[i].iIndex);
		GetPrivateProfileString(app, key, _T(""), m_vtAllFile[i].filename, MAX_PATH, filename);
	}
}


	
// 
// 嚙瞇嚙踝蕭嚙緩嚙踝蕭嚙踝蕭嚙磐嚙褕伐蕭嚙踝蕭嚙踝蕭嚙踝蕭嚙磐
// 
// pszFilePath
//						嚙踝蕭嚙磐嚙褕名嚙踝蕭
// pszOutputPath
//						嚙踝蕭嚙踝蕭嚙磐嚙編嚙踏的賂蕭嚙罵
bool CLinLauncherDlg::unZipPackage(TCHAR* pszFilePath, TCHAR* pszOutputPath)
{
	int i;
	TCHAR* pstScanner = NULL, *pstExeSimpleName = NULL, *pstDllSimpleName = NULL;
	TCHAR dest[MAX_PATH], msg[MAX_PATH], pstExeName[MAX_PATH], pstDllName[MAX_PATH];
	bool bNeedRestart = false;
	HZIP hz = OpenZip(pszFilePath, 0, ZIP_FILENAME);  
	ZIPENTRY ze;
	GetZipItem(hz, -1, &ze);

	GetModuleFileName(GetModuleHandle(0), pstExeName, MAX_PATH);
	GetProcessFileName(pstDllName, MAX_PATH, _T(".dll"));
	pstExeSimpleName = _tcsrchr(pstExeName, _T('\\')) + 1;

	int numitems = ze.index;
		
	for (i = 0; i < numitems; i++)
	{
		GetZipItem(hz, i, &ze);
		pstScanner = ze.name;
			
		while (*pstScanner)
		{
			if (*pstScanner == '/')
			{
				*pstScanner = '\\';
			}

			pstScanner++;
		}
			
		wsprintf(dest, _T("%s\\%s"), pszOutputPath, ze.name);

		// 嚙緘嚙瘦嚙諍標穿蕭嚙緯嚙踝蕭嚙踝蕭嚙踝蕭嚙踝蕭嚙踝蕭嚙褕殷蕭
		if (_tcsicmp((const TCHAR*)ze.name, pstExeSimpleName) == 0)
		{
			_tcscat(dest, _T(".new"));
			bNeedRestart = true;
		}
			
		wsprintf(msg, _T("Unzipping the %s file..."), ze.name);
		m_pProgressState->SetText(msg);
		DeleteFile(dest);
		UnzipItem(hz, i, dest, 0, ZIP_FILENAME);
		SetFileAttributes(dest, (GetFileAttributes(dest) | FILE_ATTRIBUTE_HIDDEN) & ~FILE_ATTRIBUTE_READONLY); // 嚙瞇嚙褕殷蕭嚙豎性設嚙緩嚙踝蕭嚙踝蕭嚙衛與嚙瘩嚙踝蕭讀
	}
	
	CloseZip(hz);
	return bNeedRestart;
}

// 
// 嚙罷嚙締嚙磊嚙踝蕭嚙踝蕭s嚙褕殷蕭
// 
bool CLinLauncherDlg::UpdateFile()
{
	int i;

	if (m_vtNeedUpdateFile.size() == 0)
	{
		return true;
	}

	TCHAR url[512], msg[MAX_PATH], dir[MAX_PATH], filename[MAX_PATH];
	DWORD size;
	BYTE* buffer;

	m_pProgressAll->SetValue(0);
	m_pProgressAll->SetMinValue(0);
	m_pProgressAll->SetMaxValue(m_vtNeedUpdateFile.size());

	GetCurrentDirectory(MAX_PATH, dir);

	for (i = 0; i < m_vtNeedUpdateFile.size(); i++)
	{
		Update_Info* pUpdateInfo = m_vtNeedUpdateFile[i];
		wsprintf(url, _T("%s/%s"), m_szUpdateBase, pUpdateInfo->filename);
		GetHttpFile(url, pUpdateInfo->filename, filename);

		bool bNeedRestart = unZipPackage(filename, dir);
		DeleteFile(filename);
		wsprintf(msg, _T("%d"), pUpdateInfo->iIndex);

		// 嚙踝蕭s嚙磊嚙踝蕭嚙踝蕭T
		WritePrivateProfileString(_T("update"), _T("count"), msg, _T("./update.local"));
		WritePrivateProfileString(_T("update"), msg, pUpdateInfo->filename, _T("./update.local"));

		// 嚙緘嚙瘦嚙緯嚙踝蕭嚙踝蕭嚙豎要嚙踝蕭嚙編嚙課動殷蕭
		if (bNeedRestart)
		{
			RebuildGameResource();
				
			m_pProgressState->SetText(_T("Restart is needed!"));
			Sleep(1000);
			
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			memset(&si, 0, sizeof(STARTUPINFO));
			si.cb = sizeof(STARTUPINFO);

			TCHAR cur_dir[MAX_PATH], cur_file[MAX_PATH], cmd_line[MAX_PATH * 2];
			GetCurrentDirectory(MAX_PATH, cur_dir);
			GetModuleFileName(NULL, cur_file, MAX_PATH);

			wsprintf(cmd_line, _T("%s.new %s"), cur_file, pstSelfUpdate);
			CreateProcess(NULL, cmd_line, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
			PostMessage(WM_QUIT, 0, 0);
			ExitProcess(0);
			return true;
		}

		/*
		// 嚙篌嚙踟等抬蕭20嚙踝蕭嚙褕案進嚙踝蕭拲K
		VMProtectBegin
		if(size >= 20)
			config_decrypt((unsigned char*)FILE_ENCRYPT_KEY, buffer + 4, 16);
		VMProtectEnd
		*/
	}

	return true;
}

LRESULT CLinLauncherDlg::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
	style &= ~WS_CAPTION;
	style &= ~WS_MAXIMIZEBOX;
	SetWindowLong(m_hWnd, GWL_STYLE, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	SetIcon(IDI_LOGIN);
	m_pm.Init(m_hWnd);

	CDialogBuilder builder;
	CControlUI* pRoot = builder.Create(_T("launcher.xml"), 0, NULL, &m_pm);
	m_pm.AttachDialog(pRoot);
	m_pm.AddNotifier(this);
	Init();
	return 0;
}

LRESULT CLinLauncherDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PostQuitMessage(0);
	return 0;
}

LRESULT CLinLauncherDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PostQuitMessage(0);
	bHandled = FALSE;
	return 0;
}

LRESULT CLinLauncherDlg::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if( ::IsIconic(*this) ) bHandled = FALSE;
	return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CLinLauncherDlg::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CLinLauncherDlg::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

LRESULT CLinLauncherDlg::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
	POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
	::ScreenToClient(*this, &pt);

	RECT rcClient;
	::GetClientRect(*this, &rcClient);

	RECT rcCaption = m_pm.GetCaptionRect();
	if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right && pt.y >= rcCaption.top && pt.y < rcCaption.bottom)
	{
		CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));

		if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 )
			return HTCAPTION;
	}

	return HTCLIENT;
}

LRESULT CLinLauncherDlg::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SIZE szRoundCorner = m_pm.GetRoundCorner();
	
	if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) )
	{
		CDuiRect rcWnd;
		::GetWindowRect(*this, &rcWnd);
		rcWnd.Offset(-rcWnd.left, -rcWnd.top);
		rcWnd.right++; rcWnd.bottom++;
		HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
		::SetWindowRgn(*this, hRgn, TRUE);
		::DeleteObject(hRgn);
	}

	bHandled = FALSE;
	return 0;
}

LRESULT CLinLauncherDlg::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT CLinLauncherDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	
	switch( uMsg )
	{
	case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
	case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
	case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
	case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
	case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
	case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
	case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
	case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
	case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
	default:
		bHandled = FALSE;
	}

	if( bHandled ) return lRes;
	if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

bool CLinLauncherDlg::LoadConfig()
{
	TCHAR stFileName[MAX_PATH];

	bool bFlag = false;
	DWORD dwFileSize = 0;
	DWORD dwResultBytes = 0;
	BYTE* lpBuffer = (BYTE*)&LauncherConfig;

	GetProcessFileName(stFileName, MAX_PATH, _T(".cfg"));
	HANDLE hFile = CreateFile(stFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		goto __LOAD_CONFIG_END;
	}
	
	dwFileSize = GetFileSize(hFile, 0);

	if (dwFileSize != sizeof(Launcher_Config))
	{
		goto __LOAD_CONFIG_END;
	}

	while (dwFileSize)
	{
		if (!ReadFile(hFile, lpBuffer, dwFileSize, &dwResultBytes, 0))
		{
			goto __LOAD_CONFIG_END;
		}

		dwFileSize -= dwResultBytes;
		lpBuffer += dwResultBytes;
	}

	bFlag = true;

__LOAD_CONFIG_END:

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}

	return bFlag;
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	TCHAR pstFileName[MAX_PATH];
	TCHAR pstCommand[MAX_PATH];
	
	if (!CLinLauncherDlg::LoadConfig())
	{
		return 0;
	}

	bool isSelfUpdate = false;

	if (_tcslen(lpCmdLine))
	{
		TCHAR* lpScanner = lpCmdLine;

		while ((lpScanner = _tcschr(lpScanner, _T('/'))))
		{
			if (_tcsnicmp(lpScanner, pstSelfUpdate, _tcslen(pstSelfUpdate)) == 0)
			{
				lpScanner += _tcslen(pstSelfUpdate);
				isSelfUpdate = true;
			}
			else
			{
				lpScanner++;
			}

			if (*lpScanner == _T('\0'))
				break;
		}
	}

	GetModuleFileName(hInstance, pstFileName, MAX_PATH);
	_tcscat(pstFileName, _T(".new"));
	DWORD attr = GetFileAttributes(pstFileName);

	if (!isSelfUpdate && ((attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY)))
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		memset(&si, 0, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);

		wsprintf(pstCommand, _T("%s %s"), pstFileName, pstSelfUpdate);
		CreateProcess(NULL, pstCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		ExitProcess(0);
		return 0;
	}

	// 嚙諛我改蕭s嚙緯嚙踝蕭嚙踝蕭
	if (isSelfUpdate)
	{
		GetProcessFileName(pstFileName, MAX_PATH, _T("\0"));
			
		while(!DeleteFile(pstFileName))
		{
			int err = GetLastError();
			if(err == ERROR_FILE_NOT_FOUND)
				break;

			Sleep(100);
		}

		TCHAR pstNewFileName[MAX_PATH];
		GetModuleFileName(0, pstNewFileName, MAX_PATH);
		CopyFile(pstNewFileName, pstFileName, FALSE);
		SetFileAttributes(pstFileName, GetFileAttributes(pstFileName) & ~(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY));

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		memset(&si, 0, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		CreateProcess(pstFileName, 0, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		return 0;
	}

	HANDLE hObject = CreateMutex(NULL, FALSE, _T("Login_Community"));

	// 嚙踝蕭嚙踝蕭{嚙踝蕭嚙篁嚙罷嚙論免嚙諛動改蕭s嚙緹嚙踝蕭嚙緻嚙談選蕭嚙羯
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hObject);
		return FALSE;
	}

	// 嚙瘡嚙踝蕭嚙踝蕭嚙緞嚙踝蕭嚙畿嚙賣此嚙箠嚙緹
	SetPrivilege(_T("SeDebugPrivilege"), TRUE);

	CoInitialize(NULL);

	CPaintManagerUI::SetInstance(hInstance);

	
	GetProcessFileName(pstFileName, MAX_PATH, _T(".zip"));
	CPaintManagerUI::SetResourceZip(pstFileName);

	gr_Initialize();
	CLinLauncherDlg* pDlg = new CLinLauncherDlg;

	if (pDlg == NULL)
	{
		//MessageBox(NULL, _T("嚙請造嚙踝蕭嚙踝蕭嚙踝蕭嚙踝蕭!"), NULL, MB_ICONERROR);
		return 0;
	}

	pDlg->Create(NULL, _T("Login"), UI_WNDSTYLE_FRAME, 0);
	pDlg->CenterWindow();
	pDlg->ShowWindow();
	CPaintManagerUI::MessageLoop();

	CoUninitialize();

	return 0;
}
