// 臨時安全模式 Hook 包裝器
// 將此程式碼添加到 Hook 相關檔案中

#ifndef SAFE_HOOK_WRAPPER_H
#define SAFE_HOOK_WRAPPER_H

#include <windows.h>
#include <excpt.h>

// 安全的 Hook 應用函數
BOOL SafeApplyBDHooks(LPVOID data, DWORD size) {
    __try {
        // 記憶體保護檢查
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(data, &mbi, sizeof(mbi)) == 0) {
            OutputDebugStringA("[SAFE_HOOK] Memory query failed\n");
            return FALSE;
        }

        // 確保記憶體可寫
        DWORD oldProtect;
        if (!VirtualProtect(data, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            OutputDebugStringA("[SAFE_HOOK] Memory protection failed\n");
            return FALSE;
        }

        // 分段應用 Hook (每次最多 1MB)
        const DWORD MAX_CHUNK = 1024 * 1024; // 1MB
        LPBYTE ptr = (LPBYTE)data;
        DWORD remaining = size;

        while (remaining > 0) {
            DWORD chunkSize = min(remaining, MAX_CHUNK);
            
            // 這裡插入原有的 Hook 邏輯
            // apply_hooks_chunk(ptr, chunkSize);
            
            ptr += chunkSize;
            remaining -= chunkSize;

            // 讓出 CPU 時間，避免系統卡死
            Sleep(1);
        }

        // 恢復原始記憶體保護
        VirtualProtect(data, size, oldProtect, &oldProtect);
        
        OutputDebugStringA("[SAFE_HOOK] All hooks applied successfully\n");
        return TRUE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        char errMsg[256];
        sprintf_s(errMsg, sizeof(errMsg), 
            "[SAFE_HOOK] Exception caught: 0x%08X\n", 
            GetExceptionCode());
        OutputDebugStringA(errMsg);
        return FALSE;
    }
}

// 記錄詳細錯誤的版本
BOOL SafeApplyBDHooksWithLogging(LPVOID data, DWORD size, const char* logFile) {
    FILE* log = NULL;
    if (logFile) {
        fopen_s(&log, logFile, "a");
        if (log) {
            fprintf(log, "[%s] Starting hook application, size=%u\n", 
                __TIMESTAMP__, size);
            fflush(log);
        }
    }

    BOOL result = SafeApplyBDHooks(data, size);

    if (log) {
        fprintf(log, "[%s] Hook application %s\n", 
            __TIMESTAMP__, result ? "SUCCESS" : "FAILED");
        fclose(log);
    }

    return result;
}

#endif // SAFE_HOOK_WRAPPER_H