#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 從 Windows 標頭排除不常使用的項目

#define UILIB_COMDAT __declspec(selectany)

#ifndef WINVER
#define WINVER 0x0600
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0700
#endif

// #include <AFXPRIV.H>
#include <atlbase.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <OleCtl.h>


#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stddef.h>
#include <time.h>

#include <algorithm>
#include <objbase.h>
#include <vector>
#include <string>
#include <map>
#include <commctrl.h>
#include <shlwapi.h>
#include <list>
#include <Commdlg.h>
#include <shellapi.h>
#include <wininet.h>
#include <MsHtmHst.h>
#include <ExDisp.h>
#include <ExDispid.h>
#include <richedit.h>


#include "../duilib/WebBrowserEventHandler.h"
#include "../Compress/XUnzip.h"
#include "../duilib/Utils.h"
#include "../duilib/UIDelegate.h"
#include "../duilib/UIDefine.h"
#include "../duilib/UIManager.h"
#include "../duilib/UIBase.h"
#include "../duilib/UIControl.h"
#include "../duilib/UIContainer.h"
#include "../duilib/UIMarkup.h"
#include "../duilib/UIDlgBuilder.h"
#include "../duilib/UIRender.h"
#include "../duilib/WinImplBase.h"

#include "../duilib/UIVerticalLayout.h"
#include "../duilib/UIHorizontalLayout.h"
#include "../duilib/UITileLayout.h"
#include "../duilib/UITabLayout.h"
#include "../duilib/UIChildLayout.h"

#include "../duilib/UIList.h"
#include "../duilib/UICombo.h"
#include "../duilib/UIScrollBar.h"
#include "../duilib/UITreeView.h"

#include "../duilib/UILabel.h"
#include "../duilib/UIText.h"
#include "../duilib/UIEdit.h"

#include "../duilib/UIButton.h"
#include "../duilib/UIOption.h"
#include "../duilib/UICheckBox.h"

#include "../duilib/UIProgress.h"
#include "../duilib/UISlider.h"

#include "../duilib/UIComboBox.h"
#include "../duilib/UIRichEdit.h"
#include "../duilib/UIDateTime.h"

#include "../duilib/UIActiveX.h"
#include "../duilib/UIWebBrowser.h"

using namespace DuiLib;
using namespace std;

#include "../Compress/XUnzip.h"
#include "../Crypto/Crypto.h"
#include "../Crypto/base64.h"
#include "../Crypto/des.h"
#include "../Crypto/rc6.h"
#include "../Crypto/configenc.h"
#include "../Hash/md5.h"
#include "../Share/ShareMemory.h"
#include "../Share/LResource.h"
#include "../WinLicenseSDK/WinLicenseSDK.h"
#include "LinLauncher.h"


#pragma comment(lib,"oledlg.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "wininet.lib")

#define lengthof(x) (sizeof(x)/sizeof(*x))
#define CLAMP(x,a,b) (min(b,max(a,x)))