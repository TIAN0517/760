// stdafx.h
#pragma once

// ① Winsock2 一定要最前面！一定要最前面！一定要最前面！
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// 先吃新版 winsock
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#pragma comment(lib, "ws2_32.lib")

// ② 告訴後面的人：我已經吃過 winsock 了，不要再塞 winsock.h 給我
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

// ③ 這裡才吃 MFC / Windows
#include <afxwin.h>
#include <afxext.h>

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>
#include <afxodlgs.h>
#include <afxdisp.h>
#endif

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>
#endif

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>
#endif

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>
#endif

#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"Psapi.lib")
