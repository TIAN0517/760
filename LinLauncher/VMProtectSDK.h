#pragma once

// VMProtect SDK 標頭檔案
// 這是一個基本的 VMProtect SDK 介面定義

#ifdef __cplusplus
extern "C" {
#endif

// VMProtect 基本函數定義
typedef void* VMProtectHandle;

// VMProtect 版本資訊
typedef struct {
    unsigned int dwMajor;
    unsigned int dwMinor;
    unsigned int dwBuild;
    unsigned int dwRevision;
} VMProtectVersion;

// VMProtect 授權資訊
typedef struct {
    unsigned int dwState;
    unsigned int dwDaysLeft;
    unsigned int dwRunningTimeLeft;
    unsigned int dwMaxBuild;
    unsigned char bUserData[256];
} VMProtectLicense;

// VMProtect 函數宣告
VMProtectHandle VMProtectBegin(const char* szName);
void VMProtectEnd(VMProtectHandle handle);
int VMProtectIsDebuggerPresent(int bCheckKernel);
int VMProtectIsVirtualMachinePresent();
int VMProtectIsValidImageCRC();
int VMProtectDecryptStringA(const char* szValue);
int VMProtectDecryptStringW(const wchar_t* szValue);
int VMProtectFreeString(const char* szValue);
int VMProtectFreeStringW(const wchar_t* szValue);
int VMProtectGetCurrentHWID(char* szHWID, int nSize);
int VMProtectActivateLicense(const char* szUserName, const char* szEMail, const char* szSerialNumber);
int VMProtectDeactivateLicense(const char* szUserName, const char* szSerialNumber);
int VMProtectGetOfflineActivationString(const char* szUserName, const char* szEMail, const char* szSerialNumber, char* szBuffer, int nSize);
int VMProtectGetOfflineDeactivationString(const char* szUserName, const char* szSerialNumber, char* szBuffer, int nSize);
int VMProtectGetLicenseInfo(const char* szUserName, const char* szSerialNumber, VMProtectLicense* pLicense);
int VMProtectGetVersion(VMProtectVersion* pVersion);

// 巨集定義
#define VMProtectBeginVirtualization(name) VMProtectBegin(name)
#define VMProtectBeginVirtualizationLockByKey(name, key) VMProtectBegin(name)
#define VMProtectBeginUltra(name) VMProtectBegin(name)
#define VMProtectBeginUltraLockByKey(name, key) VMProtectBegin(name)
#define VMProtectBeginMutation(name) VMProtectBegin(name)
#define VMProtectBeginMutationLockByKey(name, key) VMProtectBegin(name)

#ifdef __cplusplus
}
#endif
