// ============================================================================
// Lin LauncherDll (patched no-UI-login-block)
// 改版日期: 2025-11-02
// 重排 + 整理 + 把「launcher=0 就不要擋登入包」邏輯保留下來
// 主要變動：
// 1. 把共用的 log / UI 擷取 / S_KEY 抓取 / 判斷是不是登入包 拆成小函式
// 2. my_send() 邏輯變清楚：先判斷是不是登入 → 需要的話抓 UI → launcher=1 才會真的擋
// 3. recv() 的 S_KEY 累積、fallback、C_ServerVersion 回送流程保留
// 4. BD 讀取流程保留，log 更集中
// 5. 保留所有原本 inline hook, RSA, BD hook 的行為
// ============================================================================

#include "stdafx.h"
#include "gamehook.h"
#include "../HelpTools.h"
#include "../MainDialog.h"
#include "../../common/zlib/zlib.h"
#include "opcodes.h"
#include "Cipher.h"

#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <exception>
#include <string>
#include <vector>

#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "version.lib")

#pragma warning(disable:4309)

using namespace gameglobalparam;

class CMainDialog;
LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);

extern CMainDialog* g_pHelpMainDlaog;
extern HANDLE       g_hWndThread;
extern HHOOK        hhk;
extern HINSTANCE    hins;

// ============================================================================
// 全域狀態
// ============================================================================
static bool      g_legacyUiHooksEnabled = false;  // 舊版 UI (17.x 以前) 是否要掛老的 username/password hook
static bool      g_versionStateInitialized = false;
static bool      g_cipherInitialized = false;
static uint32_t  g_cipherSeed = 0;
static Cipher    g_cipherSend;
static SOCKET    g_gameSocket = INVALID_SOCKET;

static bool      g_droppedFirstLogin = false;

// 啟動器要求的「要加密」旗標
static bool      g_respectLauncherEncrypt = false; // launcher 說要 → 我們才會真的開「runtime encode」
static bool      g_runtimeEncryptOn = false; // 真正送封包時要不要走 Cipher
static bool      g_rsaInited = false; // RSA 是否已跑過
static bool      g_forceCipherSniff = false; // launcher=0 也要抓 S_KEY，但是「只抓不加密」
static bool      g_inEncryptedSend = false; // 遞迴保護
static bool      g_dropLoginIfNoUi = true;  // ★ launcher=1 + 沒抓到 UI → 不送登入包
static bool      g_forceRuntimeCipher = false; // 新版客戶端但 launcher 未開加密 → 強制啟用
static DWORD     g_detectedVersionMajor = 0;
static DWORD     g_detectedVersionMinor = 0;
static DWORD     g_detectedVersionBuild = 0;
static DWORD     g_detectedVersionRevision = 0;

// S_KEY 封包緩衝 (因為有時候 15 bytes 會被 TCP 分成兩包)
static char      g_handshakeBuffer[32] = { 0 };
static int       g_handshakeBufferLen = 0;
static const int HANDSHAKE_EXPECTED_LEN = 15;

// 最近一次擷取到的帳號密碼 (UI / 鍵盤)
static std::string g_lastCapturedAccount;
static std::string g_lastCapturedPassword;

// 鍵盤擷取暫存
static std::string g_keyboardBuffer;
static bool        g_isCapturingInput = false;
static int         g_tabPressCount = 0;

// ---------------------------------------------------------------------
// BD 檔案常數
// ---------------------------------------------------------------------
#define BD_FILE_SIGNATURE_BYTES "BDk1"
#define BD_FILE_SIGNATURE_SIZE  4
#define BD_HEADER_SIZE          40  // 4(size) + 4(signature) + 32(key)
#define BD_LEGACY_HEADER_SIZE   20  // 4(size) + 16(key)

// ---------------------------------------------------------------------
// 全域 hook 變數
// ---------------------------------------------------------------------
HHOOK     hhk = NULL;
HINSTANCE hins = NULL;
HWND      g_hGameMainWnd = NULL;
HANDLE    g_hWndThread = INVALID_HANDLE_VALUE;
extern CMainDialog* g_pHelpMainDlaog;

// 啟動器共享資料
SHARE_INFO ShareInfo;

// BD buffer
BYTE* buffer = NULL;
DWORD  buffer_len = 0;
static bool g_bdHooksApplied = false;

// 遊戲視窗亂數標題
char szTitle[32];

// RSA
int      _seed = 0;
int      _xorByte = 0;
BN_CTX* bn_ctx = nullptr;
BIGNUM* d = nullptr;
BIGNUM* n = nullptr;

// ---------------------------------------------------------------------
// 前置宣告
// ---------------------------------------------------------------------
static void HookLog(const char* fmt, ...);
static void ResetCipherState();
static void DetectGameVersion();
static void CaptureLoginInputs();
static void InstallKeyboardHook();
static void ApplyOpcodeTranslation(unsigned char* data, size_t length, const char* contextTag);
static bool IsLoginLikePacket(bool hasLengthPrefix, size_t packetSize, unsigned char originalOpcode, unsigned char translatedOpcode);
static void DumpLoginPacket(const unsigned char* data, size_t len,
    bool hasLengthPrefix, int opcodeOffset,
    unsigned char originalOpcode, unsigned char translatedOpcode,
    unsigned char probeTranslated,
    int shapeLikeDll, int s_loginDumpIndex);
static BYTE* GetFileBuffer();
static bool LoadHelp(HWND hGameWnd);
static void UnLoadHelp();

// ---------------------------------------------------------------------
// 工具：寬字串 → UTF-8
// ---------------------------------------------------------------------
static std::string WideToUtf8(const std::wstring& input)
{
    if (input.empty())
        return {};

    int required = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (required <= 1)
        return {};

    std::string output(static_cast<size_t>(required - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, const_cast<char*>(output.data()), required, nullptr, nullptr);
    return output;
}

// ============================================================================
// UI 擷取：列舉子視窗
// ============================================================================
struct ChildCaptureContext
{
    int         editIndex = 0;
    std::wstring account;
    std::wstring password;
};

struct LoginCaptureContext
{
    DWORD       processId = 0;
    std::wstring account;
    std::wstring password;
};

static BOOL CALLBACK EnumLoginChildProc(HWND hwnd, LPARAM lParam)
{
    auto* ctx = reinterpret_cast<ChildCaptureContext*>(lParam);
    if (!ctx) return TRUE;

    wchar_t className[128] = { 0 };
    if (GetClassNameW(hwnd, className, _countof(className)) == 0)
        return TRUE;

    HookLog("[UI Debug] Found child window: class='%S'", className);

    bool isInputControl =
        (wcscmp(className, L"Edit") == 0) ||
        (wcsstr(className, L"Edit") != nullptr) ||
        (wcsstr(className, L"edit") != nullptr) ||
        (wcsstr(className, L"Input") != nullptr) ||
        (wcsstr(className, L"Text") != nullptr);

    if (!isInputControl)
        return TRUE;

    wchar_t textBuffer[256] = { 0 };
    int len = GetWindowTextW(hwnd, textBuffer, _countof(textBuffer));
    if (len < 0)
        return TRUE;

    std::wstring content(textBuffer, static_cast<size_t>(len));
    if (len > 0)
        HookLog("[UI Debug] Edit control #%d has text (len=%d): '%S'", ctx->editIndex, len, content.c_str());

    if (ctx->editIndex == 0)
        ctx->account = content;
    else if (ctx->editIndex == 1)
        ctx->password = content;

    ctx->editIndex++;
    if (!ctx->account.empty() && !ctx->password.empty())
        return FALSE;

    return TRUE;
}

static BOOL CALLBACK EnumLoginWindowProc(HWND hwnd, LPARAM lParam)
{
    auto* parentCtx = reinterpret_cast<LoginCaptureContext*>(lParam);
    if (!parentCtx) return TRUE;

    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid != parentCtx->processId)
        return TRUE;

    wchar_t windowTitle[256] = { 0 };
    GetWindowTextW(hwnd, windowTitle, _countof(windowTitle));
    if (wcslen(windowTitle) > 0)
        HookLog("[UI Debug] Checking window: '%S'", windowTitle);

    ChildCaptureContext childCtx{};
    EnumChildWindows(hwnd, EnumLoginChildProc, reinterpret_cast<LPARAM>(&childCtx));

    if (!childCtx.account.empty() || !childCtx.password.empty())
    {
        parentCtx->account = childCtx.account;
        parentCtx->password = childCtx.password;
        return FALSE;
    }

    return TRUE;
}

