#if !defined(AFX_LOGIN_H__9445BE80_959E_43C6_9E8A_3373838242C0__INCLUDED_)
#define AFX_LOGIN_H__9445BE80_959E_43C6_9E8A_3373838242C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include <vector>
#include <windows.h>
#include <tchar.h>
#include <comdef.h>
#include <atlbase.h>
#include <olectl.h>

#define MAX_LOCAL_STRING_LEN 63
#define CONFIG_HEADER_LENGTH  25
#define LAUNCHER_CONFIG_SIGN  0x98511076CCCCBAFF
#define FILE_ENCRYPT_KEY      "PAt82IqEvNB1Gray"

// ==================== DuiLib 基礎類別定義 ====================

// CStdPtrArray
class CStdPtrArray
{
public:
    CStdPtrArray() : m_ppData(NULL), m_nCount(0), m_nAllocated(0) {}
    ~CStdPtrArray() { if (m_ppData) delete[] m_ppData; }
protected:
    void** m_ppData;
    int m_nCount;
    int m_nAllocated;
};

// CStdStringPtrMap (簡化版)
class CStdStringPtrMap
{
public:
    CStdStringPtrMap() {}
    ~CStdStringPtrMap() {}
};

// CDuiString
class CDuiString
{
public:
    CDuiString() : m_pstr(m_szBuffer) { m_szBuffer[0] = '\0'; }

    CDuiString(const TCHAR ch) : m_pstr(m_szBuffer) {
        m_szBuffer[0] = ch;
        m_szBuffer[1] = '\0';
    }

    CDuiString(const CDuiString& src) : m_pstr(m_szBuffer) {
        m_szBuffer[0] = '\0';
        Assign(src.m_pstr);
    }

    CDuiString(LPCTSTR lpsz, int nLen = -1) : m_pstr(m_szBuffer) {
        m_szBuffer[0] = '\0';
        if (lpsz != NULL) Assign(lpsz, nLen);
    }

    ~CDuiString() {
        if (m_pstr != m_szBuffer && m_pstr != NULL) delete[] m_pstr;
    }

    operator LPCTSTR() const { return m_pstr; }

    CDuiString& operator=(LPCTSTR lpsz) {
        Assign(lpsz);
        return *this;
    }

    void Assign(LPCTSTR pstr, int nLength = -1) {
        if (pstr == NULL) pstr = _T("");
        nLength = (nLength < 0) ? lstrlen(pstr) : nLength;

        if (nLength < MAX_LOCAL_STRING_LEN) {
            if (m_pstr != m_szBuffer) {
                delete[] m_pstr;
                m_pstr = m_szBuffer;
            }
        }
        else if (nLength > GetLength() || m_pstr == m_szBuffer) {
            if (m_pstr != m_szBuffer) delete[] m_pstr;
            m_pstr = new TCHAR[nLength + 1];
        }

        lstrcpyn(m_pstr, pstr, nLength + 1);
    }

    int GetLength() const { return lstrlen(m_pstr); }

    CDuiString Left(int nLength) const;
    CDuiString Mid(int iPos, int nLength = -1) const;
    CDuiString Right(int nLength) const;

    LPCTSTR GetData() const { return m_pstr; }

protected:
    LPTSTR m_pstr;
    TCHAR m_szBuffer[MAX_LOCAL_STRING_LEN + 1];
};

// TRelativePosUI
typedef struct TRelativePosUI
{
    bool bRelative;
    SIZE szParent;
    int nMoveXPercent;
    int nMoveYPercent;
    int nZoomXPercent;
    int nZoomYPercent;
} TRelativePosUI;

// 前置宣告
class CControlUI;
class CPaintManagerUI;

// TNotifyUI
typedef struct tagTNotifyUI
{
    CDuiString sType;
    CDuiString sVirtualWnd;
    CControlUI* pSender;
    DWORD dwTimestamp;
    POINT ptMouse;
    WPARAM wParam;
    LPARAM lParam;
} TNotifyUI;

// INotifyUI
class INotifyUI
{
public:
    virtual void Notify(TNotifyUI& msg) = 0;
};

// IMessageFilterUI
class IMessageFilterUI
{
public:
    virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) = 0;
};

// CWindowWnd
class CWindowWnd
{
public:
    CWindowWnd() : m_hWnd(NULL) {}
    virtual ~CWindowWnd() {}

    HWND GetHWND() const { return m_hWnd; }
    operator HWND() const { return m_hWnd; }

    bool RegisterWindowClass();
    void ResizeClient(int cx = -1, int cy = -1);

    virtual LPCTSTR GetWindowClassName() const = 0;
    virtual LPCTSTR GetSuperClassName() const { return NULL; }
    virtual UINT GetClassStyle() const { return CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; }

protected:
    HWND m_hWnd;
};

