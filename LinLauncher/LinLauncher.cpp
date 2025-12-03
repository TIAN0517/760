#include "pch.h"
#include "linlauncher.h"
#include "resource.h"
#include "../Compress/XUnzip.h"
#include <cstring>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <cwctype>
#include <sstream>
#include <initializer_list>
#include <algorithm>
#include <cctype>

namespace
{
	// Local Base64 decode helpers (trimmed version of Crypto/base64.cpp) to avoid extra linkage.
	const unsigned char kBase64Lookup[256] =
	{
		/* ASCII table */
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
		64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
		64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
	};

	int LocalBase64DecodeLen(const char* src)
	{
		int nbytesdecoded;
		const unsigned char* bufin = reinterpret_cast<const unsigned char*>(src);

		while (kBase64Lookup[*(bufin++)] <= 63)
		{
		}

		int nprbytes = static_cast<int>(bufin - reinterpret_cast<const unsigned char*>(src)) - 1;
		nbytesdecoded = ((nprbytes + 3) / 4) * 3;

		return nbytesdecoded + 1;
	}

	int LocalBase64Decode(char* dst, const char* src)
	{
		int nbytesdecoded;
		const unsigned char* bufin = reinterpret_cast<const unsigned char*>(src);

		while (kBase64Lookup[*(bufin++)] <= 63)
		{
		}

		int nprbytes = static_cast<int>(bufin - reinterpret_cast<const unsigned char*>(src)) - 1;
		nbytesdecoded = ((nprbytes + 3) / 4) * 3;

		unsigned char* bufout = reinterpret_cast<unsigned char*>(dst);
		bufin = reinterpret_cast<const unsigned char*>(src);

		while (nprbytes > 4)
		{
			*(bufout++) = static_cast<unsigned char>(kBase64Lookup[*bufin] << 2 | kBase64Lookup[bufin[1]] >> 4);
			*(bufout++) = static_cast<unsigned char>(kBase64Lookup[bufin[1]] << 4 | kBase64Lookup[bufin[2]] >> 2);
			*(bufout++) = static_cast<unsigned char>(kBase64Lookup[bufin[2]] << 6 | kBase64Lookup[bufin[3]]);
			bufin += 4;
			nprbytes -= 4;
		}

		if (nprbytes > 1)
		{
			*(bufout++) = static_cast<unsigned char>(kBase64Lookup[*bufin] << 2 | kBase64Lookup[bufin[1]] >> 4);
		}
		if (nprbytes > 2)
		{
			*(bufout++) = static_cast<unsigned char>(kBase64Lookup[bufin[1]] << 4 | kBase64Lookup[bufin[2]] >> 2);
		}
		if (nprbytes > 3)
		{
			*(bufout++) = static_cast<unsigned char>(kBase64Lookup[bufin[2]] << 6 | kBase64Lookup[bufin[3]]);
		}

		*(bufout++) = '\0';
		nbytesdecoded -= (4 - nprbytes) & 3;
		return nbytesdecoded;
	}
}

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

static std::wstring GetExecutableDirectory()
{
	wchar_t modulePath[MAX_PATH] = { 0 };
	if (GetModuleFileName(NULL, modulePath, MAX_PATH) == 0)
	{
		return L"";
	}

	std::wstring directory(modulePath);
	size_t pos = directory.find_last_of(L"\\/");
	if (pos != std::wstring::npos)
	{
		directory.resize(pos + 1);
	}
	else
	{
		directory.clear();
	}

	return directory;
}

static bool FileExists(const std::wstring& path)
{
	DWORD attributes = GetFileAttributesW(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

static bool DirExists(const std::wstring& path)
{
    DWORD attributes = GetFileAttributesW(path.c_str());
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

static std::string WideToUtf8(const std::wstring& w)
{
	if (w.empty()) return std::string();
	int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), NULL, 0, NULL, NULL);
	std::string s;
	s.resize(len);
	WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], len, NULL, NULL);
	return s;
}

static void AppendLog(const std::wstring& line)
{
	// ❌ DISABLED - ui_load.log 已取消
	//std::wstring logPath = GetExecutableDirectory() + L"ui_load.log";
	//HANDLE h = CreateFileW(logPath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//if (h == INVALID_HANDLE_VALUE) return;
	//std::string utf8 = WideToUtf8(line + L"\r\n");
	//DWORD written = 0;
	//WriteFile(h, utf8.data(), (DWORD)utf8.size(), &written, NULL);
	//CloseHandle(h);
}