// ============================================================================
// 鍵盤 Hook (給奇怪的自繪登入視窗用)
// ============================================================================
static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_CHAR))
    {
        KBDLLHOOKSTRUCT* pKbd = (KBDLLHOOKSTRUCT*)lParam;

        HWND foreground = GetForegroundWindow();
        if (foreground)
        {
            DWORD fgPid = 0;
            GetWindowThreadProcessId(foreground, &fgPid);
            if (fgPid == GetCurrentProcessId())
            {
                DWORD vkCode = pKbd->vkCode;

                // Tab → 視為輸入帳號結束，改抓密碼
                if (vkCode == VK_TAB)
                {
                    if (!g_keyboardBuffer.empty() && g_tabPressCount == 0)
                    {
                        g_lastCapturedAccount = g_keyboardBuffer;
                        HookLog("[Keyboard] Account captured (Tab): '%s'", g_keyboardBuffer.c_str());
                        g_keyboardBuffer.clear();
                        g_tabPressCount = 1;
                    }
                }
                // Enter → 結束輸入
                else if (vkCode == VK_RETURN)
                {
                    if (!g_keyboardBuffer.empty())
                    {
                        if (!g_lastCapturedAccount.empty() || g_tabPressCount == 1)
                        {
                            g_lastCapturedPassword = g_keyboardBuffer;
                            HookLog("[Keyboard] Password captured (Enter): '%s'", g_keyboardBuffer.c_str());
                        }
                        else
                        {
                            g_lastCapturedAccount = g_keyboardBuffer;
                            g_lastCapturedPassword = g_keyboardBuffer;
                            HookLog("[Keyboard] Quick login (Enter): account=password='%s'", g_keyboardBuffer.c_str());
                        }
                        g_keyboardBuffer.clear();
                    }
                    g_tabPressCount = 0;
                }
                // Backspace
                else if (vkCode == VK_BACK)
                {
                    if (!g_keyboardBuffer.empty())
                        g_keyboardBuffer.pop_back();
                }
                // 0-9 / A-Z
                else if (vkCode >= 0x30 && vkCode <= 0x5A)
                {
                    char ch = (char)vkCode;
                    if (!(GetKeyState(VK_SHIFT) & 0x8000))
                    {
                        if (ch >= 'A' && ch <= 'Z')
                            ch = ch - 'A' + 'a';
                    }
                    g_keyboardBuffer += ch;
                }
            }
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static HHOOK g_keyboardHook = NULL;

static void InstallKeyboardHook()
{
    if (!g_keyboardHook)
    {
        g_keyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardHookProc, hins, 0);
        if (g_keyboardHook)
            HookLog("[Keyboard] Low-level keyboard hook installed successfully");
        else
            HookLog("[Keyboard] Failed to install keyboard hook, error=%lu", GetLastError());
    }
}

// ============================================================================
// 取得登入 UI
// ============================================================================
static void CaptureLoginInputs()
{
    HookLog("[UI Capture] Starting credential capture, PID=%lu", GetCurrentProcessId());

    // 1. 優先用鍵盤勾到的
    if (!g_lastCapturedAccount.empty() && !g_lastCapturedPassword.empty())
    {
        HookLog("[UI Capture] Using keyboard-captured credentials: account='%s' password='%s'",
            g_lastCapturedAccount.c_str(), g_lastCapturedPassword.c_str());
        return;
    }

    // 2. 實際去掃視窗
    LoginCaptureContext ctx{};
    ctx.processId = GetCurrentProcessId();
    EnumWindows(EnumLoginWindowProc, reinterpret_cast<LPARAM>(&ctx));

    std::string account = WideToUtf8(ctx.account);
    std::string password = WideToUtf8(ctx.password);

    HookLog("[UI Capture] Window enum results: account='%s' (len=%d), password='%s' (len=%d)",
        account.c_str(), (int)account.length(),
        password.c_str(), (int)password.length());

    if (!account.empty())
        g_lastCapturedAccount = account;
    if (!password.empty())
        g_lastCapturedPassword = password;

    if (!account.empty() || !password.empty())
    {
        HookLog("[LauncherDll] UI capture -> account='%s' password='%s'",
            account.c_str(), password.c_str());
    }
    else
    {
        HookLog("[UI Capture] WARNING: No credentials captured - check window structure!");
        if (!g_lastCapturedAccount.empty() || !g_lastCapturedPassword.empty())
        {
            HookLog("[UI Capture] Fallback keyboard data: account='%s' password='%s'",
                g_lastCapturedAccount.c_str(), g_lastCapturedPassword.c_str());
        }
    }
}

// ============================================================================
// 重置 Cipher 狀態
// ============================================================================
static void ResetCipherState()
{
    g_cipherInitialized = false;
    g_cipherSeed = 0;
    g_cipherSend = Cipher();
    g_droppedFirstLogin = false;
    g_runtimeEncryptOn = false;
    g_rsaInited = false;
    g_forceRuntimeCipher = false;
    g_dropLoginIfNoUi = true;

    g_handshakeBufferLen = 0;
    memset(g_handshakeBuffer, 0, sizeof(g_handshakeBuffer));

    // ★ 不要清 g_forceCipherSniff
}

// ============================================================================
// 計算 MD5 (hex)
// ============================================================================
static void ComputeMd5Hex(const unsigned char* data, size_t len, char* out_hex, size_t out_size)
{
    if (!out_hex || out_size < 33) return;

    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    if (!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return;

    if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
    {
        if (CryptHashData(hHash, data, (DWORD)len, 0))
        {
            BYTE digest[16];
            DWORD digestLen = 16;
            if (CryptGetHashParam(hHash, HP_HASHVAL, digest, &digestLen, 0))
            {
                for (DWORD i = 0; i < digestLen; i++)
                    sprintf_s(&out_hex[i * 2], out_size - (i * 2), "%02x", digest[i]);
                out_hex[32] = '\0';
            }
        }
        CryptDestroyHash(hHash);
    }
    CryptReleaseContext(hProv, 0);
}

#ifdef _DEBUG
void __dbg_print(const char* fmt, ...)
{
    char buffer[8192] = { 0 };
    va_list args;
    va_start(args, fmt);
    vsprintf_s(buffer, fmt, args);
    va_end(args);
    OutputDebugStringA(buffer);
}
#else
void __dbg_print(const char* fmt, ...) { ; }
#endif

#define NAKED __declspec(naked)

// ============================================================================
// 共用 Log
// ============================================================================
static void HookLog(const char* fmt, ...)
{
    char buf[512] = { 0 };
    va_list args;
    va_start(args, fmt);
    _vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, args);
    va_end(args);
    OutputDebugStringA(buf);

    static HANDLE s_logHandle = INVALID_HANDLE_VALUE;
    static bool   s_logTriedOpen = false;
    if (!s_logTriedOpen)
    {
        char logPath[MAX_PATH] = { 0 };
        if (GetModuleFileNameA(NULL, logPath, MAX_PATH))
        {
            char* slash = strrchr(logPath, '\\');
            if (slash)
            {
                slash[1] = '\0';
                strncat_s(logPath, "LauncherDll.log", _TRUNCATE);
                s_logHandle = CreateFileA(logPath, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (s_logHandle != INVALID_HANDLE_VALUE)
                    SetFilePointer(s_logHandle, 0, NULL, FILE_END);
            }
        }
        s_logTriedOpen = true;
    }

    if (s_logHandle != INVALID_HANDLE_VALUE)
    {
        size_t len = strlen(buf);
        DWORD written = 0;
        WriteFile(s_logHandle, buf, (DWORD)len, &written, NULL);
        const char newline[] = "\r\n";
        WriteFile(s_logHandle, newline, ARRAYSIZE(newline) - 1, &written, NULL);
    }
}

// ============================================================================
// 偵測遊戲 exe 版本，決定要不要掛舊 UI hook
// ============================================================================
static void DetectGameVersion()
{
    if (g_versionStateInitialized)
        return;
    g_versionStateInitialized = true;

    char modulePath[MAX_PATH] = { 0 };
    if (!GetModuleFileNameA(NULL, modulePath, MAX_PATH))
    {
        HookLog("[LauncherDll] DetectGameVersion: GetModuleFileNameA failed (err=%lu)", GetLastError());
        g_legacyUiHooksEnabled = false;
        return;
    }

    DWORD handle = 0;
    DWORD verSize = GetFileVersionInfoSizeA(modulePath, &handle);
    if (verSize == 0)
    {
        HookLog("[LauncherDll] DetectGameVersion: no version info (err=%lu)", GetLastError());
        g_legacyUiHooksEnabled = false;
        return;
    }

    std::vector<BYTE> verData(verSize);
    if (!GetFileVersionInfoA(modulePath, handle, verSize, verData.data()))
    {
        HookLog("[LauncherDll] DetectGameVersion: GetFileVersionInfoA failed (err=%lu)", GetLastError());
        g_legacyUiHooksEnabled = false;
        return;
    }

    VS_FIXEDFILEINFO* verInfo = nullptr;
    UINT verInfoLen = 0;
    if (!VerQueryValueA(verData.data(), "\\", (LPVOID*)&verInfo, &verInfoLen) || !verInfo)
    {
        HookLog("[LauncherDll] DetectGameVersion: VerQueryValueA failed");
        g_legacyUiHooksEnabled = false;
        return;
    }

    DWORD major = HIWORD(verInfo->dwFileVersionMS);
    DWORD minor = LOWORD(verInfo->dwFileVersionMS);
    DWORD build = HIWORD(verInfo->dwFileVersionLS);
    DWORD revision = LOWORD(verInfo->dwFileVersionLS);

    g_detectedVersionMajor = major;
    g_detectedVersionMinor = minor;
    g_detectedVersionBuild = build;
    g_detectedVersionRevision = revision;

    HookLog("[LauncherDll] Game version detected: %lu.%lu.%lu.%lu", major, minor, build, revision);

    if (major < 17)
    {
        g_legacyUiHooksEnabled = true;
        HookLog("[LauncherDll] Legacy UI hooks enabled for this client build");
    }
    else
    {
        g_legacyUiHooksEnabled = false;
        HookLog("[LauncherDll] Legacy UI hooks disabled (client >= 17.x)");
    }
}

// ============================================================================
// 安全 Inline Hook
// ============================================================================
static bool TryInlineHookSafe(ULONG32 address, ULONG32 hookAddr, int inlineLen, const char* tag)
{
    __try
    {
        InlineHookEx(address, hookAddr, inlineLen);
        HookLog("[LauncherDll] InlineHookEx succeeded for %s at 0x%08X", tag, address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        HookLog("[LauncherDll] InlineHookEx FAILED for %s at 0x%08X (exception=0x%08X)", tag, address, GetExceptionCode());
        return false;
    }
}

static bool TryPatchMemorySafe(ULONG32 address, const void* bytes, int len, const char* tag)
{
    __try
    {
        PatachHook(address, (char*)bytes, len);
        HookLog("[LauncherDll] Patch succeeded for %s at 0x%08X", tag, address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        HookLog("[LauncherDll] Patch FAILED for %s at 0x%08X (exception=0x%08X)", tag, address, GetExceptionCode());
        return false;
    }
}

// ============================================================================
// 舊版客戶端 opcode → 新伺服器 opcode
// ============================================================================
static unsigned char TranslateLegacyClientOpcode(unsigned char opcode)
{
    switch (opcode)
    {
    case 3:   return C_VERSION;
    case 21:  return C_EXTENDED_PROTOBUF;
    case 28:  return C_ONOFF;
    case 32:  return C_CLIENT_READY;
    case 48:  // fall
    case 49:  // fall
    case 64:  // fall
        return 52;  // 新版登入
    case 158: return C_ALIVE;
    case 163: return C_ENTER_WORLD;
    case 202: return C_LOGOUT;
    default:  return opcode;
    }
}

static void ApplyOpcodeTranslation(unsigned char* data, size_t length, const char* contextTag)
{
    if (!data || length == 0) return;

    unsigned char legacy = data[0];
    unsigned char translated = TranslateLegacyClientOpcode(legacy);
    if (translated != legacy)
    {
        HookLog("[LauncherDll] translate opcode %u -> %u via %s",
            (unsigned)legacy, (unsigned)translated,
            contextTag ? contextTag : "send");
        data[0] = translated;
    }
}

// ============================================================================
// 原生 API 的跳板宣告
// ============================================================================
NAKED int WINAPI real_connect(SOCKET s, const struct sockaddr* name, int namelen)
{
    __asm
    {
        mov edi, edi
        push ebp
        mov  ebp, esp
        mov  eax, connect
        add  eax, 5
        jmp  eax
    }
}

typedef int (WINAPI* PFN_SEND)(SOCKET, const char*, int, int);
static PFN_SEND g_original_send = nullptr;

NAKED int WINAPI real_send(SOCKET s, const char* buf, int len, int flag)
{
    __asm
    {
        mov edi, edi
        push ebp
        mov  ebp, esp
        mov  eax, send
        add  eax, 5
        jmp  eax
    }
}

NAKED int WINAPI real_WSASend(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    __asm
    {
        mov edi, edi
        push ebp
        mov  ebp, esp
        mov  eax, WSASend
        add  eax, 5
        jmp  eax
    }
}

NAKED int WINAPI real_recv(SOCKET s, char* buf, int len, int flag)
{
    __asm
    {
        mov edi, edi
        push ebp
        mov  ebp, esp
        mov  eax, recv
        add  eax, 5
        jmp  eax
    }
}

NAKED HWND WINAPI real_CreateWindowEx(
    DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
    DWORD dwStyle, int x, int y, int nWidth, int nHeight,
    HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    __asm
    {
        mov edi, edi
        push ebp
        mov  ebp, esp
        mov  eax, CreateWindowEx
        add  eax, 5
        jmp  eax
    }
}

// ============================================================================
// connect hook → 強制改成 ShareInfo.ip / port
// ============================================================================
int WINAPI my_connect(SOCKET s, const struct sockaddr* name, int namelen)
{
    sockaddr_in* addr_in = (sockaddr_in*)name;
    addr_in->sin_addr.s_addr = inet_addr(ShareInfo.ip);
    addr_in->sin_port = htons(ShareInfo.port);
    return real_connect(s, name, namelen);
}

// ============================================================================
// 啟動器隨機數
// ============================================================================
int nextRand()
{
    _seed = (214013 * _seed + 2531011) & 0x7FFFFFFF;
    return (int)(_seed >> 16) & 0xFF;
}

// ============================================================================
// 判斷是不是登入包
// ============================================================================
static bool IsLoginLikePacket(bool hasLengthPrefix, size_t packetSize,
    unsigned char originalOpcode,
    unsigned char translatedOpcode)
{
    // 1. 新版 Login → opcode 52
    if (translatedOpcode == 52) return true;

    // 2. 舊版轉成 52 的
    unsigned char probeTranslated = TranslateLegacyClientOpcode(originalOpcode);
    if (probeTranslated == 52) return true;

    // 3. DLL 送來的登入包 → 都是 length-prefix，長度大約 340~360
    if (hasLengthPrefix && (packetSize >= 340 && packetSize <= 360))
        return true;

    return false;
}

// ============================================================================
// Dump 登入包 (原本就有的行為)
// ============================================================================
static void DumpLoginPacket(const unsigned char* data, size_t len,
    bool hasLengthPrefix, int opcodeOffset,
    unsigned char originalOpcode, unsigned char translatedOpcode,
    unsigned char probeTranslated,
    int shapeLikeDll, int s_loginDumpIndex)
{
    SYSTEMTIME st; GetLocalTime(&st);
    char basePath[MAX_PATH] = { 0 };
    char dirPath[MAX_PATH] = { 0 };

    if (GetModuleFileNameA(NULL, basePath, MAX_PATH))
    {
        char* slash = strrchr(basePath, '\\');
        if (slash) slash[1] = '\0';
    }
    else
    {
        strcpy_s(basePath, ".\\");
    }

    sprintf_s(dirPath, "%s%s", basePath, "login_clientdump\\");
    CreateDirectoryA(dirPath, NULL);

    char binPath[MAX_PATH] = { 0 };
    char txtPath[MAX_PATH] = { 0 };
    sprintf_s(binPath, "%slogin_%04d%02d%02d_%02d%02d%02d_%03d.bin", dirPath,
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    sprintf_s(txtPath, "%slogin_%04d%02d%02d_%02d%02d%02d_%03d.txt", dirPath,
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    HANDLE hBin = CreateFileA(binPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hBin != INVALID_HANDLE_VALUE)
    {
        DWORD written = 0;
        WriteFile(hBin, data, (DWORD)len, &written, NULL);
        CloseHandle(hBin);
    }

    HANDLE hTxt = CreateFileA(txtPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTxt != INVALID_HANDLE_VALUE)
    {
        auto writeLine = [&](const char* s)
            {
                DWORD w = 0;
                WriteFile(hTxt, s, (DWORD)strlen(s), &w, NULL);
                const char crlf[] = "\r\n";
                WriteFile(hTxt, crlf, 2, &w, NULL);
            };

        char header[512];
        sprintf_s(header,
            "len=%zu hasLenPrefix=%d opcodeOffset=%d originalOpcode=%u translatedOpcode=%u probeTranslated=%u seed=0x%08X launcherEnc=%d runtimeEnc=%d cipherInit=%d shapeLikeDll=%d",
            len, hasLengthPrefix ? 1 : 0, opcodeOffset,
            (unsigned)originalOpcode, (unsigned)translatedOpcode, (unsigned)probeTranslated,
            g_cipherSeed,
            g_respectLauncherEncrypt ? 1 : 0,
            g_runtimeEncryptOn ? 1 : 0,
            g_cipherInitialized ? 1 : 0,
            shapeLikeDll ? 1 : 0);

        writeLine("==== LauncherDll login candidate dump ====");
        writeLine(header);

        for (size_t i = 0; i < len; i += 16)
        {
            char line[256] = { 0 };
            int off = 0;
            off += sprintf_s(line + off, sizeof(line) - off, "%04zx: ", i);
            for (int j = 0; j < 16; ++j)
            {
                if (i + j < len) off += sprintf_s(line + off, sizeof(line) - off, "%02X ", data[i + j]);
                else off += sprintf_s(line + off, sizeof(line) - off, "   ");
            }
            off += sprintf_s(line + off, sizeof(line) - off, " | ");
            for (int j = 0; j < 16 && i + j < len; ++j)
            {
                unsigned char c = data[i + j];
                off += sprintf_s(line + off, sizeof(line) - off, "%c", (c >= 32 && c < 127) ? c : '.');
            }
            writeLine(line);
        }

        writeLine("-- XOR 0xAA view (first 256 bytes) --");
        size_t viewLen = len < 256 ? len : 256;
        for (size_t i = 0; i < viewLen; i += 16)
        {
            char line[256] = { 0 };
            int off = 0;
            off += sprintf_s(line + off, sizeof(line) - off, "%04zx: ", i);
            for (int j = 0; j < 16; ++j)
            {
                if (i + j < viewLen) off += sprintf_s(line + off, sizeof(line) - off, "%02X ", (unsigned char)(data[i + j] ^ 0xAA));
                else off += sprintf_s(line + off, sizeof(line) - off, "   ");
            }
            off += sprintf_s(line + off, sizeof(line) - off, " | ");
            for (int j = 0; j < 16 && i + j < viewLen; ++j)
            {
                unsigned char c = (unsigned char)(data[i + j] ^ 0xAA);
                off += sprintf_s(line + off, sizeof(line) - off, "%c", (c >= 32 && c < 127) ? c : '.');
            }
            writeLine(line);
        }

        CloseHandle(hTxt);
        HookLog("[LauncherDll] dumped login candidate to %s and %s", binPath, txtPath);
    }
}

// ============================================================================
// 我們自己的 send
// ============================================================================
int WINAPI my_send(SOCKET s, const char FAR* buf, int len, int flags)
{
    int ret;

    // 遞迴保護
    static thread_local int send_recursion_count = 0;
    struct RecursionGuard { RecursionGuard() { send_recursion_count++; } ~RecursionGuard() { send_recursion_count--; } } guard;
    if (send_recursion_count > 0)
    {
        HookLog("[LauncherDll] !!! RECURSION in my_send detected, bypass");
        if (g_original_send)
            return g_original_send(s, buf, len, flags);
        return real_send(s, buf, len, flags);
    }

    if (!buf || len <= 0)
        return real_send(s, buf, len, flags);

    // 前幾包 dump 一下
    static int s_rawPacketCount = 0;
    if (s_rawPacketCount < 20)
    {
        HookLog("[LauncherDll] ========== Raw Packet #%d (len=%d) seed=0x%08X cipher_init=%d ==========",
            s_rawPacketCount, len, g_cipherSeed, g_cipherInitialized ? 1 : 0);
        for (int i = 0; i < len; i += 16)
        {
            char hexLine[256] = { 0 };
            char asciiLine[32] = { 0 };
            int hexOffset = 0, asciiOffset = 0;
            for (int j = 0; j < 16; j++)
            {
                if (i + j < len)
                    hexOffset += sprintf_s(hexLine + hexOffset, sizeof(hexLine) - hexOffset, "%02X ", (unsigned char)buf[i + j]);
                else
                    hexOffset += sprintf_s(hexLine + hexOffset, sizeof(hexLine) - hexOffset, "   ");
            }
            for (int j = 0; j < 16 && i + j < len; j++)
            {
                unsigned char c = (unsigned char)buf[i + j];
                asciiOffset += sprintf_s(asciiLine + asciiOffset, sizeof(asciiLine) - asciiOffset,
                    "%c", (c >= 32 && c < 127) ? c : '.');
            }
            HookLog("[LauncherDll] %04X: %s | %s", i, hexLine, asciiLine);
        }
        HookLog("[LauncherDll] ========== End Packet #%d ==========", s_rawPacketCount);
        ++s_rawPacketCount;
    }

    // 準備工作 buffer
    unsigned char stackBuffer[4096];
    unsigned char* workingBuffer = nullptr;
    std::vector<unsigned char> heapBuffer;
    size_t packetSize = (size_t)len;
    if (packetSize <= sizeof(stackBuffer))
        workingBuffer = stackBuffer;
    else
    {
        heapBuffer.resize(packetSize);
        workingBuffer = heapBuffer.data();
    }
    memcpy(workingBuffer, buf, packetSize);

    // 判斷有沒有 2 bytes 長度
    bool hasLengthPrefix = false;
    int  opcodeOffset = 0;
    unsigned char originalOpcode = workingBuffer[0];
    if (packetSize >= 3)
    {
        uint16_t possibleLength = workingBuffer[0] | (workingBuffer[1] << 8);
        if (possibleLength == packetSize)
        {
            hasLengthPrefix = true;
            opcodeOffset = 2;
            originalOpcode = workingBuffer[2];
        }
    }

    static int s_sendLogCount = 0;
    if (s_sendLogCount < 50)
    {
        char hexBuf[128] = { 0 };
        int offset = 0;
        int dumpLen = (packetSize > 32) ? 32 : (int)packetSize;
        for (int i = 0; i < dumpLen; ++i)
            offset += sprintf_s(hexBuf + offset, sizeof(hexBuf) - offset, "%02X ", workingBuffer[i]);

        HookLog("[LauncherDll] my_send #%d: len=%d hasLengthPrefix=%d opcode=%u hex=[%s%s]",
            s_sendLogCount, len, hasLengthPrefix ? 1 : 0,
            (unsigned)originalOpcode,
            hexBuf, packetSize > 32 ? "..." : "");
        ++s_sendLogCount;
    }

    // 有開加密時才翻 opcode
    if (g_cipherInitialized && g_runtimeEncryptOn)
        ApplyOpcodeTranslation(workingBuffer + opcodeOffset, packetSize - opcodeOffset, "send");

    unsigned char translatedOpcode = workingBuffer[opcodeOffset];
    if (originalOpcode != translatedOpcode)
        HookLog("[LauncherDll] Opcode translated: %u -> %u", originalOpcode, translatedOpcode);

    // -------------------------------------------------------------
    // ★★★ 這段：偵測登入包 + 沒抓到 UI 要不要擋 ★★★
    // -------------------------------------------------------------
    {
        bool isLoginCandidate = IsLoginLikePacket(hasLengthPrefix, packetSize, originalOpcode, translatedOpcode);
        if (isLoginCandidate)
        {
            CaptureLoginInputs();

            const char* acc = g_lastCapturedAccount.empty() ? "<empty>" : g_lastCapturedAccount.c_str();
            const char* pwd = g_lastCapturedPassword.empty() ? "<empty>" : g_lastCapturedPassword.c_str();
            HookLog("[LauncherDll] Login packet detected: captured credentials account='%s' password='%s'", acc, pwd);

            // ★ 重點：launcher=1 才會強制要 UI；launcher=0 直接放
            if (g_respectLauncherEncrypt)
            {
                if (g_dropLoginIfNoUi &&
                    g_lastCapturedAccount.empty() &&
                    g_lastCapturedPassword.empty())
                {
                    HookLog("[LauncherDll] ❌ launcher=1 + 沒抓到帳密，這包登入『不送』");
                    return len; // 假裝送成功
                }
            }
            else
            {
                HookLog("[LauncherDll] launcher=0 → 不做 UI 強制檢查，直接放行登入包");
            }

            // 備份 dump
            static int s_loginDumpCount = 0;
            if (s_loginDumpCount < 10)
            {
                unsigned char probeTranslated = TranslateLegacyClientOpcode(originalOpcode);
                int shapeLikeDll = (hasLengthPrefix && (packetSize >= 340 && packetSize <= 360)) ? 1 : 0;
                DumpLoginPacket(workingBuffer, packetSize,
                    hasLengthPrefix, opcodeOffset,
                    originalOpcode, translatedOpcode,
                    probeTranslated, shapeLikeDll,
                    s_loginDumpCount);
                ++s_loginDumpCount;
            }
        }
    }
    // -------------------------------------------------------------
    // ★★★ 登入包處理完畢 ★★★
    // -------------------------------------------------------------

    HookLog("[LauncherDll] Encryption check: launcher=%d runtime=%d cipherInit=%d",
        g_respectLauncherEncrypt ? 1 : 0,
        g_runtimeEncryptOn ? 1 : 0,
        g_cipherInitialized ? 1 : 0);

    // 有開加密 → 走 Cipher
    if (g_runtimeEncryptOn && g_cipherInitialized)
    {
        int encryptOffset = hasLengthPrefix ? 2 : 0;
        int encryptLen = len - encryptOffset;

        HookLog("[LauncherDll] send WITH Cipher: len=%d opcode=%u hasLengthPrefix=%d encryptLen=%d",
            len, (unsigned)workingBuffer[opcodeOffset], hasLengthPrefix ? 1 : 0, encryptLen);

        std::vector<uint8_t> plain(workingBuffer + encryptOffset, workingBuffer + len);
        try
        {
            std::vector<uint8_t> encrypted = g_cipherSend.encryptClient(plain);

            std::vector<uint8_t> finalPacket;
            if (hasLengthPrefix)
            {
                uint16_t totalLen = static_cast<uint16_t>(encrypted.size() + 2);
                finalPacket.reserve(encrypted.size() + 2);
                finalPacket.push_back(totalLen & 0xFF);
                finalPacket.push_back((totalLen >> 8) & 0xFF);
                finalPacket.insert(finalPacket.end(), encrypted.begin(), encrypted.end());
            }
            else
            {
                finalPacket = encrypted;
            }

            int result = SOCKET_ERROR;
            if (g_original_send)
                result = g_original_send(s, (const char*)finalPacket.data(), (int)finalPacket.size(), flags);
            else
                result = real_send(s, (const char*)finalPacket.data(), (int)finalPacket.size(), flags);

            return result;
        }
        catch (const std::exception& ex)
        {
            HookLog("[LauncherDll] Cipher encryptClient failed: %s", ex.what());
            return SOCKET_ERROR;
        }
    }

    // 沒開加密 → 原封不動送出去
    HookLog("[LauncherDll] send WITHOUT Cipher: len=%d opcode=%u (cipherInit=%d runtime=%d launcher=%d)",
        len, (unsigned)workingBuffer[opcodeOffset],
        g_cipherInitialized ? 1 : 0,
        g_runtimeEncryptOn ? 1 : 0,
        g_respectLauncherEncrypt ? 1 : 0);

    if (g_original_send)
        ret = g_original_send(s, (const char*)workingBuffer, len, flags);
    else
        ret = real_send(s, (const char*)workingBuffer, len, flags);
    return ret;
}

// ============================================================================
// WSASend
// ============================================================================
int WINAPI my_WSASend(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    static int s_wsaSendLogCount = 0;
    if (s_wsaSendLogCount < 5)
    {
        DWORD firstLen = 0;
        unsigned int firstByte = 0;
        if (lpBuffers && dwBufferCount > 0 && lpBuffers[0].buf && lpBuffers[0].len > 0)
        {
            firstLen = lpBuffers[0].len;
            firstByte = (unsigned char)lpBuffers[0].buf[0];
        }
        HookLog("[LauncherDll] my_WSASend buffers=%lu firstLen=%lu firstByte=%u",
            dwBufferCount, firstLen, firstByte);
        ++s_wsaSendLogCount;
    }

    // 沒開加密就丟回原函式
    if (!g_cipherInitialized || !g_runtimeEncryptOn)
        return real_WSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);

    if (lpBuffers && dwBufferCount > 0)
    {

        // 非同步 send → 不包起來
        if ((lpOverlapped != NULL) || (lpCompletionRoutine != NULL))
        {
            static bool s_warned = false;
            if (!s_warned)
            {
                HookLog("[LauncherDll] WARNING: WSASend overlapped, cannot cipher → send plaintext");
                s_warned = true;
            }
        }
        else
        {
            // 集中加密
            size_t totalBytes = 0;
            for (DWORD i = 0; i < dwBufferCount; ++i)
            {
                if (lpBuffers[i].buf)
                    totalBytes += lpBuffers[i].len;
            }
            if (totalBytes > 0)
            {
                std::vector<uint8_t> raw;
                raw.reserve(totalBytes);
                for (DWORD i = 0; i < dwBufferCount; ++i)
                {
                    if (!lpBuffers[i].buf || !lpBuffers[i].len) continue;

                    unsigned char* bytes = (unsigned char*)lpBuffers[i].buf;
                    raw.insert(raw.end(), bytes, bytes + lpBuffers[i].len);
                }

                bool hasLengthPrefix = false;
                std::vector<uint8_t> plain;
                if (raw.size() >= 3)
                {
                    uint16_t possibleLength = raw[0] | (raw[1] << 8);
                    if (possibleLength == raw.size())
                    {
                        hasLengthPrefix = true;
                        plain.assign(raw.begin() + 2, raw.end());
                    }
                }

                if (!hasLengthPrefix)
                    plain = raw;

                if (!plain.empty())
                {
                    ApplyOpcodeTranslation(plain.data(), plain.size(), "WSASend");
                }

                try
                {
                    std::vector<uint8_t> encrypted = g_cipherSend.encryptClient(plain);
                    std::vector<uint8_t> finalPacket;
                    if (hasLengthPrefix)
                    {
                        uint16_t totalLen = static_cast<uint16_t>(encrypted.size() + 2);
                        finalPacket.reserve(encrypted.size() + 2);
                        finalPacket.push_back(totalLen & 0xFF);
                        finalPacket.push_back((totalLen >> 8) & 0xFF);
                        finalPacket.insert(finalPacket.end(), encrypted.begin(), encrypted.end());
                    }
                    else
                    {
                        finalPacket = encrypted;
                    }

                    WSABUF singleBuf;
                    singleBuf.buf = (char*)finalPacket.data();
                    singleBuf.len = (ULONG)finalPacket.size();

                    DWORD tmp = 0;
                    LPDWORD pOut = lpNumberOfBytesSent ? lpNumberOfBytesSent : &tmp;

                    int sendResult = real_WSASend(s, &singleBuf, 1, pOut, dwFlags, lpOverlapped, lpCompletionRoutine);
                    if (sendResult != SOCKET_ERROR && lpNumberOfBytesSent)
                        *lpNumberOfBytesSent = (DWORD)finalPacket.size();
                    return sendResult;
                }
                catch (const std::exception& ex)
                {
                    HookLog("[LauncherDll] Cipher encryptClient failed in WSASend: %s", ex.what());
                    return SOCKET_ERROR;
                }
            }
        }
    }

    return real_WSASend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine);
}

// ============================================================================
// recv
// ============================================================================
int WINAPI my_recv(SOCKET s, char* buf, int len, int flag)
{
    int ret;

    // 先看 RSA
    char buffer32[32];
    if (g_respectLauncherEncrypt && !g_rsaInited)
    {
        memset(buffer32, 0, sizeof(buffer32));
        int need = 0;
        while (need < 4)
        {
            ret = real_recv(s, &buffer32[need], 4 - need, 0);
            if (ret > 0) need += ret;
            else
            {
                if (WSAGetLastError() == WSAEWOULDBLOCK)
                    continue;
                else
                    return ret;
            }
        }

        unsigned long _authdata = *(unsigned long*)buffer32;
        BIGNUM* c = BN_new();
        BIGNUM* m = BN_new();
        BN_set_word(c, _authdata);
        BN_mod_exp(m, c, d, n, bn_ctx);

        if (ShareInfo.randenc)
            _seed = strtoul(BN_bn2hex(m), NULL, 16);
        else
            _xorByte = (unsigned char)strtoul(BN_bn2hex(m), NULL, 16);

        BN_free(c);
        BN_free(m);

        g_rsaInited = true;
        HookLog("[LauncherDll] RSA handshake completed");
    }

    // 真正收資料
    ret = real_recv(s, buf, len, flag);
    if (ret <= 0) return ret;

    unsigned char opcode = (unsigned char)buf[0];

    static int s_recvLogCount = 0;
    bool shouldLog = (s_recvLogCount < 50) || (opcode == 147) || (opcode == 0x93);
    if (shouldLog && ret <= 64)
    {
        char hexBuf[256] = { 0 };
        int offset = 0;
        for (int i = 0; i < ret && i < 64; ++i)
            offset += sprintf_s(hexBuf + offset, sizeof(hexBuf) - offset, "%02X ", (unsigned char)buf[i]);
        HookLog("[LauncherDll] recv #%d: len=%d opcode=%u hex=[%s] launcherEnc=%d runtimeEnc=%d cipherInit=%d",
            s_recvLogCount, ret, opcode, hexBuf,
            g_respectLauncherEncrypt ? 1 : 0,
            g_runtimeEncryptOn ? 1 : 0,
            g_cipherInitialized ? 1 : 0);
        s_recvLogCount++;
    }
    else if (shouldLog)
    {
        HookLog("[LauncherDll] recv #%d: len=%d opcode=%u launcherEnc=%d runtimeEnc=%d cipherInit=%d",
            s_recvLogCount, ret, opcode,
            g_respectLauncherEncrypt ? 1 : 0,
            g_runtimeEncryptOn ? 1 : 0,
            g_cipherInitialized ? 1 : 0);
        s_recvLogCount++;
    }

    // 若已加密 → 有可能收到「長度 + 加密」
    if (g_cipherInitialized && g_runtimeEncryptOn && ret >= 3)
    {
        uint16_t packetLength = (unsigned char)buf[0] | ((unsigned char)buf[1] << 8);
        if (packetLength == ret)
        {
            HookLog("[LauncherDll] Detected length prefix: %u bytes, removing it", packetLength);
            memmove(buf, buf + 2, ret - 2);
            ret -= 2;
            opcode = (unsigned char)buf[0];
            HookLog("[LauncherDll] After removing length prefix: len=%d opcode=%u", ret, opcode);
        }
    }

    // 想抓 S_KEY
    bool wantCipher = (g_respectLauncherEncrypt || g_forceCipherSniff);

    // 還沒抓到 → 緩衝
    if (!g_cipherInitialized && wantCipher)
    {
        // 累積
        if (g_handshakeBufferLen + ret <= (int)sizeof(g_handshakeBuffer))
        {
            memcpy(g_handshakeBuffer + g_handshakeBufferLen, buf, ret);
            g_handshakeBufferLen += ret;
            HookLog("[LauncherDll] Accumulating handshake data: added=%d total=%d/%d",
                ret, g_handshakeBufferLen, HANDSHAKE_EXPECTED_LEN);
        }

        // 若夠長 → 開始找 0x4F (S_KEY)
        if (g_handshakeBufferLen >= HANDSHAKE_EXPECTED_LEN)
        {
            int skeyPos = -1;
            for (int i = 0; i <= g_handshakeBufferLen - 5; i++)
            {
                if ((unsigned char)g_handshakeBuffer[i] == 79)
                {
                    skeyPos = i;
                    break;
                }
            }

            if (skeyPos >= 0 && skeyPos + 5 <= g_handshakeBufferLen)
            {
                unsigned char* skeyData = (unsigned char*)(g_handshakeBuffer + skeyPos);
                uint32_t seed = skeyData[1] | (skeyData[2] << 8) | (skeyData[3] << 16) | (skeyData[4] << 24);

                HookLog("[LauncherDll] ✅ S_KEY packet found at offset %d! Raw bytes: %02X %02X %02X %02X %02X, seed=0x%08X",
                    skeyPos, skeyData[0], skeyData[1], skeyData[2], skeyData[3], skeyData[4], seed);

                if (g_cipherInitialized && g_cipherSeed == seed)
                {
                    HookLog("[LauncherDll] ⚠️ S_KEY already processed (seed=0x%08X), ignoring duplicate", seed);
                    g_handshakeBufferLen = 0;
                }
                else
                {
                    // 初始化 Cipher
                    g_cipherSend.initKeys((int)seed);
                    g_cipherSeed = seed;
                    g_cipherInitialized = true;
                    g_gameSocket = s;

                    if (g_respectLauncherEncrypt)
                    {
                        g_runtimeEncryptOn = true;
                        const char* mode = g_forceRuntimeCipher ? "auto-encrypt" : "launcher=1";
                        HookLog("[LauncherDll] Runtime encryption ENABLED (%s, seed=0x%08X)", mode, seed);
                    }
                    else
                    {
                        g_runtimeEncryptOn = false;
                        HookLog("[LauncherDll] Runtime encryption DISABLED (launcher=0, just sniff S_KEY, seed=0x%08X)", seed);
                    }

                    // 做回送 C_ServerVersion
                    char versionPacket[20] = { 0 };
                    versionPacket[0] = 20;
                    versionPacket[1] = 0;
                    versionPacket[2] = 3;

                    if (skeyPos + 13 <= g_handshakeBufferLen)
                    {
                        memcpy(&versionPacket[3], &skeyData[5], 8);
                        HookLog("[LauncherDll] clientVersion from handshake: %02X %02X %02X %02X %02X %02X %02X %02X",
                            (unsigned char)versionPacket[3], (unsigned char)versionPacket[4],
                            (unsigned char)versionPacket[5], (unsigned char)versionPacket[6],
                            (unsigned char)versionPacket[7], (unsigned char)versionPacket[8],
                            (unsigned char)versionPacket[9], (unsigned char)versionPacket[10]);
                    }
                    else
                    {
                        HookLog("[LauncherDll] WARNING: Not enough data for clientVersion, using zeros");
                    }

                    HookLog("[LauncherDll] Preparing C_ServerVersion (20 bytes) after S_KEY...");
                    {
                        char hexBuf[256] = { 0 };
                        int offset = 0;
                        for (int i = 0; i < 20; i++)
                            offset += sprintf_s(hexBuf + offset, sizeof(hexBuf) - offset, "%02X ", (unsigned char)versionPacket[i]);
                        HookLog("[LauncherDll] C_ServerVersion (plain): %s", hexBuf);
                    }

                    if (g_respectLauncherEncrypt)
                    {
                        std::vector<unsigned char> plain;
                        for (int i = 2; i < 20; i++)
                            plain.push_back((unsigned char)versionPacket[i]);

                        std::vector<unsigned char> encrypted = g_cipherSend.encryptClient(plain);
                        int totalLen = 2 + (int)encrypted.size();
                        std::vector<char> fullPacket;
                        fullPacket.reserve(totalLen);
                        fullPacket.push_back(totalLen & 0xFF);
                        fullPacket.push_back((totalLen >> 8) & 0xFF);
                        for (size_t i = 0; i < encrypted.size(); i++)
                            fullPacket.push_back((char)encrypted[i]);

                        int sendRet = real_send(s, fullPacket.data(), (int)fullPacket.size(), 0);
                        HookLog("[LauncherDll] ✅ C_ServerVersion sent: %d bytes (encrypted, launcher=1)", sendRet);
                    }
                    else
                    {
                        int sendRet = real_send(s, versionPacket, 20, 0);
                        HookLog("[LauncherDll] ✅ C_ServerVersion sent: %d bytes (plain, launcher=0)", sendRet);
                    }

                    g_handshakeBufferLen = 0;
                    memset(g_handshakeBuffer, 0, sizeof(g_handshakeBuffer));
                    g_droppedFirstLogin = false;
                }
            }
            else
            {
                HookLog("[LauncherDll] Buffered %d bytes but no valid S_KEY found yet", g_handshakeBufferLen);
            }
        }
        else
        {
            HookLog("[LauncherDll] Waiting for more handshake data: %d/%d bytes", g_handshakeBufferLen, HANDSHAKE_EXPECTED_LEN);
        }
    }
    // 單包 fallback
    else if (!g_cipherInitialized && wantCipher && ret >= 5)
    {
        if ((unsigned char)buf[0] == 79)
        {
            uint32_t seed =
                (unsigned char)buf[1] |
                ((unsigned char)buf[2] << 8) |
                ((unsigned char)buf[3] << 16) |
                ((unsigned char)buf[4] << 24);

            HookLog("[LauncherDll] [Fallback] S_KEY packet detected in single recv! seed=0x%08X", seed);

            g_cipherSend.initKeys((int)seed);
            g_cipherSeed = seed;
            g_cipherInitialized = true;
            g_gameSocket = s;

            if (g_respectLauncherEncrypt)
            {
                g_runtimeEncryptOn = true;
                const char* mode = g_forceRuntimeCipher ? "auto-encrypt" : "launcher=1";
                HookLog("[LauncherDll] Runtime encryption ENABLED (fallback, %s, seed=0x%08X)", mode, seed);
            }
            else
            {
                g_runtimeEncryptOn = false;
                HookLog("[LauncherDll] Runtime encryption DISABLED (fallback, launcher=0, just sniff S_KEY, seed=0x%08X)", seed);
            }

            g_droppedFirstLogin = false;
        }
    }
    // 收到加密封包 → 嘗試解密
    else if (g_cipherInitialized && g_runtimeEncryptOn)
    {
        if (ret >= 4)
        {
            HookLog("[LauncherDll] Attempting to decrypt packet: len=%d opcode(encrypted)=%u", ret, (unsigned char)buf[0]);
            try
            {
                std::vector<uint8_t> encrypted((uint8_t*)buf, (uint8_t*)buf + ret);
                std::vector<uint8_t> plain = g_cipherSend.decryptServer(encrypted);
                if (plain.size() <= (size_t)len)
                {
                    memcpy(buf, plain.data(), plain.size());
                    ret = (int)plain.size();
                    unsigned char decOp = (unsigned char)buf[0];
                    HookLog("[LauncherDll] Decryption successful: len=%d opcode(decrypted)=%u", ret, decOp);

                    const char* packetName = nullptr;
                    if (decOp == 93)       packetName = "S_LoginResult";
                    else if (decOp == 84)  packetName = "S_CharAmount";
                    else if (decOp == 147) packetName = "S_ServerVersion";
                    else if (decOp == 79)  packetName = "S_KEY";

                    if (packetName)
                        HookLog("[LauncherDll] >>> Received important packet: %s (opcode=%u)", packetName, decOp);
                }
                else
                {
                    HookLog("[LauncherDll] decryptServer returned oversized buffer (%zu > %d)", plain.size(), len);
                    ret = SOCKET_ERROR;
                }
            }
            catch (const std::exception& ex)
            {
                HookLog("[LauncherDll] Cipher decryptServer exception: %s", ex.what());
                ret = SOCKET_ERROR;
            }
        }
        else
        {
            HookLog("[LauncherDll] Packet too short for decryption (len=%d)", ret);
        }
    }

    return ret;
}

// ============================================================================
// 建立遊戲視窗 → 換標題字串 + 開 helper
// ============================================================================
HWND WINAPI my_CreateWindowEx(
    DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
    DWORD dwStyle, int x, int y, int nWidth, int nHeight,
    HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    if (_stricmp(lpClassName, "Lineage") == 0)
    {
        char szTitle[32] = { 0 };
        srand(GetTickCount());
        const char* str = "abcdefghigklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        int cnt = 8 + (rand() % 8);
        for (int i = 0; i < cnt; i++)
            szTitle[i] = str[rand() % strlen(str)];
        szTitle[cnt] = 0;

        lpWindowName = szTitle;
        g_hGameMainWnd = real_CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle,
            x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

        if (ShareInfo.usehelper)
            LoadHelp(g_hGameMainWnd);

        return g_hGameMainWnd;
    }

    return real_CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle,
        x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

// ============================================================================
// 幫助視窗掛勾 / 拔除
// ============================================================================
bool LoadHelp(HWND hGameWnd)
{
    if (!IsWindow(hGameWnd))
        return false;

    DWORD GPid = GetWindowThreadProcessId(hGameWnd, NULL);
    hhk = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, hins, GPid);
    return true;
}

void UnLoadHelp()
{
    TRY
    {
        if (g_hWndThread != INVALID_HANDLE_VALUE)
            TerminateThread(g_hWndThread, -2);

        if (g_pHelpMainDlaog)
            g_pHelpMainDlaog->SaveUserConfig();

        if (hhk)
            UnhookWindowsHookEx(hhk);
    }
        CATCH(CMemoryException, e)
    {
        delete e;
    }
    END_CATCH
}

// ============================================================================
// IAT / EAT hook helper
// ============================================================================
BOOL HookIatApi(char* pszDllName, DWORD dwTargetFunction, DWORD dwNewFunction, void** lplpOldFunction)
{
    if (!pszDllName) return FALSE;
    BOOL bFlag = FALSE;
    BOOL bFind = FALSE;

    DWORD dwModuleBase = *(DWORD*)(__readfsdword(0x30) + 0x08);
    PIMAGE_DOS_HEADER pidh = (PIMAGE_DOS_HEADER)dwModuleBase;
    PIMAGE_NT_HEADERS pinh = (PIMAGE_NT_HEADERS)(dwModuleBase + pidh->e_lfanew);
    PIMAGE_DATA_DIRECTORY pDir = pinh->OptionalHeader.DataDirectory;
    PIMAGE_IMPORT_DESCRIPTOR pid = (PIMAGE_IMPORT_DESCRIPTOR)(dwModuleBase + pDir[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while (pid->Name)
    {
        char* pszName = (char*)(pid->Name + dwModuleBase);
        if (_stricmp(pszDllName, pszName) == 0)
        {
            bFind = TRUE;
            break;
        }
        pid++;
    }

    if (bFind)
    {
        DWORD dwFunctionPtr = pid->FirstThunk + dwModuleBase;
        DWORD dwBackup = 0, dwBackupOld = 0;

        while (*(DWORD*)dwFunctionPtr)
        {
            if (*(DWORD*)dwFunctionPtr == dwTargetFunction)
            {
                VirtualProtect((LPVOID)dwFunctionPtr, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &dwBackup);
                *lplpOldFunction = (void*)(*(DWORD*)dwFunctionPtr);
                *(DWORD*)dwFunctionPtr = dwNewFunction;
                VirtualProtect((LPVOID)dwFunctionPtr, sizeof(DWORD), dwBackup, &dwBackupOld);
                bFlag = TRUE;
                break;
            }
            dwFunctionPtr += 4;
        }
    }
    return bFlag;
}

BOOL HookEatApi(HMODULE hDllModule, DWORD dwTargetFunction, DWORD lpNewFunction, void** lplpOldFunction)
{
    if (!hDllModule) return FALSE;
    BOOL bFlag = FALSE;

    DWORD dwBackup = 0, dwBackupOld = 0;
    PIMAGE_DOS_HEADER pidh = (PIMAGE_DOS_HEADER)hDllModule;
    PIMAGE_NT_HEADERS pinh = (PIMAGE_NT_HEADERS)((DWORD)pidh + pidh->e_lfanew);
    PIMAGE_DATA_DIRECTORY pDir = pinh->OptionalHeader.DataDirectory;
    PIMAGE_EXPORT_DIRECTORY pied = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pidh + pDir->VirtualAddress);
    DWORD* lpFunctionTable = (DWORD*)(pied->AddressOfFunctions + (DWORD)hDllModule);

    for (int i = 0; i < (int)pied->NumberOfFunctions; i++)
    {
        DWORD dwFunctionAddr = lpFunctionTable[i] + (DWORD)hDllModule;
        if (dwFunctionAddr == dwTargetFunction)
        {
            VirtualProtect(&lpFunctionTable[i], sizeof(DWORD), PAGE_EXECUTE_READWRITE, &dwBackup);
            *lplpOldFunction = (void*)(lpFunctionTable[i] + (DWORD)hDllModule);
            lpFunctionTable[i] = (lpNewFunction - (DWORD)hDllModule);
            VirtualProtect(&lpFunctionTable[i], sizeof(DWORD), dwBackup, &dwBackupOld);
            bFlag = TRUE;
            break;
        }
    }

    return bFlag;
}

// ============================================================================
// GetACP hook
// ============================================================================
UINT WINAPI my_GetACP(void);
typedef UINT(WINAPI* fn_GetACP)(void);
fn_GetACP real_GetACP = NULL;
BOOL bInitialize = FALSE;

// ============================================================================
// InlineSystemHook → 把 Winsock、CreateWindowExA、GetACP 掛進去
// ============================================================================
bool InlineSystemHook(bool bIsHook)
{
    if (bIsHook)
    {
        ResetCipherState();

        char szKernel32[] = "kernel32.dll";
        char szUser32[] = "user32.dll";
        char szWs2_32[] = "ws2_32.dll";

        HMODULE hKernel32 = GetModuleHandleA(szKernel32);
        HMODULE hUser32 = GetModuleHandleA(szUser32);
        HMODULE hWs2_32 = GetModuleHandleA(szWs2_32);

        HookLog("[LauncherDll] InlineSystemHook attach send=%p WSASend=%p recv=%p", send, WSASend, recv);

        // 建 trampoline 給 send
        if (!g_original_send)
        {
            BYTE original_bytes[5];
            memcpy(original_bytes, (void*)send, 5);

            HookLog("[LauncherDll] Original send bytes BEFORE hook at %p: %02X %02X %02X %02X %02X",
                send, original_bytes[0], original_bytes[1], original_bytes[2],
                original_bytes[3], original_bytes[4]);

            BYTE* trampoline = (BYTE*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (trampoline)
            {
                memcpy(trampoline, original_bytes, 5);
                trampoline[5] = 0xE9;

                BYTE* jmp_ins = trampoline + 5;
                BYTE* target = (BYTE*)send + 5;
                DWORD offset = (DWORD)(target - (jmp_ins + 5));
                *(DWORD*)(trampoline + 6) = offset;

                g_original_send = (PFN_SEND)trampoline;

                HookLog("[LauncherDll] Created trampoline at %p for send %p", trampoline, send);
                HookLog("[LauncherDll] JMP offset calculated: 0x%08X", offset);
            }
            else
                HookLog("[LauncherDll] ERROR: VirtualAlloc failed for trampoline!");
        }

        if (!HookIatApi(szKernel32, (DWORD)GetACP, (DWORD)my_GetACP, (void**)&real_GetACP))
            HookEatApi(hKernel32, (DWORD)GetACP, (DWORD)my_GetACP, (void**)&real_GetACP);

        InlineHook((ULONG32)connect, (ULONG32)my_connect, true);
        InlineHook((ULONG32)send, (ULONG32)my_send, true);
        InlineHook((ULONG32)WSASend, (ULONG32)my_WSASend, true);
        InlineHook((ULONG32)recv, (ULONG32)my_recv, true);
        InlineHook((ULONG32)CreateWindowExA, (ULONG32)my_CreateWindowEx, true);
    }
    else
    {
        HookLog("[LauncherDll] InlineSystemHook detach send=%p WSASend=%p recv=%p", send, WSASend, recv);
        InlineHook((ULONG32)connect, (ULONG32)my_connect, false);
        InlineHook((ULONG32)send, (ULONG32)my_send, false);
        InlineHook((ULONG32)WSASend, (ULONG32)my_WSASend, false);
        InlineHook((ULONG32)recv, (ULONG32)my_recv, false);
        InlineHook((ULONG32)CreateWindowExA, (ULONG32)my_CreateWindowEx, false);
        ResetCipherState();
        g_original_send = nullptr;
    }

    return true;
}

// ============================================================================
// BD 檔案讀取 / 解密 / 解壓
// ============================================================================
BYTE* GetFileBuffer()
{
    FILE* fp = NULL;
    DWORD len = 0;
    buffer_len = 0;

    const wchar_t* fallbackPaths[] = {
        L"./Login.pak",
        L"Login.pak",
        L"../Login.pak",
        NULL
    };

    const wchar_t* launcherPath = NULL;
    wchar_t sanitized[64] = { 0 };

    if (ShareInfo.bdfile[0] != L'\0')
    {
        wcsncpy_s(sanitized, _countof(sanitized), ShareInfo.bdfile, _TRUNCATE);

        // 去頭尾空白/引號
        wchar_t* start = sanitized;
        while (*start == L' ' || *start == L'\t' || *start == L'"')
            ++start;
        wchar_t* end = start + wcslen(start);
        while (end > start && (end[-1] == L' ' || end[-1] == L'\t' || end[-1] == L'"'))
            *--end = L'\0';

        if (*start != L'\0' && !(start[0] == L'.' && start[1] == L'\0'))
            launcherPath = start;
    }

    if (launcherPath)
    {
        HookLog("[LauncherDll] Trying BD file hinted by launcher: '%S'", launcherPath);
        if (_wfopen_s(&fp, launcherPath, L"rb") != 0)
            HookLog("[LauncherDll] Failed to open hinted BD file '%S'", launcherPath);
        else
            HookLog("[LauncherDll] Opened BD file from launcher hint '%S'", launcherPath);
    }

    if (!fp)
    {
        for (int i = 0; fallbackPaths[i] != NULL; i++)
        {
            if (_wfopen_s(&fp, fallbackPaths[i], L"rb") == 0)
            {
                HookLog("[LauncherDll] Opened BD file from '%S'", fallbackPaths[i]);
                break;
            }
        }
    }

    if (!fp)
    {
        HookLog("[LauncherDll] Failed to open BD file from any known location");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    BYTE* file_data = new BYTE[len];
    fread(file_data, 1, len, fp);
    fclose(fp);

    HookLog("[LauncherDll] Read %d bytes from BD file", len);

    DWORD un_len = *(DWORD*)file_data;
    HookLog("[LauncherDll] Expected uncompressed size from header: %d bytes", un_len);
    HookLog("[LauncherDll] Raw header bytes (first 32): "
        "%02X %02X %02X %02X %02X %02X %02X %02X "
        "%02X %02X %02X %02X %02X %02X %02X %02X "
        "%02X %02X %02X %02X %02X %02X %02X %02X "
        "%02X %02X %02X %02X %02X %02X %02X %02X",
        file_data[0], file_data[1], file_data[2], file_data[3],
        file_data[4], file_data[5], file_data[6], file_data[7],
        file_data[8], file_data[9], file_data[10], file_data[11],
        file_data[12], file_data[13], file_data[14], file_data[15],
        file_data[16], file_data[17], file_data[18], file_data[19],
        file_data[20], file_data[21], file_data[22], file_data[23],
        file_data[24], file_data[25], file_data[26], file_data[27],
        file_data[28], file_data[29], file_data[30], file_data[31]);

    DWORD header_len = 0;
    const BYTE* header_key_ptr = NULL;
    size_t header_key_len = 0;

    if (len > BD_HEADER_SIZE && memcmp(&file_data[sizeof(DWORD)], BD_FILE_SIGNATURE_BYTES, BD_FILE_SIGNATURE_SIZE) == 0)
    {
        header_len = BD_HEADER_SIZE;
        header_key_ptr = &file_data[sizeof(DWORD) + BD_FILE_SIGNATURE_SIZE];
        header_key_len = BD_HEADER_SIZE - sizeof(DWORD) - BD_FILE_SIGNATURE_SIZE;
        HookLog("[LauncherDll] New format detected (BDk1 signature)");
        HookLog("[LauncherDll] Header size: %d bytes", header_len);
    }
    else if (len > BD_LEGACY_HEADER_SIZE)
    {
        header_len = BD_LEGACY_HEADER_SIZE;
        header_key_ptr = &file_data[sizeof(DWORD)];
        header_key_len = BD_LEGACY_HEADER_SIZE - sizeof(DWORD);
        HookLog("[LauncherDll] Using legacy format (header_len=%d)", header_len);
    }
    else
    {
        HookLog("[LauncherDll] BD file too small (len=%d)", len);
        delete[] file_data;
        return NULL;
    }

    BYTE* payload = &file_data[header_len];
    DWORD payload_len = len - header_len;
    HookLog("[LauncherDll] Payload length=%u bytes (total file=%u, header=%u)", payload_len, len, header_len);

    if (payload_len == 0)
    {
        HookLog("[LauncherDll] Empty payload");
        delete[] file_data;
        return NULL;
    }

    BYTE* encrypted_backup = new BYTE[payload_len];
    memcpy(encrypted_backup, payload, payload_len);

    bool has_share_key = false;
    for (size_t i = 0; i < sizeof(ShareInfo.key); ++i)
    {
        if (ShareInfo.key[i] != 0)
        {
            has_share_key = true;
            HookLog("[LauncherDll] SHARE_INFO key detected (first 16 bytes): "
                "%02X %02X %02X %02X %02X %02X %02X %02X "
                "%02X %02X %02X %02X %02X %02X %02X %02X",
                ShareInfo.key[0], ShareInfo.key[1], ShareInfo.key[2], ShareInfo.key[3],
                ShareInfo.key[4], ShareInfo.key[5], ShareInfo.key[6], ShareInfo.key[7],
                ShareInfo.key[8], ShareInfo.key[9], ShareInfo.key[10], ShareInfo.key[11],
                ShareInfo.key[12], ShareInfo.key[13], ShareInfo.key[14], ShareInfo.key[15]);
            break;
        }
    }

    auto NormalizeKey = [](BYTE dst[32], const BYTE* src, size_t src_len)
        {
            memset(dst, 0, 32);
            if (!src || !src_len) return false;
            size_t cp = src_len >= 32 ? 32 : src_len;
            memcpy(dst, src, cp);
            if (cp < 32)
            {
                for (size_t i = cp; i < 32; ++i)
                    dst[i] = src[i % cp];
            }
            return true;
        };

    auto TryDecryptWithKey = [&](const char* tag, const BYTE* raw_key, size_t raw_len) -> bool
        {
            if (!raw_key || !raw_len) return false;
            BYTE key32[32];
            if (!NormalizeKey(key32, raw_key, raw_len)) return false;

            memcpy(payload, encrypted_backup, payload_len);

            HookLog("[LauncherDll] Trying %s key (raw_len=%zu)", tag, raw_len);
            HookLog("[LauncherDll] DEBUG: Before config_decrypt, payload first 16 bytes: "
                "%02X %02X %02X %02X %02X %02X %02X %02X "
                "%02X %02X %02X %02X %02X %02X %02X %02X",
                payload[0], payload[1], payload[2], payload[3],
                payload[4], payload[5], payload[6], payload[7],
                payload[8], payload[9], payload[10], payload[11],
                payload[12], payload[13], payload[14], payload[15]);

            config_decrypt(key32, payload, (int)payload_len);

            // 看起來像 zlib
            if (payload_len >= 2 && payload[0] == 0x78 &&
                (payload[1] == 0x9C || payload[1] == 0xDA || payload[1] == 0x01))
            {
                HookLog("[LauncherDll] Payload looks like valid zlib data after using %s key", tag);

                char md5[64] = { 0 };
                ComputeMd5Hex(payload, payload_len, md5, sizeof(md5));
                HookLog("[LauncherDll] Payload MD5 after %s key: %s", tag, md5);
                return true;
            }

            HookLog("[LauncherDll] Payload still invalid after using %s key (first bytes %02X %02X)", tag, payload[0], payload[1]);
            return false;
        };

    bool decrypted = false;
    if (header_key_ptr && header_key_len > 0)
        decrypted = TryDecryptWithKey("BD header", header_key_ptr, header_key_len);

    if (!decrypted && has_share_key)
        decrypted = TryDecryptWithKey("SHARE_INFO", ShareInfo.key, sizeof(ShareInfo.key));

    if (!decrypted)
        decrypted = TryDecryptWithKey("FILE_ENCRYPT_KEY",
            (const BYTE*)FILE_ENCRYPT_KEY,
            strlen(FILE_ENCRYPT_KEY));

    if (!decrypted)
    {
        HookLog("[LauncherDll] Failed to decrypt BD payload with all known keys");
        delete[] encrypted_backup;
        delete[] file_data;
        return NULL;
    }

    HookLog("[LauncherDll] Payload after decryption (first 32 bytes): "
        "%02X %02X %02X %02X %02X %02X %02X %02X "
        "%02X %02X %02X %02X %02X %02X %02X %02X "
        "%02X %02X %02X %02X %02X %02X %02X %02X "
        "%02X %02X %02X %02X %02X %02X %02X %02X",
        payload[0], payload[1], payload[2], payload[3],
        payload[4], payload[5], payload[6], payload[7],
        payload[8], payload[9], payload[10], payload[11],
        payload[12], payload[13], payload[14], payload[15],
        payload[16], payload[17], payload[18], payload[19],
        payload[20], payload[21], payload[22], payload[23],
        payload[24], payload[25], payload[26], payload[27],
        payload[28], payload[29], payload[30], payload[31]);

    delete[] encrypted_backup;

    BYTE* un_buffer = new BYTE[un_len + 1];
    HookLog("[LauncherDll] Allocated %d bytes for decompression", un_len + 1);
    int retz = uncompress(un_buffer, &un_len, payload, payload_len);
    un_buffer[un_len] = 0;

    delete[] file_data;

    if (retz == Z_OK)
    {
        HookLog("[LauncherDll] BD decompression successful, final size=%d", un_len);
        buffer_len = un_len;
        return un_buffer;
    }

    HookLog("[LauncherDll] BD decompression failed, error code=%d", retz);
    delete[] un_buffer;
    return NULL;
}

// ============================================================================
// init(): 模組初始化
// ============================================================================
void init()
{
    DetectGameVersion();

    SHARE_INFO* pShareInfo = get_shm(GetCurrentProcessId(), false);
    if (pShareInfo)
        memcpy(&ShareInfo, pShareInfo, sizeof(SHARE_INFO));

    ResetCipherState();

    g_respectLauncherEncrypt = (ShareInfo.encrypt != 0);
    g_forceRuntimeCipher = false;
    g_forceCipherSniff = false;

    if (!g_respectLauncherEncrypt && g_detectedVersionMajor >= 17)
    {
        g_forceRuntimeCipher = true;
        g_respectLauncherEncrypt = true;
        ShareInfo.encrypt = 1;
        HookLog("[LauncherDll] launcher沒開encrypt，但偵測到 %lu.%lu 版本 → 強制啟用封包加密", g_detectedVersionMajor, g_detectedVersionMinor);
        if (pShareInfo)
            pShareInfo->encrypt = 1;
    }
    else if (!g_respectLauncherEncrypt)
    {
        g_forceCipherSniff = true;
        HookLog("[LauncherDll] launcher沒開encrypt，改成只抓S_KEY不加密");
    }

    g_dropLoginIfNoUi = g_respectLauncherEncrypt && !g_forceRuntimeCipher;
    if (!g_dropLoginIfNoUi)
        HookLog("[LauncherDll] Login packet drop guard disabled (auto-encrypt mode)");

    HookLog("[LauncherDll] Launcher encrypt setting=%d", g_respectLauncherEncrypt ? 1 : 0);

#ifdef _UNICODE
    if (ShareInfo.usebd)
    {
        char bdAnsi2[128] = { 0 };
        WideCharToMultiByte(CP_UTF8, 0, ShareInfo.bdfile, -1, bdAnsi2, sizeof(bdAnsi2) - 1, nullptr, nullptr);
        HookLog("[LauncherDll] bdfile=%s", bdAnsi2);
    }
    else
        HookLog("[LauncherDll] bdfile (not used)");
#else
    if (ShareInfo.usebd)
        HookLog("[LauncherDll] bdfile=%s", ShareInfo.bdfile);
    else
        HookLog("[LauncherDll] bdfile (not used)");
#endif

    // RSA 初始化
    if (g_respectLauncherEncrypt && pShareInfo)
    {
        bn_ctx = BN_CTX_new();
        d = BN_new();
        n = BN_new();

        BN_set_word(d, pShareInfo->RSA_D ^ 32345678);
        BN_set_word(n, pShareInfo->RSA_N ^ 22345678);
        __dbg_print("d: %s, n: %s, randenc: %d",
            BN_bn2dec(d), BN_bn2dec(n), pShareInfo->randenc);
    }
    else if (g_respectLauncherEncrypt && !pShareInfo)
    {
        HookLog("[LauncherDll] WARNING: encrypt requested but SHARE_INFO unavailable; skipping RSA init");
    }

    // 通知啟動器
    if (pShareInfo)
    {
        pShareInfo->bIsFinish = true;
        HookLog("[LauncherDll] Set bIsFinish=true, launcher can exit now");
    }

    free_shm();

    HookLog("[LauncherDll] usebd=%d (from launcher)", ShareInfo.usebd ? 1 : 0);

    // 實際載入 BD
    if (ShareInfo.usebd)
    {
        buffer = GetFileBuffer();
        if (!buffer || buffer_len == 0)
        {
            HookLog("[LauncherDll] BD load failed, but continuing with usebd=1");
            buffer = NULL;
            buffer_len = 0;
        }
        else
        {
            HookLog("[LauncherDll] BD buffer loaded, size=%u bytes.", buffer_len);

            FILE* dumpFile = fopen("bd_dump.bin", "wb");
            if (dumpFile)
            {
                size_t written = fwrite(buffer, 1, buffer_len, dumpFile);
                fclose(dumpFile);
                HookLog("[LauncherDll] BD buffer dumped to bd_dump.bin (%zu bytes written)", written);
            }
        }
    }
    else
    {
        buffer = NULL;
        buffer_len = 0;
    }
    g_bdHooksApplied = false;

    encdec_init_key(ShareInfo.key);
    encdec_set_key16(ShareInfo.key);

    InstallKeyboardHook();

    InlineSystemHook(true);
}

// ============================================================================
// 遊戲原本呼叫的 BD 讀取點
// ============================================================================
NAKED void GetFileData(void)
{
    __asm
    {
        mov eax, buffer_len
        mov[ebp - 0x22C], eax
        mov eax, buffer
        add eax, 0x1
        mov edx, [ebp - 0x20C]
        mov[edx + 0x8], eax
        mov eax, 0x097ABDF
        jmp eax
    }
}

// ============================================================================
// (舊版) 抓帳號 / 密碼 (只在舊版 client 上掛)
// ============================================================================
const char* pUserName = "cssddddddd";

NAKED void GetUserNameS(void)
{
    __asm
    {
        lea edx, [ebp - 0x94]
        push edx

        lea ecx, [ebp - 0x80]
        push ecx
        mov  ecx, [ebp - 0xB0]
        mov  eax, 0xA544F0
        call eax

        lea  edx, [ebp - 0x80]
        pop  eax
        push 0x1F
        push 0
        push 0
        push 0
        push 0
        push 0
        push 0x7F
        push edx
        push eax
        push 0x3E
        push pUserName
        mov  eax, 0x966060
        call eax
        add  esp, 0x2C
        mov  eax, 0xD8A071
        jmp  eax
    }
}

UINT dwCurHP;
UINT dwMuseID;
const char* pPassword = "( -\\\\fRf3%d\\\\fRf> )\\\\fRf=%d";
char szPassword[MAX_PATH] = { 0 };

void WINAPI PrintfAttack(char* pTextBuff, UINT32 nAttack1, UINT32 nAttack2)
{
    wsprintfA(pTextBuff, pPassword, nAttack1, nAttack2);
}

NAKED void GetPasswordS(void)
{
    __asm
    {
        mov[ebp - 0x110], edx
        movzx ecx, word ptr[ebp - 0x2B0]
        mov[ebp - 0x10C], ecx
        mov  eax, [ebp - 0x110]
        cmp  eax, [dwMuseID]
        jz   loc_zero
        mov[dwCurHP], 0
        loc_zero:
        mov  ecx, [dwCurHP]
            add  ecx, [ebp - 0x10C]
            mov[dwCurHP], ecx
            mov  edx, [ebp - 0x110]
            mov[dwMuseID], edx
            mov  eax, [dwCurHP]
            push eax
            mov  ecx, [ebp - 0x10C]
            push ecx
            push offset szPassword

            call PrintfAttack

            pushad
            push 0
            push 0
            push 0
            push 8
            push 0xFFDF
            lea  ecx, szPassword
            push ecx
            mov  edx, dwMuseID
            push edx
            mov  ebx, 0x74E280
            call ebx
            add  esp, 0x1C
            popad

            mov  eax, 0x8AB399
            jmp  eax
    }
}

const char* pPassword2 = "( -\\\\fRf3%d\\\\fRf> )";
char szPassword2[MAX_PATH] = { 0 };

NAKED void GetPasswordT(void)
{
    memset(szPassword2, 0, MAX_PATH);

    __asm
    {
        mov[ebp - 0x110], ecx
        mov  ecx, [ebp - 0x1B4]
        mov  edx, [ecx + 4]
        mov  eax, [ebp - 0x1C0]
        mov  ecx, [edx + eax * 4]
        mov[ebp - 0x10C], ecx
        mov  eax, [ebp - 0x110]
        push eax
        mov  ecx, pPassword2
        push ecx
        push 0x104
        lea  edx, szPassword2
        push edx
        call PrintfAttack

        pushad
        push 0
        push 0
        push 0
        push 8
        push 0xFFDF
        lea  ecx, szPassword2
        push ecx
        mov  edx, [ebp - 0x10C]
        push edx
        mov  ebx, 0x74E280
        call ebx
        add  esp, 0x1C
        popad

        mov eax, 0x8A9D9E
        jmp eax
    }
}

// ============================================================================
// 遊戲切換場景要解除 hook
// ============================================================================
NAKED void ChangeHook(void)
{
    __asm
    {
        pushad
        call UnLoadHelp
        popad

        mov ebx, 0x0FEF9BC
        call ebx

        mov eax, 0x8F025A
        jmp eax
    }
}

// ============================================================================
// my_GetACP → 一開遊戲真正跑到遊戲程式碼內時才掛舊 hook / BD hook
// ============================================================================
UINT WINAPI my_GetACP(void)
{
    DWORD dwFunc = NULL;
    __asm
    {
        mov eax, [ebp + 04h]
        mov dwFunc, eax
    }

    DWORD dwBase = (DWORD)GetModuleHandle(NULL);
    PIMAGE_DOS_HEADER pidh = (PIMAGE_DOS_HEADER)dwBase;
    PIMAGE_NT_HEADERS pinh = (PIMAGE_NT_HEADERS)(dwBase + pidh->e_lfanew);

    DWORD codeBase = dwBase + pinh->OptionalHeader.BaseOfCode;
    DWORD codeEnd = codeBase + pinh->OptionalHeader.SizeOfCode;

    if ((dwFunc >= codeBase && dwFunc < codeEnd) && !bInitialize)
    {
        bInitialize = TRUE;

        // 舊版才掛舊的帳號/密碼截取
        if (g_legacyUiHooksEnabled)
        {
            TryInlineHookSafe(0x0D89E32, (ULONG32)GetUserNameS, 6, "GetUserNameS");
            TryInlineHookSafe(0x08AB26B, (ULONG32)GetPasswordS, 6, "GetPasswordS");
            TryInlineHookSafe(0x08A9D73, (ULONG32)GetPasswordT, 6, "GetPasswordT");
        }
        else
        {
            HookLog("[LauncherDll] Skipping legacy inline hooks (GetUserNameS/GetPasswordS/GetPasswordT)");
        }

        // BD hook
        if (!g_bdHooksApplied)
        {
            if (ShareInfo.usebd)
            {
                if (buffer && buffer_len)
                {
                    HookLog("[LauncherDll] applying BD hooks with data (len=%u).", buffer_len);
                    const char code2[2] = { 0x90, 0xE9 };
                    if (TryPatchMemorySafe(0x0835A70, code2, sizeof(code2), "BD runtime check (with data)"))
                        TryInlineHookSafe(0x097AB00, (ULONG32)GetFileData, 5, "GetFileData");
                }
                else
                {
                    HookLog("[LauncherDll] WARNING: No BD data, but trying to patch runtime check anyway");
                    const char code2[2] = { 0x90, 0xE9 };
                    if (TryPatchMemorySafe(0x0835A70, code2, sizeof(code2), "BD runtime check (no data)"))
                        HookLog("[LauncherDll] Patched runtime check at 0x835A70");
                }
                g_bdHooksApplied = true;
            }
            else
            {
                HookLog("[LauncherDll] skip BD hooks (usebd=%d buffer=%p len=%u).",
                    ShareInfo.usebd ? 1 : 0, buffer, buffer_len);
            }
        }
    }

    return real_GetACP();
}

// ============================================================================
// 基本 Inline Hook 實作
// ============================================================================
void InlineHook(ULONG32 ulOldAddr, ULONG32 ulHookAddr, bool bIsHook /*= true*/)
{
    DWORD  OrigProtect = 0;
    char   uszHookCode[5] = { 0xE9, 0,0,0,0 };
    char   uszOrigCode[5] = { 0x8B, 0xFF, 0x55, 0x8B, 0xEC };

    VirtualProtect((LPVOID)ulOldAddr, 5, PAGE_READWRITE, &OrigProtect);
    if (bIsHook)
    {
        (*(PULONG32)&uszHookCode[1]) = ulHookAddr - ulOldAddr - 5;
        CopyMemory((void*)ulOldAddr, uszHookCode, 5);
    }
    else
        CopyMemory((void*)ulOldAddr, uszOrigCode, 5);

    VirtualProtect((LPVOID)ulOldAddr, 5, OrigProtect, &OrigProtect);
}

void InlineHookEx(ULONG32 ulOldAddr, ULONG32 ulHookAddr, int nInlineLen)
{
    DWORD OrigProtect = 0;
    char  uszHookCode[9] = { 0xE9,0,0,0,0, 0x90,0x90,0x90,0x90 };
    VirtualProtect((LPVOID)ulOldAddr, nInlineLen, PAGE_READWRITE, &OrigProtect);

    (*(PULONG32)&uszHookCode[1]) = ulHookAddr - ulOldAddr - 5;
    CopyMemory((void*)ulOldAddr, uszHookCode, nInlineLen);

    VirtualProtect((LPVOID)ulOldAddr, nInlineLen, OrigProtect, &OrigProtect);
}

void PatachHook(ULONG32 ulOldAddr, char* pathcode, int npathlen)
{
    DWORD oldProtect;
    VirtualProtect((LPVOID)ulOldAddr, npathlen, PAGE_READWRITE, &oldProtect);
    CopyMemory((void*)ulOldAddr, pathcode, npathlen);
    VirtualProtect((LPVOID)ulOldAddr, npathlen, oldProtect, &oldProtect);
}

// ============================================================================
// 導出
// ============================================================================
int WINAPI DLLGetVersion()
{
    return 0x1001;
}

char* WINAPI DLLGetInformation()
{
    return (char*)"Lin LauncherDll (patched no-UI-login-block)";
}