// CPaintManagerUI
class CPaintManagerUI
{
public:
    CPaintManagerUI() : m_hWndPaint(NULL), m_hDcPaint(NULL), m_hwndTooltip(NULL),
        m_bOffscreenPaint(false), m_bUsedVirtualWnd(false), m_pParentResourcePM(NULL) {
    }
    ~CPaintManagerUI() {}

    HWND GetPaintWindow() const { return m_hWndPaint; }
    static CDuiString GetInstancePath();
    static CDuiString GetCurrentPath();
    bool UseParentResource(CPaintManagerUI* pm) { m_pParentResourcePM = pm; return true; }
    bool AddNotifier(INotifyUI* pControl);
    bool RemoveNotifier(INotifyUI* pControl);

public:
    HWND m_hWndPaint;
    HDC m_hDcPaint;
    HWND m_hwndTooltip;
    SIZE m_szMinWindow;
    SIZE m_szMaxWindow;
    SIZE m_szInitWindowSize;
    bool m_bOffscreenPaint;
    bool m_bUsedVirtualWnd;
    CStdPtrArray m_aPostPaintControls;
    CStdPtrArray m_aAsyncNotify;
    CStdStringPtrMap m_mNameHash;
    CStdStringPtrMap m_mOptionGroup;
    CPaintManagerUI* m_pParentResourcePM;
    CStdStringPtrMap m_mImageHash;
    CStdStringPtrMap m_DefaultAttrHash;
    static CDuiString m_pStrResourcePath;
    static CDuiString m_pStrResourceZip;
    static CDuiString m_pStrDefaultFontName;
};

// CControlUI
class CControlUI
{
public:
    CControlUI() : m_pManager(NULL) {}
    virtual ~CControlUI() {}

    virtual CDuiString GetName() const { return m_sName; }
    virtual LPCTSTR GetClass() const { return _T("ControlUI"); }
    virtual void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true) {
        m_pManager = pManager;
    }

    virtual CDuiString GetText() const { return m_sText; }
    virtual TRelativePosUI GetRelativePos() const { return m_tRelativePos; }
    virtual CDuiString GetToolTip() const { return m_sToolTip; }

    virtual void DoPaint(HDC hDC, const RECT& rcPaint) {}
    virtual void PaintBkColor(HDC hDC) {}
    virtual void PaintBkImage(HDC hDC) {}
    virtual void PaintStatusImage(HDC hDC) {}
    virtual void PaintText(HDC hDC) {}
    virtual void PaintBorder(HDC hDC) {}

    CDuiString GetVirtualWnd() const { return m_sVirtualWnd; }

protected:
    CPaintManagerUI* m_pManager;
    CDuiString m_sVirtualWnd;
    CDuiString m_sName;
    TRelativePosUI m_tRelativePos;
    CDuiString m_sText;
    CDuiString m_sToolTip;
    CDuiString m_sUserData;
    CDuiString m_sBkImage;
    CDuiString m_sForeImage;
    RECT m_rcPaint;
};

// CLabelUI
class CLabelUI : public CControlUI
{
public:
    CLabelUI() : m_iFont(-1) {}

    CDuiString GetText() const { return m_TextValue; }

protected:
    int m_iFont;
    CDuiString m_TextValue;
};

// CProgressUI
class CProgressUI : public CLabelUI
{
public:
    CProgressUI() {}

    void SetValue(int nValue) {}
    void SetMinValue(int nMin) {}
    void SetMaxValue(int nMax) {}

protected:
    CDuiString m_sForeImage;
    CDuiString m_sForeImageModify;
};

// CButtonUI
class CButtonUI : public CLabelUI
{
public:
    CButtonUI() {}

protected:
    CDuiString m_sNormalImage;
    CDuiString m_sHotImage;
    CDuiString m_sHotForeImage;
    CDuiString m_sPushedImage;
    CDuiString m_sPushedForeImage;
    CDuiString m_sFocusedImage;
    CDuiString m_sDisabledImage;
};

// CActiveXUI
class CActiveXUI : public CControlUI, public IMessageFilterUI
{
public:
    CActiveXUI() : m_pUnk(NULL) {}

    CDuiString GetModuleName() const { return m_sModuleName; }

    virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) {
        bHandled = false;
        return 0;
    }

protected:
    CDuiString m_sModuleName;
    IOleObject* m_pUnk;
};

// ==================== 專案相關結構 ====================

// Hyperlink 結構
struct Hyperlink
{
    bool bEnable;
    TCHAR swName[16];
    TCHAR swUrl[MAX_PATH];
};

// 先檢查是否已經有定義，如果沒有才定義
#ifndef _LAUNCHER_CONFIG_DEFINED
#define _LAUNCHER_CONFIG_DEFINED

// 使用 pragma pack 確保結構對齊
#pragma pack(push, 1)

