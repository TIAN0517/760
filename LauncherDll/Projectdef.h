#pragma once

#include <Windows.h>

enum TAB_CHILD_WINDOW
{
	CHILD_WINDOW_GENERIC,		//常規
	CHILD_WINDOW_STATE,			//藥水
	CHILD_WINDOW_STATUS,		//狀態
	CHILD_WINDOW_CHANGE,		//變身
	CHILD_WINDOW_ITEMS,			//物品
	CHILD_WINDOW_SPEAK,		//喊話
	CHILD_WINDOW_HOTKEY,		//熱鍵
};

//定時器定義
enum WINDOW_TIMERID
{
	TIMERID_GENERIC = WM_TIMER + WM_USER + 100,		//常規狀態下的定時器
	TIMERID_STATE,																//喝水窗口的定時器
	TIMERID_STATUS,															//狀態窗口的定時器
	TIMERID_CHANGE,															//變身窗口的定時器
	TIMERID_ITEMS,																//物品窗口的定時器
	TIMERID_SPEAK,																//喊話窗口的定時器
};

//用於更新提示消息
#define WM_USER_UPDATE			(WM_USER + 250)
#define WPARAM_OK				1
#define WPARAM_CANCEL			0
#define WPARAM_ERROR			-1


//用戶自定義時間
#define USER_STATE_TIME				(WM_USER + 123)
#define USER_ITEMS_TIME				(WM_USER + 124)
#define USER_SPEAK_TIME				(WM_USER + 130)

//這個宏 sum pos
#define __CurrentPos(c,t)			((float)c/((float)t/100))

//這個宏定義
#define MAX_COMBOX_LEN				0x64

//定義我們主進程的文件大小
#define MAIN_PROCESS_SIZE			5914536



//物品欄界面定義
//封包列表類
enum LIST_ITEMS_INFOMATION
{
	LIST_ITEMS_INDEX,				//下標
	LIST_ITEMS_NUMBER,				//數量
	LIST_ITEMS_NAME,				//名字
};

//物品列表信息
typedef struct _ITEMSINFO
{
	UINT	uItemNumber;						//物品的number
	PCHAR	pszItemName;							//物品的name
}ITEMSINFO, *PITEMSINFO;


#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <Windows.h>

//加入命名空間
namespace gameglobalparam
{
	//這裡存放我們遊戲版本定義的數據

	//文件鉤子地址
	const DWORD FILE_HOOK_ADDR = 0x0058788B;
	const DWORD FILE_RETN_ADDR = 0x0058794F;
	//帳號鉤子地址
	const DWORD USER_HOOK_ADDR = 0x0077317D;
	const DWORD USER_RETN_ADDR = 0x00773183;
	//密碼鉤子地址
	const DWORD PASS_HOOK_ADDR = 0x004AA38E;
	const DWORD PASS_RETN_ADDR = 0x004AA395;
	const DWORD PASS_CALL_ADDR = 0x00402800;
	//uid鉤子地址
	const DWORD SETID_HOOK_ADDR = 0x00772BA3;
	const DWORD SETID_RETN_ADDR = 0x00772BAD;
}

//寫入到dll中的數據
#pragma pack(push, 1)

typedef struct _SERVERINFO
{
	CHAR name[32];
	char ip[32];
	int port;
	bool used;
	BYTE key[16];
	bool encrypt;
	bool usehelper;
	bool usebd;
	CHAR bdfile[32];
	bool randkey;
	unsigned long e;
	unsigned long d;
	unsigned long n;
	BYTE fix[16];
}SERVERINFO, *PSERVERINFO;

typedef struct _DLL_CONFIG
{
	ULONG32		  m_myGameId;			//遊戲ID，防止讀取錯誤
	SERVERINFO m_myServer;			//定位於我們服務器的參數對象
	CHAR	szLink1[128];						//修正我們訪問的http link 連接
	CHAR	szLink2[128];
}DLLCONFIG, *PDLLCONFIG;
#pragma pack(pop)