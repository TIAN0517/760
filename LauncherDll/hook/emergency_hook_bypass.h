/*
 * 緊急 Hook 繞過測試版本
 * 用於診斷是否為 Hook 機制問題
 * 
 * 使用方法：
 * 1. 備份原始 Hook 相關檔案
 * 2. 使用此版本測試基本連線
 * 3. 如果成功，問題確定在 Hook 機制
 */

#ifndef EMERGENCY_HOOK_BYPASS_H
#define EMERGENCY_HOOK_BYPASS_H

#include <windows.h>
#include <stdio.h>

// 緊急模式 - 最小化 Hook 應用
BOOL EmergencyApplyMinimalHooks(LPVOID data, DWORD size) {
    // 記錄日誌但不實際應用 Hook
    char logMsg[256];
    sprintf_s(logMsg, sizeof(logMsg), 
        "[EMERGENCY] Received data size: %u bytes\n", size);
    OutputDebugStringA(logMsg);
    
    // 暫時跳過所有 Hook，直接返回成功
    OutputDebugStringA("[EMERGENCY] Skipping all hooks - TEST MODE\n");
    return TRUE;
}

// 段階式測試 Hook
BOOL TestHooksGradually(LPVOID data, DWORD size) {
    OutputDebugStringA("[TEST] Starting gradual hook test\n");
    
    // 測試 1: 僅記憶體檢查
    if (!data || size == 0) {
        OutputDebugStringA("[TEST] Invalid parameters\n");
        return FALSE;
    }
    
    // 測試 2: 記憶體存取測試
    __try {
        BYTE testByte = ((LPBYTE)data)[0];
        char msg[128];
        sprintf_s(msg, sizeof(msg), "[TEST] First byte: 0x%02X\n", testByte);
        OutputDebugStringA(msg);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        OutputDebugStringA("[TEST] Memory access failed\n");
        return FALSE;
    }
    
    // 測試 3: 小量資料處理 (前 1KB)
    DWORD testSize = min(size, 1024);
    __try {
        for (DWORD i = 0; i < testSize; i += 256) {
            volatile BYTE b = ((LPBYTE)data)[i];
            (void)b; // 避免編譯器最佳化
        }
        OutputDebugStringA("[TEST] 1KB access test passed\n");
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        OutputDebugStringA("[TEST] 1KB access test failed\n");
        return FALSE;
    }
    
    OutputDebugStringA("[TEST] All gradual tests passed\n");
    return TRUE;
}

// 詳細記錄版本
BOOL VerboseHookApplication(LPVOID data, DWORD size) {
    FILE* logFile = NULL;
    fopen_s(&logFile, "hook_debug.log", "a");
    
    if (logFile) {
        fprintf(logFile, "=== Hook Application Start ===\n");
        fprintf(logFile, "Time: %s", __TIMESTAMP__);
        fprintf(logFile, "Data pointer: %p\n", data);
        fprintf(logFile, "Data size: %u bytes\n", size);
        fflush(logFile);
    }
    
    BOOL result = FALSE;
    
    __try {
        // 這裡可以插入真正的 Hook 邏輯
        // 目前只做基本檢查
        
        if (logFile) {
            fprintf(logFile, "Basic checks passed\n");
            fflush(logFile);
        }
        
        // 模擬 Hook 處理時間
        Sleep(100);
        
        result = TRUE;
        
        if (logFile) {
            fprintf(logFile, "Hook application completed successfully\n");
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        DWORD exceptionCode = GetExceptionCode();
        
        if (logFile) {
            fprintf(logFile, "EXCEPTION CAUGHT: 0x%08X\n", exceptionCode);
            
            switch (exceptionCode) {
                case EXCEPTION_ACCESS_VIOLATION:
                    fprintf(logFile, "Access Violation - Invalid memory access\n");
                    break;
                case EXCEPTION_STACK_OVERFLOW:
                    fprintf(logFile, "Stack Overflow\n");
                    break;
                case EXCEPTION_INT_DIVIDE_BY_ZERO:
                    fprintf(logFile, "Division by zero\n");
                    break;
                default:
                    fprintf(logFile, "Unknown exception\n");
                    break;
            }
        }
        
        result = FALSE;
    }
    
    if (logFile) {
        fprintf(logFile, "=== Hook Application End (Result: %s) ===\n\n", 
                result ? "SUCCESS" : "FAILED");
        fclose(logFile);
    }
    
    return result;
}

// 主要緊急函數 - 替換原有的 apply_bd_hooks
BOOL EmergencyBDHookHandler(LPVOID data, DWORD size) {
    OutputDebugStringA("[EMERGENCY] Using emergency hook handler\n");
    
    // 選擇測試模式：
    // 1. 完全跳過 Hook
    // return EmergencyApplyMinimalHooks(data, size);
    
    // 2. 段階式測試
    // return TestHooksGradually(data, size);
    
    // 3. 詳細記錄模式
    return VerboseHookApplication(data, size);
}

#endif // EMERGENCY_HOOK_BYPASS_H