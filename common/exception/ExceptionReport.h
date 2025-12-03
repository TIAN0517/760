#ifndef MDUMP_H_ 
#define MDUMP_H_ 

#include <windows.h> 
#include <tchar.h> 

#pragma warning(disable:4996)

struct _EXCEPTION_POINTERS; 

class CMiniDumper 
{ 
public: 
	static void Enable(LPCTSTR pszAppName, bool bShowErrors); 

private: 
	static TCHAR m_szAppName[MAX_PATH]; 

	static HMODULE GetDebugHelperDll(FARPROC* ppfnMiniDumpWriteDump, bool bShowErrors); 
	static LONG WINAPI TopLevelFilter(struct _EXCEPTION_POINTERS* pExceptionInfo); 
}; 

extern CMiniDumper theCrashDumper; 

#endif//MDUMP_H_