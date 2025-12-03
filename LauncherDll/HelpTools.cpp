// HelpTools.cpp : DLL 幫助視窗 & 鍵盤 Hook 進入點
//

#include "stdafx.h"
#include "HelpTools.h"
#include "MainDialog.h"

#include <sys/stat.h>
#include "hook/ExceptionReport.h"
#include "hook/debug_log.h"   // HookLog()
#include "hook/gamehook.h"    // 為了 InlineSystemHook(false);

// MFC Debug new
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ============================================================
// 共用資料段（你原本就有的）
// ============================================================
#pragma data_seg(".init")
DLLCONFIG g_GameParam = { 0 };
#pragma data_seg()
#pragma comment(linker,"/section:.init,rws")

// ============================================================
// 全域變數
// ============================================================
CMainDialog* g_pHelpMainDlaog = NULL;
HWND         g_hGameWnd = NULL;
HHOOK        g_MsgHook = NULL;
extern HHOOK hhk;                     // 在別的檔案定義

SOCKADDR_IN  g_ServerAddr = { 0 };
bool         g_bIsRand = false;
bool         g_bIsDecrypt = false;
bool         g_bIsHelp = false;

// ============================================================
// CHelpToolsApp 實作（類別在 .h 宣告了，這裡只做實作）
// ============================================================

// 建構子
CHelpToolsApp::CHelpToolsApp()
{
    // 通常 DLL 的 CWinApp 這裡不用做太多
}

// 全域唯一的 app 物件
CHelpToolsApp theApp;

// 真正進來的地方
BOOL CHelpToolsApp::InitInstance()
{
    CWinApp::InitInstance();

    TRACE("正在載入 HelpTools DLL...\r\n");
    TRACE("=== DLL VERSION: BUILD-20251029-2310 with InjectPlaintext ===\r\n");
    TRACE("Compiled: %s %s\r\n", __DATE__, __TIME__);

    // 這個你原本的程式有
    hins = theApp.m_hInstance;

    // 這是你 LauncherDll 的初始化
    init();

    return TRUE;
}

// ============================================================
// 鍵盤 / 訊息 Hook
// ============================================================
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    PMSG pMsg = (PMSG)lParam;
    if (!pMsg)
        return CallNextHookEx(hhk, nCode, wParam, lParam);

    // 放開鍵
    if (pMsg->message == WM_KEYUP)
    {
        if (pMsg->wParam == VK_HOME)
        {
            if (g_pHelpMainDlaog == NULL)
            {
                // 第一次按，建立視窗
                g_hGameWnd = pMsg->hwnd;
                g_pHelpMainDlaog = new CMainDialog();
                g_pHelpMainDlaog->Create(IDD_DIALOG_MAIN);
                ::SetWindowPos(*g_pHelpMainDlaog, HWND_TOPMOST,
                    0, 0, 0, 0,
                    SWP_NOSIZE | SWP_NOMOVE);
                g_pHelpMainDlaog->ShowWindow(SW_SHOW);
            }
            else
            {
                // 之後就切換顯示
                if (g_pHelpMainDlaog->IsWindowVisible())
                    HideHelpWindow();
                else
                    ShowHelpWindow();
            }
        }
    }
    // 按下鍵，給熱鍵對話框
    else if (pMsg->message == WM_KEYDOWN)
    {
        if (pMsg->wParam - VK_F1 <= 4 &&
            g_pHelpMainDlaog &&
            g_pHelpMainDlaog->m_pHotKeyDlg &&
            IsWindow(*g_pHelpMainDlaog->m_pHotKeyDlg))
        {
            TRACE("偵測到 HotKey，轉給 m_pHotKeyDlg\r\n");
            if (g_pHelpMainDlaog->m_pHotKeyDlg->DispatachSysHotKey(pMsg->wParam - VK_F1))
            {
                pMsg->message = 0;
                return CallNextHookEx(hhk, nCode, wParam, lParam);
            }
        }
    }
    // 遊戲視窗被關閉 → 一起收
    else if (pMsg->message == WM_DESTROY && pMsg->hwnd == g_hGameWnd)
    {
        TRACE("偵測到遊戲視窗關閉，開始收掉 Help 視窗\r\n");
        if (g_pHelpMainDlaog != NULL)
        {
            CloseWindow(*g_pHelpMainDlaog);
            delete g_pHelpMainDlaog;
            g_pHelpMainDlaog = NULL;
        }
    }

    return CallNextHookEx(hhk, nCode, wParam, lParam);
}

// ============================================================
// 顯示 / 隱藏幫助視窗
// ============================================================
void ShowHelpWindow()
{
    if (g_pHelpMainDlaog)
        g_pHelpMainDlaog->ShowWindow(SW_SHOW);
}

void HideHelpWindow()
{
    if (g_pHelpMainDlaog)
        g_pHelpMainDlaog->ShowWindow(SW_HIDE);
}

// ============================================================
// 取得主程式 EXE 大小
// ============================================================
int GetMainProcessSize()
{
    char szProcessPath[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, szProcessPath, MAX_PATH - 1);

    struct _stat buf;
    int iresult = _stat(szProcessPath, &buf);
    if (iresult == 0)
        return (int)buf.st_size;
    return 0;
}

// ============================================================
// 對外：開始 Hook
// ============================================================
DLL_FUNCTION VOID StartHook(CHAR szClassName[MAX_PATH])
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HWND hGameWindow = ::FindWindowA(szClassName, NULL);
    if (hGameWindow)
    {
        DWORD threadId = GetWindowThreadProcessId(hGameWindow, NULL);
        g_MsgHook = SetWindowsHookExA(
            WH_GETMESSAGE,
            GetMsgProc,
            GetModuleHandleA("HelpTools.dll"),
            threadId
        );
    }
}

// ============================================================
// 對外：停止 Hook
// ============================================================
DLL_FUNCTION VOID StopHook()
{
    if (g_MsgHook != NULL)
    {
        UnhookWindowsHookEx(g_MsgHook);
        g_MsgHook = NULL;
    }
}

// ============================================================
// 對外：載入參數
// ============================================================
DLL_FUNCTION bool Loader(LPVOID lpParam)
{
    __try
    {
        CopyMemory(&g_GameParam, lpParam, sizeof(DLLCONFIG));
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        TRACE("load game param exception!\r\n");
    }
    return true;
}

// ============================================================
// 收尾：把 socket hook 拔掉
// ============================================================
void UnInitializeLoader()
{
    InlineSystemHook(false);
}