static bool ResolveLauncherXmlPath(std::wstring& outPath)
{
	std::wstring exeDir = GetExecutableDirectory();
	std::vector<std::wstring> candidates;
	// Prefer skin/launcher.xml if provided
	candidates.push_back(exeDir + L"skin\\launcher.xml");
	candidates.push_back(exeDir + L"..\\skin\\launcher.xml");
	candidates.push_back(exeDir + L"..\\..\\Release\\skin\\launcher.xml");
	candidates.push_back(exeDir + L"launcher.xml");
	candidates.push_back(exeDir + L"..\\launcher.xml");
	candidates.push_back(exeDir + L"..\\..\\launcher.xml");
	candidates.push_back(exeDir + L"..\\Release\\launcher.xml");
	candidates.push_back(exeDir + L"..\\..\\Release\\launcher.xml");

	DWORD currentDirLength = GetCurrentDirectory(0, NULL);
	if (currentDirLength > 1)
	{
		std::wstring currentDir(currentDirLength, L'\0');
		DWORD copied = GetCurrentDirectory(currentDirLength, &currentDir[0]);
		if (copied > 0 && copied < currentDirLength)
		{
			currentDir.resize(copied);
			if (!currentDir.empty() && currentDir.back() != L'\\')
			{
				currentDir.push_back(L'\\');
			}
			candidates.push_back(currentDir + L"launcher.xml");
		}
	}

	for (const auto& candidate : candidates)
	{
		if (FileExists(candidate))
		{
			outPath = candidate;
			return true;
		}
	}

	return false;
}

static std::wstring ResolveResourceRootPath()
{
	// Return a path whose child "skin" folder contains images
	std::wstring exeDir = GetExecutableDirectory();
	std::vector<std::wstring> roots;
	roots.push_back(exeDir);                               // ...\LinLauncher\Release\
	roots.push_back(exeDir + L"..\\");                    // ...\LinLauncher\
	roots.push_back(exeDir + L"..\\..\\Release\\");     // ...\LinLogin76\Release\
	roots.push_back(exeDir + L"..\\..\\");               // ...\LinLogin76\

	for (auto& r : roots)
	{
		if (DirExists(r + L"skin")) return r;
	}
	return exeDir; // fallback
}

// 在多個名稱中找第一個存在的控制項，提升相容舊版皮膚 XML 的容錯
static CControlUI* FindAny(CPaintManagerUI& pm, std::initializer_list<LPCTSTR> names)
{
	for (auto name : names)
	{
		if (!name || !*name) continue;
		CControlUI* ctrl = pm.FindControl(name);
		if (ctrl) return ctrl;
	}
	return nullptr;
}

static bool LoadUtf8FileToWideString(const std::wstring& filePath, std::wstring& output)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}

	std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	if (buffer.empty())
	{
		return false;
	}

	if (buffer.size() >= 3 && static_cast<unsigned char>(buffer[0]) == 0xEF &&
		static_cast<unsigned char>(buffer[1]) == 0xBB &&
		static_cast<unsigned char>(buffer[2]) == 0xBF)
	{
		buffer.erase(buffer.begin(), buffer.begin() + 3);
	}

	int wideLength = MultiByteToWideChar(CP_UTF8, 0, buffer.data(), static_cast<int>(buffer.size()), nullptr, 0);
	if (wideLength <= 0)
	{
		return false;
	}

		output.resize(wideLength);
		MultiByteToWideChar(CP_UTF8, 0, buffer.data(), static_cast<int>(buffer.size()), &output[0], wideLength);
	return true;
}

//
// 取得進程名稱並將副檔名取代為指定的副檔名
// 
// pstFileName
//					要回傳的字串
// dwTextSize
//					要回傳的字串緩存空間
// pstExtraName
//					要取代的副檔名名稱
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
// 將寬字元
// 
char* tchar_to_ansi(const TCHAR* str)
{
#ifdef _UNICODE
	static char buffer[8192];
	memset(buffer, 0, sizeof(buffer));
	::WideCharToMultiByte(CP_ACP, 0, str, _tcslen(str), buffer, sizeof(buffer), NULL, NULL);
	return buffer;
#else
	return (char*)str;
#endif
}