// Launcher_Config 結構 - 根據錯誤訊息修正
typedef struct _Launcher_Config
{
    ULONGLONG sign;           // 簽名
    bool configed;             // 已設定
    bool encrypted;            // 已加密
    TCHAR title[MAX_PATH];     // 標題
    TCHAR ver[32];             // 版本
    TCHAR web[MAX_PATH];       // 網站
    TCHAR patach[MAX_PATH];    // 路徑 (注意: 可能是 patch 的拼寫錯誤)
    TCHAR list[MAX_PATH];      // 列表
    bool useupdate;            // 使用更新
    TCHAR update[MAX_PATH];    // 更新路徑
    bool helper;               // 輔助工具
    BYTE key[16];              // 金鑰
    BYTE data[256];            // 資料
    
    // 超連結相關 - 使用陣列形式
    bool uselink[5];           // 使用連結
    TCHAR link_name[5][16];    // 連結名稱
    TCHAR link_url[5][MAX_PATH]; // 連結網址
    
} Launcher_Config;

#pragma pack(pop)

#endif // _LAUNCHER_CONFIG_DEFINED

// ServerInfo
struct ServerInfo
{
    TCHAR name[32];
    char ip[MAX_PATH];
    int port;
    bool encrypt;
    bool usehelper;
    unsigned char key[16];
    bool usebd;
    TCHAR bdfile[32];
    bool randkey;
    unsigned long e;
    unsigned long d;
    unsigned long n;
    BYTE fix[16];
};

// Update_Info
typedef struct
{
    TCHAR filename[MAX_PATH];
    int iIndex;
} Update_Info;

// ==================== CLinLauncherDlg ====================

class CLinLauncherDlg : public CWindowWnd, public INotifyUI
{
public:
    CLinLauncherDlg();
    ~CLinLauncherDlg();

    static bool LoadConfig();

    LPCTSTR GetWindowClassName() const;
    UINT GetClassStyle();
    void OnFinalMessage(HWND hWnd);
    void Init();
    void OnPrepare();
    void Notify(TNotifyUI& msg);
    void OnSelectServer(CControlUI* pSender);
    void StartGame();
    void GetHttpFile(const TCHAR* url, const TCHAR* filename, TCHAR* result);
    void GetServerList();
    void LoadServerList(const TCHAR* filename);
    void RebuildGameResource();
    void GetUpdateList();
    int CheckUpdateFile();
    void LoadUpdateList(const TCHAR* filename);
    bool unZipPackage(TCHAR* pszFilePath, TCHAR* pszOutputPath);
    bool UpdateFile();

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
    CPaintManagerUI m_pm;
    CLabelUI* m_pLabelCaption;
    CButtonUI* m_pBtnMin;
    CButtonUI* m_pBtnClose;
    CButtonUI* m_pBtnStart;
    CButtonUI* m_pBtnExit;
    CButtonUI* m_pBtnLinks[4];
    CButtonUI* m_pBtnServer[8];
    CProgressUI* m_pProgressCur;
    CProgressUI* m_pProgressAll;
    CProgressUI* m_pProgressState;
    CActiveXUI* m_pWeb;

    int m_nVer;
    int m_nSelectIndex;
    int m_nServerCount;
    ServerInfo si[8];
    int m_nUpdateCount;
    TCHAR m_szUpdateBase[MAX_PATH];
    Update_Info* m_vtAllFile;
    std::vector<Update_Info*> m_vtNeedUpdateFile;
};

// ==================== 輔助函數 ====================

// config_encrypt 函數宣告 - 修正參數數量
void config_encrypt(BYTE* data, int size, const BYTE* key = NULL);

// WritePrivateProfileString 的正確宣告
BOOL WINAPI WritePrivateProfileStringA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpString,
    LPCSTR lpFileName
);

BOOL WINAPI WritePrivateProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpString,
    LPCWSTR lpFileName
);

// BSTR 轉換函數
inline BSTR TCharToBSTR(const TCHAR* str)
{
    if (!str) return NULL;

#ifdef UNICODE
    return SysAllocString(str);
#else
    int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    if (len == 0) return NULL;

    BSTR bstr = SysAllocStringLen(NULL, len - 1);
    if (bstr) {
        MultiByteToWideChar(CP_ACP, 0, str, -1, bstr, len);
    }
    return bstr;
#endif
}

// 使用範例巨集，簡化 BSTR 轉換和釋放
#define SAFE_NAVIGATE(pWebBrowser, url) \
    do { \
        BSTR bstrUrl = TCharToBSTR(url); \
        if (bstrUrl) { \
            (pWebBrowser)->Navigate(bstrUrl, NULL, NULL, NULL, NULL); \
            SysFreeString(bstrUrl); \
        } \
    } while(0)

#endif // !defined(AFX_LOGIN_H__9445BE80_959E_43C6_9E8A_3373838242C0__INCLUDED_)