#pragma once

#pragma once
#ifdef _DLL_EXPORT
#define DLL_FUNCTION	extern "C" __declspec(dllexport)
#else
#define DLL_FUNCTION	extern "C" __declspec(dllimport)
#endif

//¶}±ÒHOOK
DLL_FUNCTION VOID StartHook(CHAR szClassName[MAX_PATH]);
DLL_FUNCTION VOID StopHook();
DLL_FUNCTION bool	  Loader(LPVOID lpParam);