// 
// 資料夾是否存在
// 
bool DirectoryExists(TCHAR* dir)
{
	DWORD attr = GetFileAttributes(dir);
	return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

// 
// 建立資料夾 (含子資料夾)
// 
void ForceDirectories(_TCHAR* dir)
{
	if (DirectoryExists(dir))
		return;

	for (size_t i = 0; i < _tcslen(dir); i++)
	{
		if (dir[i] == _T('\\') || dir[i] == _T('/'))
		{
			_TCHAR chr = dir[i];
			dir[i] = _T('\0');
			if (!DirectoryExists(dir))
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
	for (size_t i = _tcslen(dir) - 1; i > 0; i--)
	{
		if (dir[i] == _T('\\') || dir[i] == _T('/'))
		{
			dir[i] = _T('\0');
			break;
		}
	}
	return dir;
}

// 
// 提升權限
// 
BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	HANDLE hToken;
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return FALSE;
	}

	if (!LookupPrivilegeValue(0, lpszPrivilege, &luid))
	{
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;
	if (!AdjustTokenPrivileges(hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
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

	// 嘗試以多組名稱尋找控制項，增加舊版 XML 的相容性
	m_pLabelCaption = static_cast<CLabelUI*>(FindAny(m_pm, { _T("label_caption"), _T("caption"), _T("title"), _T("lbl_title") }));
	m_pBtnMin = static_cast<CButtonUI*>(FindAny(m_pm, { _T("btn_min"), _T("minbtn"), _T("sys_min"), _T("minimize") }));
	m_pBtnClose = static_cast<CButtonUI*>(FindAny(m_pm, { _T("btn_close"), _T("closebtn"), _T("sys_close"), _T("close") }));
	m_pBtnStart = static_cast<CButtonUI*>(FindAny(m_pm, { _T("btn_start"), _T("btn_login"), _T("startbtn"), _T("btn_ok"), _T("ok") }));
	m_pBtnExit = static_cast<CButtonUI*>(FindAny(m_pm, { _T("btn_exit"), _T("btn_cancel"), _T("exitbtn"), _T("cancel") }));
	m_pProgressCur = static_cast<CProgressUI*>(FindAny(m_pm, { _T("progress_cur"), _T("progress1"), _T("progress_single") }));
	m_pProgressAll = static_cast<CProgressUI*>(FindAny(m_pm, { _T("progress_all"), _T("progress2"), _T("progress_total") }));
	m_pProgressState = static_cast<CProgressUI*>(FindAny(m_pm, { _T("progress_status"), _T("progress_state") }));
	// 若沒有獨立的狀態列，退而使用單次進度條顯示文字
	if (!m_pProgressState && m_pProgressCur) m_pProgressState = m_pProgressCur;
	if (m_pBtnStart) m_pBtnStart->SetEnabled(false);

	CHECK_PROTECTION(MyCheckVar, 0x29c105d7);

	// 設定檔已由 _tWinMain() 透過 LoadConfig() 載入，若不存在則使用預設值。
	// 這裡不再重複讀取或因缺失而結束行程。

	if (m_pLabelCaption) m_pLabelCaption->SetText(LauncherConfig.szTitle);

	TCHAR tmp[MAX_PATH];

	for (i = 0; i < 5; i++)
	{
		wsprintf(tmp, _T("btn_link%d"), i + 1);
		// 支援舊名 "link%d"
		m_pBtnLinks[i] = static_cast<CButtonUI*>(FindAny(m_pm, { tmp, (std::basic_string<TCHAR>(_T("link")) + std::to_wstring(i + 1)).c_str() }));
		if (!m_pBtnLinks[i]) continue;
		m_pBtnLinks[i]->SetTag(0xFFFF0001);

		if (LauncherConfig.hyperlinks[i].bEnable)
		{
			m_pBtnLinks[i]->SetText(LauncherConfig.hyperlinks[i].swName);
			m_pBtnLinks[i]->SetUserData(LauncherConfig.hyperlinks[i].swUrl);
		}
		else if (m_pBtnLinks[i])
		{
			m_pBtnLinks[i]->SetUserData(_T(""));
			m_pBtnLinks[i]->SetVisible(false);
		}
	}

	for (i = 0; i < 8; i++)
	{
		wsprintf(tmp, _T("btn_server%d"), i + 1);
		// 支援舊名 "server%d"
		m_pBtnServer[i] = static_cast<CButtonUI*>(FindAny(m_pm, { tmp, (std::basic_string<TCHAR>(_T("server")) + std::to_wstring(i + 1)).c_str() }));
		if (!m_pBtnServer[i]) continue;
		m_pBtnServer[i]->SetTag(0xFFFF0002);
		wsprintf(tmp, _T("%d"), i);
		m_pBtnServer[i]->SetUserData(tmp);
		m_pBtnServer[i]->SetVisible(false);
	}

	m_pWeb = static_cast<CActiveXUI*>(FindAny(m_pm, { _T("web"), _T("webctrl"), _T("browser") }));

	if (m_pWeb)
	{
		if (LauncherConfig.bUseWebBowser)
		{
			IWebBrowser2* pWebBrowser = NULL;
			m_pWeb->GetControl(IID_IWebBrowser2, (void**)&pWebBrowser);

			if (pWebBrowser != NULL)
			{
				TCHAR url[MAX_PATH] = { 0 };
				wsprintf(url, _T("%s?t=%d"), LauncherConfig.szWebBowserUrl, time(NULL));
				CComBSTR navigateUrl(url);
				pWebBrowser->Navigate(navigateUrl, NULL, NULL, NULL, NULL);
				pWebBrowser->Release();
			}
		}
		else if (m_pWeb)
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
// 更新檔案的進程
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
	if (m_pBtnStart) m_pBtnStart->SetEnabled(false);

	// 登錄器是否啟用更新
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
	if (msg.sType == _T("windowinit"))
	{
		OnPrepare();
	}
	else if (msg.sType == _T("click"))
	{
		if (msg.pSender == m_pBtnMin)
		{
			PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
		else if (msg.pSender == m_pBtnClose || msg.pSender == m_pBtnExit)
		{
			PostQuitMessage(0);
		}
		else if (msg.pSender == m_pBtnStart)
		{
			if (m_nSelectIndex >= 0 && m_nSelectIndex < 8)
			{
				m_pBtnStart->SetEnabled(false);
				StartGame();
			}
			else
			{
				MessageBox(m_hWnd, _T("Please choose a server!"), 0, MB_ICONINFORMATION);
			}
		}
		else if (msg.pSender->GetTag() == 0xFFFF0001)
		{
			if (msg.pSender->GetUserData() != _T(""))
				ShellExecute(m_hWnd, _T("OPEN"), msg.pSender->GetUserData(), NULL, NULL, SW_SHOWNORMAL);
		}
		else if (msg.pSender->GetTag() == 0xFFFF0002)
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
		if (!m_pBtnServer[i] || !m_pBtnServer[i]->IsVisible())
			continue;
		// 重設為預設外觀
		m_pBtnServer[i]->SetNormalImage(_T("file='server_normal.png'"));
	}

	CButtonUI* pBtn = static_cast<CButtonUI*>(pSender);
	if (pBtn) pBtn->SetNormalImage(_T("file='server_hot.png'"));
	m_nSelectIndex = _ttoi(pSender->GetUserData());
}

// 
// 開始遊戲
// 
void CLinLauncherDlg::StartGame()
{
	// 從FS區段中找尋Kernel32.dll並取得LoadLibraryA
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
	_tcscpy(pstLinbin, _T("Login.bin"));

	// 嘗試以管理員權限創建遊戲進程
	DWORD dwError = 0;
	if (!CreateProcess(pstLinbin, 0, 0, 0, FALSE, CREATE_SUSPENDED, 0, 0, &sin, &pi))
	{
		dwError = GetLastError();
		// 如果錯誤碼是 740 (需要提升權限)，使用 ShellExecuteEx 重新啟動
		if (dwError == ERROR_ELEVATION_REQUIRED)
		{
			SHELLEXECUTEINFO sei = { sizeof(sei) };
			sei.lpVerb = _T("runas");  // 請求管理員權限
			sei.lpFile = pstLinbin;
			sei.hwnd = m_hWnd;
			sei.nShow = SW_NORMAL;

			if (ShellExecuteEx(&sei))
			{
				// 成功以管理員權限啟動，關閉當前登入器
				ExitProcess(0);
				return;
			}
			else
			{
				dwError = GetLastError();
			}
		}
	}

	// 創建遊戲進程成功
	if (dwError == 0)
	{
		// 按照進程識別碼建立共享記憶體
		SHARE_INFO* pShareInfo = get_shm(pi.dwProcessId, TRUE);

		if (pShareInfo)
		{
			ZeroMemory(pShareInfo, sizeof(*pShareInfo));
			pShareInfo->bIsFinish = false;
			strncpy_s(pShareInfo->ip, si[m_nSelectIndex].ip, _TRUNCATE);
			pShareInfo->port = si[m_nSelectIndex].port;
			pShareInfo->encrypt = si[m_nSelectIndex].encrypt;
			pShareInfo->usehelper = si[m_nSelectIndex].usehelper;
			memcpy(pShareInfo->key, si[m_nSelectIndex].key, sizeof(pShareInfo->key));
			pShareInfo->usebd = si[m_nSelectIndex].usebd;
			pShareInfo->randenc = si[m_nSelectIndex].randkey;
			pShareInfo->RSA_D = si[m_nSelectIndex].d;
			pShareInfo->RSA_N = si[m_nSelectIndex].n;
			pShareInfo->read = false;
			// DEBUG: check bdfile value before copying
#ifdef _UNICODE
			TCHAR dbgMsg[256];
			_stprintf_s(dbgMsg, _T("[LinLauncher] si[%d].bdfile before copy: '%s' (len=%d)\n"), m_nSelectIndex, si[m_nSelectIndex].bdfile, _tcslen(si[m_nSelectIndex].bdfile));
			OutputDebugString(dbgMsg);
#endif
			// bdfile is now always wchar_t[32], use wcscpy_s
			wcscpy_s(pShareInfo->bdfile, 32, L"./Login.pak");
#ifdef _UNICODE
			TCHAR dbgMsg2[256];
			_stprintf_s(dbgMsg2, _T("[LinLauncher] After wcscpy_s, bdfile='%s'\n"), pShareInfo->bdfile);
			OutputDebugString(dbgMsg2);
			OutputDebugString(L"[LinLauncher] Set bdfile to ./Login.pak\n");
#endif
			memcpy(pShareInfo->szServerHost, si[m_nSelectIndex].szServerHost, 32);
			pShareInfo->dwServerPort = si[m_nSelectIndex].dwServerPort;
			memcpy(pShareInfo->data, si[m_nSelectIndex].data.data, sizeof(pShareInfo->data));

			// 注入輔助程式dll：明確指向發佈檔案名稱，避免依賴啟動器檔名
			char* pszDllName = (char*)(bFindLoadLibraryA + 183);
			TCHAR pstFileName[MAX_PATH];
			GetModuleFileName(NULL, pstFileName, MAX_PATH);
			TCHAR* pSlash = _tcsrchr(pstFileName, _T('\\'));
			if (pSlash)
			{
				*(pSlash + 1) = _T('\0');
			}
			_tcscat_s(pstFileName, MAX_PATH, _T("LauncherDll.dll"));
			char* pszDllPath = tchar_to_ansi(pstFileName);
			lstrcpyA(pszDllName, pszDllPath);
			LPVOID lpInjectAddress = VirtualAllocEx(pi.hProcess, NULL, sizeof(bFindLoadLibraryA), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); // 對指定的進程申請記憶體
			*(DWORD*)((DWORD)bFindLoadLibraryA + 1) = ((DWORD)lpInjectAddress) + 183; // 更新變數的記憶體位址

			if (lpInjectAddress != NULL)
			{
				CONTEXT cont;
				ZeroMemory(&cont, sizeof(cont));
				cont.ContextFlags = CONTEXT_INTEGER;
				GetThreadContext(pi.hThread, &cont);
#if defined(_M_IX86)
				cont.Eax = (DWORD)lpInjectAddress;
#else
				cont.Rax = (DWORD64)lpInjectAddress;
#endif
				SetThreadContext(pi.hThread, &cont);
				// 將要注入的DLL路徑寫入到新分配的記憶體中
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

			// 關閉共享記憶體
			free_shm();
		}

		// 結束進程運行
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
// 下載暫存檔案
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
		// 檔案名稱開頭有HTTP或HTTPS時
		if (_tcsnicmp(filename, pstHttp, _tcslen(pstHttp)) == 0 || _tcsnicmp(filename, pstHttps, _tcslen(pstHttps)) == 0)
		{
			_tcscpy((TCHAR*)url, (TCHAR*)filename);
			filename = _tcsrchr(filename, _T('/')) + 1; // 獲取檔案名稱
		}
	}

	// 取得暫存檔路徑
	wsprintf(result, _T("%s\\%d.tmp"), temp_path, time(NULL));
	// 刪除暫存檔
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
	} while (false);

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
// 取得伺服器列表
// 
void CLinLauncherDlg::GetServerList()
{
	TCHAR url[512];
	TCHAR temp_file[MAX_PATH];
	GetProcessFileName(url, 512, _T(".ini"));
	DWORD dwAttrib = GetFileAttributes(url);

	// 遠程伺服器列表為開啟
	if (LauncherConfig.bUseServerList)
	{
		_tcscpy(url, LauncherConfig.szServerList);
		GetHttpFile(url, 0, temp_file); // 下載伺服器列表至暫存路徑
		LoadServerList(temp_file);
		DeleteFile(temp_file);
	}
	// 檔案如果非為資料夾與檔案非為無效屬性時
	else if ((dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)))
	{
		LoadServerList(url);
	}
	if (m_pProgressState) m_pProgressState->SetText(_T("Press 'Start' to play Lineage."));
	if (m_pBtnStart) m_pBtnStart->SetEnabled(TRUE);
}

//
// 加載伺服器列表
// 
void CLinLauncherDlg::LoadServerList(const TCHAR* filename)
{
	m_nServerCount = 0;
	m_nSelectIndex = -1;
	ZeroMemory(si, sizeof(si));

	for (int idx = 0; idx < static_cast<int>(_countof(m_pBtnServer)); ++idx)
	{
		if (!m_pBtnServer[idx])
		{
			continue;
		}

		m_pBtnServer[idx]->SetVisible(false);
		m_pBtnServer[idx]->SetText(_T(""));
	}

	auto loadPackedServerList = [&](const TCHAR* path) -> bool
	{
		bool loaded = false;
		TCHAR sectionKey[32];
		TCHAR encodedValue[2048];

		for (int rawIndex = 0; rawIndex < static_cast<int>(_countof(si)) && m_nServerCount < static_cast<int>(_countof(si)); ++rawIndex)
		{
			wsprintf(sectionKey, _T("ServerData%d"), rawIndex);
			DWORD readLen = GetPrivateProfileString(_T("list"), sectionKey, _T(""), encodedValue, _countof(encodedValue), path);
			if (readLen == 0)
			{
				continue;
			}

#ifdef _UNICODE
			int ansiLen = WideCharToMultiByte(CP_ACP, 0, encodedValue, -1, nullptr, 0, nullptr, nullptr);
			if (ansiLen <= 1)
			{
				continue;
			}
			std::string encodedAnsi(static_cast<size_t>(ansiLen - 1), '\0');
			WideCharToMultiByte(CP_ACP, 0, encodedValue, -1, &encodedAnsi[0], ansiLen - 1, nullptr, nullptr);
#else
			std::string encodedAnsi(encodedValue);
#endif

			encodedAnsi.erase(std::remove_if(encodedAnsi.begin(), encodedAnsi.end(), [](unsigned char ch)
				{
					return std::isspace(ch) != 0;
				}), encodedAnsi.end());

			if (encodedAnsi.empty())
			{
				continue;
			}

			int required = LocalBase64DecodeLen(encodedAnsi.c_str());
			if (required < static_cast<int>(sizeof(Server_Info)))
			{
				continue;
			}

			std::vector<char> decoded(static_cast<size_t>(required));
			int actual = LocalBase64Decode(decoded.data(), encodedAnsi.c_str());
			if (actual < static_cast<int>(sizeof(Server_Info)))
			{
				continue;
			}

			Server_Info info = {};
			memcpy(&info, decoded.data(), sizeof(Server_Info));
			config_decrypt(SERVER_LIST_PRIVATE_KEY, reinterpret_cast<BYTE*>(&info), sizeof(Server_Info));

			if (!info.used)
			{
				continue;
			}

			si[m_nServerCount] = info;
			if (m_pBtnServer[m_nServerCount])
			{
				m_pBtnServer[m_nServerCount]->SetText(info.name);
				m_pBtnServer[m_nServerCount]->SetVisible(true);
			}

			++m_nServerCount;
			loaded = true;
		}

		return loaded;
	};

	if (loadPackedServerList(filename))
	{
		return;
	}

	for (int i = 0; i < static_cast<int>(_countof(si)) && m_nServerCount < static_cast<int>(_countof(si)); ++i)
	{
		TCHAR section[32];
		wsprintf(section, _T("Server%d"), i + 1);

		Server_Info info = {};
		if (!GetPrivateProfileStruct(section, _T("Data"), &info, sizeof(Server_Info), filename))
		{
			continue;
		}

		config_decrypt(SERVER_LIST_PRIVATE_KEY, reinterpret_cast<BYTE*>(&info), sizeof(Server_Info));

		if (!info.used)
		{
			continue;
		}

		si[m_nServerCount] = info;
		if (m_pBtnServer[m_nServerCount])
		{
			if (info.name[0] != _T('\0'))
			{
				m_pBtnServer[m_nServerCount]->SetText(info.name);
			}
			m_pBtnServer[m_nServerCount]->SetVisible(true);
		}

		++m_nServerCount;
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

	if (count > 0)
	{
		if (bMultiResource)
		{
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
// 開始更新檔案
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

	// 需要更新的檔案數量大於0的情況下
	if (CheckUpdateFile())
	{
		// 下載更新檔案失敗時
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
// 檢查可以更新的檔案
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
// 加載更新的項目清單
// 
// filename
//					更新列表的檔案路徑
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
// 將指定的壓縮檔全部解壓縮
// 
// pszFilePath
//						壓縮檔名稱
// pszOutputPath
//						解壓縮存放的路徑
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

		// 如果目標為登錄器的執行檔時
		if (_tcsicmp((const TCHAR*)ze.name, pstExeSimpleName) == 0)
		{
			_tcscat(dest, _T(".new"));
			bNeedRestart = true;
		}

		wsprintf(msg, _T("Unzipping the %s file..."), ze.name);
		m_pProgressState->SetText(msg);
		DeleteFile(dest);
		UnzipItem(hz, i, dest, 0, ZIP_FILENAME);
		SetFileAttributes(dest, (GetFileAttributes(dest) | FILE_ATTRIBUTE_HIDDEN) & ~FILE_ATTRIBUTE_READONLY); // 將檔案屬性設定為隱藏與非唯讀
	}

	CloseZip(hz);
	return bNeedRestart;
}

// 
// 開始下載更新檔案
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

		// 更新下載資訊
		WritePrivateProfileString(_T("update"), _T("count"), msg, _T("./update.local"));
		WritePrivateProfileString(_T("update"), msg, pUpdateInfo->filename, _T("./update.local"));

		// 如果登錄器需要重新啟動時
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
		// 大於等於20的檔案進行解密
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
	// 先設置視窗尺寸（XML 可能沒有正確應用）
	::SetWindowPos(m_hWnd, NULL, 0, 0, 780, 455, SWP_NOMOVE | SWP_NOZORDER);
	::SetWindowTextW(m_hWnd, L"Lingo");
	AppendLog(L"[OnCreate] set window size to 780x455 and title");

	m_pm.Init(m_hWnd);
	AppendLog(L"[OnCreate] m_pm.Init completed");

	CDialogBuilder builder;
	CControlUI* pRoot = NULL;
	std::wstring xmlContent;  // 用於存放讀取的 XML 內容
	bool xmlContentLoaded = false;
	std::wstring resourceRootUsed;

	std::wstring xmlPath;
	bool xmlPathFound = ResolveLauncherXmlPath(xmlPath);
	AppendLog(L"[OnCreate] ResolveLauncherXmlPath completed");
	{
		wchar_t curDirBuf[MAX_PATH] = { 0 };
		GetCurrentDirectoryW(MAX_PATH, curDirBuf);
		AppendLog(L"[OnCreate] current dir = " + std::wstring(curDirBuf));
		AppendLog(L"[OnCreate] xml path found = " + std::wstring(xmlPathFound ? L"true" : L"false"));
		if (xmlPathFound) AppendLog(L"[OnCreate] xml path = " + xmlPath);
	}

	auto ensureTrailingBackslash = [](std::wstring path)
	{
		if (!path.empty() && path.back() != L'\\' && path.back() != L'/')
		{
			path.push_back(L'\\');
		}
		return path;
	};

	// 優先將資源根目錄設為 XML 所在資料夾，確保相對路徑（例如 bg.png、server_*.png）能正確載入
	bool xmlUsesSkinPrefix = false;
	if (xmlPathFound)
	{
		xmlContentLoaded = LoadUtf8FileToWideString(xmlPath, xmlContent);
		AppendLog(L"[OnCreate] xml pre-load result = " + std::wstring(xmlContentLoaded ? L"success" : L"failed"));

		std::wstring xmlDir = xmlPath;
		size_t pos = xmlDir.find_last_of(L"\\/");
		if (pos != std::wstring::npos) xmlDir.resize(pos + 1);

		std::wstring resourceRoot = xmlDir;
		if (xmlContentLoaded)
		{
			std::wstring xmlLower = xmlContent;
			std::transform(xmlLower.begin(), xmlLower.end(), xmlLower.begin(), [](wchar_t ch) { return static_cast<wchar_t>(towlower(ch)); });
			if (xmlLower.find(L"skin/") != std::wstring::npos || xmlLower.find(L"skin\\") != std::wstring::npos)
			{
				xmlUsesSkinPrefix = true;
			}
		}

		if (xmlUsesSkinPrefix)
		{
			std::wstring normalized = xmlDir;
			std::replace(normalized.begin(), normalized.end(), L'/', L'\\');
			while (!normalized.empty() && normalized.back() == L'\\') normalized.pop_back();
			if (!normalized.empty())
			{
				size_t parentPos = normalized.rfind(L'\\');
				if (parentPos != std::wstring::npos)
				{
					resourceRoot = normalized.substr(0, parentPos + 1);
				}
			}
			AppendLog(L"[OnCreate] xml references skin/* paths, using parent dir = " + resourceRoot);
		}
		else
		{
			AppendLog(L"[OnCreate] xml uses local asset paths, resource root = " + resourceRoot);
		}

		resourceRoot = ensureTrailingBackslash(resourceRoot);
		resourceRootUsed = resourceRoot;
		AppendLog(L"[OnCreate] set resource root to = " + resourceRootUsed);
		m_pm.SetResourcePath(resourceRootUsed.c_str());
		AppendLog(L"[OnCreate] SetResourcePath completed");
	}
	else
	{
		// 找不到 XML 時才退回以可偵測到 skin 目錄的路徑
		std::wstring resourceRoot = ensureTrailingBackslash(ResolveResourceRootPath());
		resourceRootUsed = resourceRoot;
		AppendLog(L"[OnCreate] resource root (fallback) = " + resourceRootUsed);
		m_pm.SetResourcePath(resourceRootUsed.c_str());
		AppendLog(L"[OnCreate] SetResourcePath (fallback) completed");
	}
	
	AppendLog(L"[OnCreate] about to attempt XML parse");
	
	// 先試圖用內存 XML 解析（帶明確的 type="xml" 標記）
	bool parsed = false;

	if (xmlContentLoaded)
	{
		AppendLog(L"[OnCreate] attempting in-memory parse with type=xml");
		const wchar_t* xmlText = xmlContent.c_str();
		// Skip leading whitespace
		while (*xmlText && iswspace(*xmlText))
		{
			++xmlText;
		}
		
		if (*xmlText == L'<')
		{
			pRoot = builder.Create(xmlText, _T("xml"), NULL, &m_pm);
			AppendLog(L"[OnCreate] after in-memory parse, pRoot = " + std::wstring(pRoot ? L"valid" : L"NULL"));
			if (pRoot) parsed = true;
		}
	}
	else if (xmlPathFound)
	{
		AppendLog(L"[OnCreate] xml pre-load failed, will attempt file-based parse");
	}
	
	// 如果內存解析失敗，嘗試用文件路徑
	if (!parsed && xmlPathFound)
	{
		AppendLog(L"[OnCreate] in-memory parse failed, attempting file path with type=xml");
		pRoot = builder.Create(xmlPath.c_str(), _T("xml"), NULL, &m_pm);
		AppendLog(L"[OnCreate] after file path parse with type=xml, pRoot = " + std::wstring(pRoot ? L"valid" : L"NULL"));
		if (pRoot) parsed = true;
	}

	if (pRoot == NULL)
	{
		std::wstring errorMessage;
		if (!xmlPathFound)
		{
			errorMessage = L"無法載入 UI 資源!\n未找到 launcher.xml。請確認檔案位於執行檔同一目錄、上層目錄或 Release 目錄。";
		}
		else
		{
			errorMessage = L"無法載入 UI 資源!\n載入失敗的路徑:\n";
			errorMessage += xmlPath;
		}
		// 收集 DuiLib 最後錯誤
		TCHAR errMsg[512] = { 0 };
		TCHAR errLoc[512] = { 0 };
		builder.GetLastErrorMessage(errMsg, 511);
		builder.GetLastErrorLocation(errLoc, 511);
		AppendLog(L"[OnCreate] builder last error: " + std::wstring(errMsg));
		AppendLog(L"[OnCreate] builder error location: " + std::wstring(errLoc));
		MessageBoxW(NULL, errorMessage.c_str(), L"錯誤", MB_OK | MB_ICONERROR);
		return -1;
	}

	m_pm.AttachDialog(pRoot);
	m_pm.AddNotifier(this);
	AppendLog(L"[OnCreate] AttachDialog and AddNotifier completed");

	// 診斷：強制設定背景和顏色
	{
		CControlUI* pRootCtrl = m_pm.GetRoot();
		AppendLog(L"[OnCreate] pRootCtrl = " + std::wstring(pRootCtrl ? L"valid" : L"NULL"));
		if (pRootCtrl)
		{
			// 設置背景顏色為深灰（後備）
			pRootCtrl->SetBkColor(0xFF333333);
			AppendLog(L"[OnCreate] set bkcolor to #FF333333");
			
			std::wstring diagImageSpec = xmlUsesSkinPrefix ? L"file='skin/bg.png'" : L"file='bg.png'";
			AppendLog(L"[OnCreate] diagnostic image spec = " + diagImageSpec);
			std::wstring diagRelPath;
			if (!resourceRootUsed.empty())
			{
				size_t firstQuote = diagImageSpec.find(L'\'');
				size_t secondQuote = diagImageSpec.find(L'\'', firstQuote + 1);
				if (firstQuote != std::wstring::npos && secondQuote != std::wstring::npos && secondQuote > firstQuote + 1)
				{
					diagRelPath = diagImageSpec.substr(firstQuote + 1, secondQuote - firstQuote - 1);
					std::wstring probePath = resourceRootUsed + diagRelPath;
					AppendLog(L"[OnCreate] probing image path = " + probePath + L" exists=" + (FileExists(probePath) ? L"yes" : L"no"));
				}
			}
			// 先嘗試將圖片加入資源快取，方便追蹤是否載入成功
			bool addImageSucceeded = false;
			if (!diagRelPath.empty())
			{
				addImageSucceeded = (m_pm.AddImage(diagRelPath.c_str()) != nullptr);
				AppendLog(L"[OnCreate] AddImage(" + diagRelPath + L") " + (addImageSucceeded ? L"succeeded" : L"failed"));
			}
			else
			{
				addImageSucceeded = (m_pm.AddImage(diagImageSpec.c_str()) != nullptr);
				AppendLog(L"[OnCreate] AddImage(" + diagImageSpec + L") " + (addImageSucceeded ? L"succeeded" : L"failed"));
			}
			
			// 設置背景圖像（使用 DuiLib 格式）
			pRootCtrl->SetBkImage(diagImageSpec.c_str());
			AppendLog(L"[OnCreate] set bkimage to " + diagImageSpec);
			
			// 強制刷新
			pRootCtrl->Invalidate();
			AppendLog(L"[OnCreate] called Invalidate()");
		}
	}
	AppendLog(L"[OnCreate] about to call Init()");
	
	try
	{
		Init();
		AppendLog(L"[OnCreate] Init() completed successfully");
	}
	catch (const std::exception& ex)
	{
		AppendLog(L"[OnCreate] Init() threw exception");
		MessageBoxA(NULL, ex.what(), "Init() Exception", MB_OK | MB_ICONERROR);
	}
	catch (...)
	{
		AppendLog(L"[OnCreate] Init() threw unknown exception");
		MessageBoxW(NULL, L"Init() 發生未知異常", L"異常", MB_OK | MB_ICONERROR);
	}
	
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
	if (::IsIconic(*this)) bHandled = FALSE;
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

		if (pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0)
			return HTCAPTION;
	}

	return HTCLIENT;
}

LRESULT CLinLauncherDlg::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SIZE szRoundCorner = m_pm.GetRoundCorner();

	if (!::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0))
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

	switch (uMsg)
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

	if (bHandled) return lRes;
	if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
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

	// 如果配置檔不存在,使用預設值
	if (!CLinLauncherDlg::LoadConfig())
	{
		// 設定預設配置
		_tcscpy(LauncherConfig.szTitle, _T("LinLauncher"));
		LauncherConfig.bUseWebBowser = false;
		LauncherConfig.szWebBowserUrl[0] = _T('\0');
		LauncherConfig.bUseServerList = false;
		LauncherConfig.szServerList[0] = _T('\0');
		LauncherConfig.bUseUpdateList = false;
		LauncherConfig.szUpdateList[0] = _T('\0');
		for (int i = 0; i < 5; i++)
		{
			LauncherConfig.hyperlinks[i].bEnable = false;
		}
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

	// 自我更新登錄器
	if (isSelfUpdate)
	{
		GetProcessFileName(pstFileName, MAX_PATH, _T("\0"));

		while (!DeleteFile(pstFileName))
		{
			int err = GetLastError();
			if (err == ERROR_FILE_NOT_FOUND)
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

	// 防止程式多開避免自動更新程式發生錯誤
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hObject);
		return FALSE;
	}

	// 以除錯權限運行此進程
	SetPrivilege(_T("SeDebugPrivilege"), TRUE);

	CoInitialize(NULL);

	HINSTANCE hModuleInst = GetModuleHandle(NULL);  // 獲取當前 EXE 的句柄
	CPaintManagerUI::SetInstance(hModuleInst);
	CPaintManagerUI::SetResourceDll(hModuleInst);  // 設置資源 DLL 實例為當前 EXE


	GetProcessFileName(pstFileName, MAX_PATH, _T(".zip"));
	if (FileExists(pstFileName))
	{
		AppendLog(L"[WinMain] detected resource zip: " + std::wstring(pstFileName));
		CPaintManagerUI::SetResourceZip(pstFileName);
	}
	else
	{
		AppendLog(L"[WinMain] resource zip not found, using loose files");
		CPaintManagerUI::SetResourceZip(_T(""));
	}

	gr_Initialize();
	CLinLauncherDlg* pDlg = new CLinLauncherDlg;

	if (pDlg == NULL)
	{
		MessageBox(NULL, _T("記憶體不足,無法建立視窗!"), _T("錯誤"), MB_OK | MB_ICONERROR);
		return 0;
	}

	HWND hWnd = pDlg->Create(NULL, _T("Login"), UI_WNDSTYLE_FRAME, 0);
	if (hWnd == NULL)
	{
		MessageBox(NULL, _T("無法建立視窗!可能缺少 UI 資源。"), _T("錯誤"), MB_OK | MB_ICONERROR);
		delete pDlg;
		return 0;
	}
	
	pDlg->CenterWindow();
	pDlg->ShowWindow();
	CPaintManagerUI::MessageLoop();

	CoUninitialize();

	return 0;
}
