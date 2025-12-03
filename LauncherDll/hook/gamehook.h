#pragma once

// ============================================================
// 先吃 PCH，才會走你在 stdafx.h 裡面那套：
// 「winsock2 要在 windows.h 前面，避免被 winsock.h 污染」
// ============================================================
#ifndef __AFXWIN_H__
#include "stdafx.h"
#endif

// ------------------------------------------------------------
// OpenSSL (你原本就有的)
// ------------------------------------------------------------
#include <openssl/bn.h>

// 老 OpenSSL + VS2015 以後相容要這兩行
#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma comment(lib, "libeay32.lib")

// ------------------------------------------------------------
// 保險：確保 winsock2 一定有進來，而且只進一次
// 有些 .cpp 沒吃 PCH 的時候就靠這裡救
// ------------------------------------------------------------
#ifndef _WINSOCKAPI_
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

// ------------------------------------------------------------
// 你專案裡本來就有的頭
// ------------------------------------------------------------
#include "ShareMemory.h"
#include "encdec.h"
#include "configenc.h"
#include "../Projectdef.h"

// ============================================================
// Hook 相關宣告區
// 目標：
//
// 1. 對外「官方版」名字：my_connect / my_send / my_recv / my_WSASend / my_CreateWindowEx
// 2. 舊碼如果還在叫 Hook_Send(...) → 這裡幫它轉掉
// 3. 不要在 .cpp 再宣告另一個同名的 Hook_Send(...)，不然就會像你前面遇到的重複定義
//
// 我這裡用 inline 包一層，型別是正確的，debug 也看得懂呼叫鏈
// ============================================================

#ifdef __cplusplus
extern "C" {
#endif

    // ❶ 「新的」命名（你現在在用的這批）
    // --------------------------------------------------------
    int  WINAPI my_connect(SOCKET s, const struct sockaddr* name, int namelen);
    int  WINAPI my_send(SOCKET s, const char FAR* buf, int len, int flags);
    int  WINAPI my_recv(SOCKET s, char* buf, int len, int flag);
    int  WINAPI my_WSASend(
        SOCKET s,
        LPWSABUF lpBuffers,
        DWORD dwBufferCount,
        LPDWORD lpNumberOfBytesSent,
        DWORD dwFlags,
        LPWSAOVERLAPPED lpOverlapped,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
    HWND WINAPI my_CreateWindowEx(
        DWORD dwExStyle,
        LPCSTR lpClassName,
        LPCSTR lpWindowName,
        DWORD dwStyle,
        int x, int y, int nWidth, int nHeight,
        HWND hWndParent,
        HMENU hMenu,
        HINSTANCE hInstance,
        LPVOID lpParam
    );

    // ❷ 「舊的」命名（相容層）
    // --------------------------------------------------------
    // 這裡我不用 #define，改成 static inline，這樣：
    // - 呼叫點的型別會是對的
    // - debug 看得到呼叫
    // - 不會被巨集展開搞到
    //
    // 舊碼：Hook_Send(...) → 會進 my_send(...)
    // --------------------------------------------------------

    static inline int WINAPI Hook_Connect(SOCKET s, const struct sockaddr* name, int namelen)
    {
        return my_connect(s, name, namelen);
    }

    static inline int WINAPI Hook_Send(SOCKET s, const char FAR* buf, int len, int flags)
    {
        return my_send(s, buf, len, flags);
    }

    static inline int WINAPI Hook_Recv(SOCKET s, char* buf, int len, int flag)
    {
        // 小防呆：有些舊程式會傳 len=0 我們就別亂解參數
        return my_recv(s, (len > 0) ? buf : nullptr, len, flag);
    }

    static inline int WINAPI Hook_WSASend(
        SOCKET s,
        LPWSABUF lpBuffers,
        DWORD dwBufferCount,
        LPDWORD lpNumberOfBytesSent,
        DWORD dwFlags,
        LPWSAOVERLAPPED lpOverlapped,
        LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
    {
        return my_WSASend(
            s,
            lpBuffers,
            dwBufferCount,
            lpNumberOfBytesSent,
            dwFlags,
            lpOverlapped,
            lpCompletionRoutine
        );
    }

    static inline HWND WINAPI Hook_CreateWindowEx(
        DWORD dwExStyle,
        LPCSTR lpClassName,
        LPCSTR lpWindowName,
        DWORD dwStyle,
        int x, int y, int nWidth, int nHeight,
        HWND hWndParent,
        HMENU hMenu,
        HINSTANCE hInstance,
        LPVOID lpParam
    )
    {
        return my_CreateWindowEx(
            dwExStyle,
            lpClassName,
            lpWindowName,
            dwStyle,
            x, y,
            nWidth, nHeight,
            hWndParent,
            hMenu,
            hInstance,
            lpParam
        );
    }

#ifdef __cplusplus
} // extern "C"
#endif

// ============================================================
// Inline / Patch 函式宣告
// （這些是你 dll 一開始就會用到的）
// ============================================================
void InlineHook(ULONG32 ulOldAddr, ULONG32 ulHookAddr, bool bIsHook = true);
void InlineHookEx(ULONG32 ulOldAddr, ULONG32 ulHookAddr, int nInlineLen);
void PatachHook(ULONG32 ulOldAddr, char* pathcode, int npathlen);

// ⚠ 重點：這裡可以留預設值
// .cpp 那邊請寫成：bool InlineSystemHook(bool bIsHook)
// 不要在 .cpp 再多做一個「沒有參數的」版本，不然 MSVC 會說你 C2572
bool InlineSystemHook(bool bIsHook = true);

// 遊戲本體要掛的另外一層（如果你有）
// 以前你有叫過 InlineGameHook(); 我也幫你留著
void InlineGameHook();

// ============================================================
// Threads / UI
// ============================================================
DWORD WINAPI PatachThread(LPVOID lParam);
DWORD WINAPI WindowThread(LPVOID lParam);

bool LoadHelp(HWND hGameWnd);
void UnLoadHelp();

// ============================================================
// DLL 進入點用的
// ============================================================
extern HINSTANCE hins;
void init();
