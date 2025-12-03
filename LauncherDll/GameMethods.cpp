#include "StdAfx.h"
#include "GameMethods.h"

//顯示白天
void SwitchAllDay(bool bIsShow)
{
	char szHook[2] = { 0x7D, 0xEB };
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)0xDA54D5, (LPVOID)&szHook[bIsShow], 1u, 0);
}

//cpu hook
void SwitchCPU(bool bIsLow)
{
	WORD szHook[2] = { 0x06EB, 0x9090 };
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)0x836DF5, (LPVOID)&szHook[bIsLow], 2u, 0);
}

//海底抽水
void SwitchWater(bool bIsWater /*= true*/)
{
	char szHook[2] = { 0x74, 0xEB };
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)0xDAEB78, (LPVOID)&szHook[bIsWater], 1u, 0);
}

//怪物等級色彩
void SwitchMonsterLevelColor(bool bIsShow/* = true*/)
{
	char szOldCode[6] = { 0x0F, 0x85, 0xAB, 0x02, 0x00, 0x00 };
	char szHookCode[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

	if (bIsShow)
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)0xD9EBDF, (LPVOID)&szHookCode, 6u, 0);
	else
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)0xD9EBDF, (LPVOID)&szOldCode, 6u, 0);
}

//顯示天堂時間
void SwitchGameTimer(bool bIsShow/* = true*/)
{
	char szOldCode[6] = { 0x0F, 0x84, 0xB2, 0x00, 0x00, 0x00 };
	char szHookCode[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

	if (bIsShow)
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)0xD76D7D, (LPVOID)&szHookCode, 6u, 0);
	else
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)0xD76D7D, (LPVOID)&szOldCode, 6u, 0);
}

//顯示傷害值
void SwitchAttackLevelColor(bool bIsShow/* = true*/)
{
	char szOldCode[6] = { 0x0F, 0x84, 0x2E, 0x01, 0x00, 0x00 };
	char szHookCode[6] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

	if (bIsShow)
	{
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)0x8AB265, (LPVOID)&szHookCode, 6u, 0);
		*(PWORD)0x8A9D5E = 0x9090;
	}
	else
	{
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)0x8AB265, (LPVOID)&szOldCode, 6u, 0);
		*(PWORD)0x8A9D5E = 0x3E74;
	}

}

//解除右鍵鎖定
void SwitchRButtonLock(bool bIsNoLock/* = true*/)
{
	char szHook[2] = { 0x74, 0xEB };
	WriteProcessMemory(GetCurrentProcess(), (LPVOID)0xC9A576, (LPVOID)&szHook[bIsNoLock], 1u, 0);
}

//顯示經驗
void SwitchEXPLevelColor(bool bIsShow/*  = true*/)
{
	static char szText[MAX_PATH] = { 0 };

	static UINT32 dwAllLevelExpArray[] = { 29931, 0, 125, 300, 500, 750, 1296, 2401, 4096, 6561
		, 10000, 14641, 20736, 28561, 38416, 50625, 65536, 83521
		, 104976, 130321, 160000, 194481, 234256, 279841, 331776
		, 390625, 456976, 531441, 614656, 707281, 810000, 923521
		, 1048576, 1185921, 1336336, 1500625, 1679616, 1874161
		, 2085136, 2313441, 2560000, 2825761, 3111696, 3418801
		, 3748096, 4100625, 4829985, 6338401, 9833664, 19745853
		, 31292598, 44473900, 59289759, 75740173, 93825145, 113544672
		, 134898756, 157887397, 182510594, 208768347, 236660657
		, 266187523, 297348946, 330144925 };

	//上一次經驗值
	static UINT32 dwLastExp = 0;
	//static char* pstrExpBuffer =  (char*)GAME_SHOW_BUFFERADDR;
	static char* pstrExpBuffer = szText;
	char* ptest1 = NULL;
	char* ptest2 = NULL;

	//獲取角色當前經驗
	UINT32 unCurrentExp = GetCurrentRoleExp();
	UINT32 unCurrentLevel = 0;

	//如果獲取經驗沒變動，直接返回不輸出
	if (unCurrentExp == dwLastExp)
		return;

	//清理臨時緩衝區
	memset(pstrExpBuffer, 0, MAX_PATH);
	if (unCurrentExp <= dwLastExp)
	{
		ptest1 = "\\d2";
		ptest2 = "\\aG";
	}
	else
	{
		ptest1 = "\\d6";
		ptest1 = "\\aD";
	}


	//獲取當前人物等級
	unCurrentLevel = GetCurrentRoleLevel();
	UINT32 unCurrentTalExp = 0;
	UINT32 unToNextTalExp = 0;

	//如果當前的人物等級小於65級
	if (unCurrentLevel <= 65)
	{
		if (unCurrentLevel <= 0)
		{
			unCurrentTalExp = 0;
			unToNextTalExp = 0;
		}
		else
		{
			unCurrentTalExp = dwAllLevelExpArray[unCurrentLevel];
			unToNextTalExp = dwAllLevelExpArray[unCurrentLevel + 1] - dwAllLevelExpArray[unCurrentLevel];
		}

	}
	else
	{
		//超過65級經驗的計算方法
		unCurrentTalExp = 0x2264F44 * (unCurrentLevel - 65) + 0x17E14A29;
		unToNextTalExp = 0x2264F44;
	}

	//格式化字符串
	wsprintfA(pstrExpBuffer, _T("%s%s%s%s %d / %d / %d / %d"), _T("\\aD"), \
		_T(" 總需經驗/累計經驗/當前經驗/已得經驗 \r\n"), \
		ptest1, ptest2, \
		unToNextTalExp, \
		unCurrentExp - unCurrentTalExp, \
		unCurrentExp,
		unCurrentExp - dwLastExp
		);

	Game_ShowMessage(pstrExpBuffer);
	dwLastExp = unCurrentExp;

	// 	if(unCurrentExp != dwLastExp)
	// 	{
	// 		memset(pstrExpBuffer,0,MAX_PATH);
	// 		if(unCurrentExp <= dwLastExp)
	// 		{
	// 			 if ( unCurrentExp >= dwLastExp )
	// 			 {
	// 
	// 			 }
	// 		}
	// 	}
	//static char szText[MAX_PATH] = {0};
	//wsprintfA(szText,_T("%s:sorry!暫時無法提供這個功能"),GetCurrentRoleName());
	//Game_ShowMessage(szText);
}


//遊戲喊話call
void Game_CallSpeak(char* strSpeakData)
{
	__try
	{
		__asm
		{
			mov esi, GAME_SPEAK_BASEADDR;
			mov esi, [esi];

			mov eax, strSpeakData;
			push eax;		//壓入喊話的數據
			push 0;			//這是喊話的類型
			push 0xCD;	//不知道
			push esi;		//壓入當前的參數
			mov ebx, GAME_SPEAK_FUNCTION;
			call ebx;
			add esp, 0x10;		//4個參數，堆棧平衡
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("遊戲角色喊話異常，坑爹啊!~");
	}
}

//遊戲吶喊call
void Game_CallRoleLife()
{
	__try
	{
		__asm
		{
			//		mov eax,GAME_ROLE_LIFECHECK1;
			//		mov ecx,GAME_ROLE_LIFECHECK2;
			mov eax, dword ptr ds : [GAME_ROLE_LIFECHECK1];
			sub eax, dword ptr ds : [GAME_ROLE_LIFECHECK2]
				// 			mov eax,[eax];
				// 			mov ecx,[ecx];
				// 
				// 			sub	eax,ecx;
				test    eax, eax;
			jg		nozero;
			push 0;
			mov ebx, GAME_ROLE_LIFECALL;
			call ebx;
			add esp, 0x4;
		nozero:
			xor eax, eax;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("使用生存的吶喊異常...\r\n");
	}

}

//遊戲輸出call
void Game_ShowMessage(char* strShowText)
{
	__asm
	{
		push 0;
		push 0xFFFFFFFF;
		push 0xFFFFFFFF;
		mov ebx, GAME_INFO_SHOWPARAM;
		mov eax, [ebx];
		mov ecx, [ebx];
		push ecx;
		mov edx, [ebx];
		push edx;
		mov eax, strShowText;
		push eax;
		mov ebx, GAME_INFO_SHOWFUNC;
		call ebx;
		add esp, 0x18;
	}
}

//遊戲是否已經初始化
bool IsGameInitializeObject()
{
	return (*(PULONG)GAME_INITIALIZEOBJECT) ? true : false;
}

//當前遊戲角色類型[這裡是檢測地圖]
UINT QueryCurrentRoleType()
{
	return (*(PULONG)GAME_INITIALIZERELO);
}

//獲取當前遊戲角色名字
char* GetCurrentRoleName()
{
	__asm
	{
		mov eax, GAME_ROLE_BASEADDR;
		mov eax, [eax];
		mov eax, [eax + 0xE0];
	}
}

//獲取當前角色狀態（其他狀態）
byte GetCurrentRoleOthState()
{
	byte bzState = 0;

	__asm
	{
		mov eax, GAME_ROLE_BASEADDR;
		mov eax, [eax];
		mov ecx, [eax + 0x13];
		xor eax, eax;
		mov bzState, cl;
	}

	return bzState;
}

//獲取當前遊戲角色經驗
UINT32 GetCurrentRoleExp()
{
	__asm
	{
		mov eax, GAME_ROLE_EXPBASE;
		mov eax, [eax];
		mov eax, [eax + 0xC];
	}
}

//獲取當前遊戲角色等級
UINT32 GetCurrentRoleLevel()
{
	ULONG ulBaseAddr = 0;

	__asm
	{
		mov eax, GAME_ROLE_LEVELBASE;
		mov eax, [eax];
		mov eax, [eax + 0x480];
		mov ulBaseAddr, eax;
	}
	//獲取當前等級輸出
	return GetGameRoleHPMP(ulBaseAddr);
}

//獲取人物的飽和度
byte GetRoleSaturate()
{
	bool bIsValue = 0;

	__asm
	{
		mov eax, GAME_ROLE_SATURATE;
		mov eax, [eax];
		mov eax, [eax + 0x13];
		mov bIsValue, al;
	}
	//返回我們當前的飽和度
	return bIsValue;
}

//獲取當前角色變身狀態
UINT32 GetRoleChangeState()
{
	__asm
	{
		mov eax, GAME_ROLE_BASEADDR;
		mov eax, [eax];
		mov eax, [eax + 0x20];
		xor ecx, ecx;
		mov cx, ax;
		xor eax, eax;
		mov eax, ecx;
	}
}

//獲取當前遊戲角色狀態
BOOL GetRoleState(UINT nStateIndex)
{
	//	bool bIsState = false;
	BOOL bIsState = false;

	__asm
	{
		mov		ecx, nStateIndex;
		push		ecx;
		mov		eax, GAME_STATE_FUNADDR;
		call		eax;
		add		esp, 0x4;
		movzx   ecx, al;
		mov		bIsState, ecx;
	}

	return bIsState;
}



//是否可以使用魔法娃娃
bool IsUseStoneItem()
{
	__asm
	{
		mov eax, GAME_ROLE_BABYCHANGE;
		mov eax, [eax];
		cmp al, 0x4
			jz zero;
		xor eax, eax;
		jmp end;
	zero:
		mov eax, 1;
	end:
		test eax, eax;
	}
}

//是否可以使用魔法娃娃
bool IsUseBabyItem()
{
	__asm
	{
		mov			ecx, 0x038978;
		mov			eax, 0x68DB8BAD;
		imul			ecx;
		sar			edx, 0xA;
		mov			eax, edx;
		shr			eax, 0x1F;
		add			eax, edx;
		imul			eax, 0x9C4;
		sub			ecx, eax;
		jnz			nouse;
		mov			eax, 0x1;
		jmp			end;
	nouse:
		xor			eax, eax;
	end:
	}
}

//是否可以使用技能
bool IsUseRoleSkill()
{
	__asm
	{
		mov		eax, 0x9832D0;
		call		eax;
		mov		ecx, 0x135EED0;
		mov[ecx], eax;
		mov		eax, 0x135EED0;
		mov		eax, [eax];
		mov		ecx, 0x135EED4;
		mov		ecx, [ecx];
		cmp		eax, ecx;
		sbb		eax, eax;
		inc		eax;
	}
}

//是否處於破損武器狀態
bool IsWeaponsDamage()
{
	bool bIsDamage = false;

	__asm
	{
		mov eax, GAME_ROLE_WEAPONDAMAGE;
		mov ecx, [eax];
		test  ecx, ecx;
		jle		end;				//首先判斷基址的值是否有效
		xor	eax, eax;
		movzx   eax, byte ptr[ecx + 0x244];	//接下來再判斷我們的磨損度是多少
		test    eax, eax;
		jz		 end;
		mov bIsDamage, al;
	end:
	}
	return bIsDamage;
}

//獲取當前物品欄最大數量
UINT32 GetItemsCount()
{
	UINT32 ulItemsCount = 0;

	__try
	{
		PULONG ulBase = (PULONG)GAME_BASE_ADDRESS;
		__asm
		{
			mov ecx, ulBase;
			mov ecx, [ecx];
			add ecx, 0x54;
			mov eax, [ecx + 0x4];				//從這裡開始，是遊戲的死碼，可以直接下斷點直接搞定遊戲血藍信息
			sub eax, [ecx];
			sar eax, 0x2;
			mov ulItemsCount, eax;

		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("獲取物品數量異常，坑爹啊!~");
	}

	//返回我們獲取到的文件列表數量
	return ulItemsCount;
}


//這個不清楚是什麼，但是要判定
bool IsSelectItem()
{
	bool bIsLoad = false;
	__try
	{
		__asm
		{
			mov eax, GAME_ITEM_ADDRESS;
			mov eax, [eax];
			mov eax, [eax + 0x154];
			mov eax, [eax + 0x1BC];
			mov bIsLoad, al;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("獲取數據異常!\r\n");
	}

	return bIsLoad;
}

//是否初始化對象
bool InitializeGameItem()
{
	__try
	{
		__asm
		{
			mov eax, GAME_ITEM_ADDRESS;
			mov eax, [eax];
			mov ecx, [eax + 0x154];
			mov ebx, 0x917A60;
			call ebx;

		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("初始化遊戲物品欄接口!\r\n");
	}

	return true;
}

//通過指定的下標獲取指定的物品對象
PITEMPOINT GetItemByIndex(UINT32 nIndex)
{
	ULONG nItemPoint = 0;
	PITEMPOINT itempoint = NULL;


	//檢查參數是否正常，否則強制初始化
	if (IsSelectItem())
		InitializeGameItem();


	TRY
	{
		__asm
		{
			mov eax, GAME_ITEM_ADDRESS;
			mov eax, [eax];
			mov eax, [eax + 0x154];
			mov eax, [eax + 0x150];
			mov esi, [nIndex];
			mov eax, [eax + esi * 4];
			mov eax, [eax + 1ECh];
			mov nItemPoint, eax;	//將指針保存到變量中返回給調用者
		}

		if (nItemPoint)
		{
			//如果這個指針是有效的話		
			itempoint = new ITEMPOINT;
			itempoint->ulIndex = nItemPoint;
			itempoint->ulItemID = (*(ULONG*)(nItemPoint + 0x8));		//$+8      >136628DD  ASCII "seEvent = "ToolTip">\r\n\t\t\t\t\t<Image Name = "Tooltip_Image" Surf = "5488;2;20" Activate = "0">\r\n\t\t\t\t\t\t<"
			itempoint->strName = (char*)(nItemPoint + 0x2B);						//物品的名字
			itempoint->ulCount = (*(ULONG*)(nItemPoint + 0x20));				//物品的數量

		}
	}
		CATCH(CMemoryException, e)
	{
		delete e;
		TRACE("獲取遊戲物品欄對象異常!\r\n");
	}
	END_CATCH

		return itempoint;
}

inline int InitItems()
{
	int nRetn = 0;
	__asm
	{
		lea   eax, nRetn;
		push eax;
		mov eax, GAME_ITEM_ADDRESS;
		mov eax, [eax];
		mov ecx, [eax + 0x154];
		mov ebx, 0x917A60;
		call ebx;
	}
	return 1;
}

//檢查是否空指針
void RefreshItems()
{
	TRY
	{
		__asm
		{
			mov eax, GAME_ITEM_ADDRESS;
			mov eax, [eax];
			mov eax, [eax + 0x154];
			mov eax, [eax + 0x1BC];
			test eax, eax;
			jz end;
			call InitItems;
		end:
		}
	}
		CATCH(CMemoryException, e)
	{
		delete e;
	}
	END_CATCH
}

//通過指定的物品名獲取指定的物品對象,失敗返回NULL
PITEMPOINT GetItemByName(char* strName)
{
	ULONG nItemPoint = 0;
	PITEMPOINT itempoint = NULL;
	PULONG pItemsArray = NULL;
	ULONG nItemCount = GetItemsCount();

	TRY
	{
		__asm
		{
			mov eax, GAME_ITEM_ADDRESS;
			mov eax, [eax];
			mov eax, [eax + 0x154];
			mov eax, [eax + 0x150];
			mov pItemsArray, eax;

			// 			mov esi,[nIndex];
			// 			mov eax, [eax+esi*4];
			// 			mov eax, [eax+1ECh];
			// 			mov nItemPoint,eax;	//將指針保存到變量中返回給調用者
		}

		//這裡我們通過遍歷指針來獲取我們指定的對象
		for (int i = 0; i < nItemCount; i++)
		{
			//這裡遍歷的時候需要檢查我們的數據是否發送變更
			RefreshItems();
			//當前物品指針
			nItemPoint = *(PULONG)(pItemsArray[i] + 0x1EC);
			//0x2B使我們物品的名字
			//if(strstr((char*)(nItemPoint + 0x2B),strName))
			int nlen = strlen(strName);
			if (nlen && memicmp((char*)(nItemPoint + 0x2B), strName, nlen) == 0)
			{
				//找到了
				//如果這個指針是有效的話		
				itempoint = new ITEMPOINT;
				//ok我們開始把這個結果返回給上傳
				itempoint->ulIndex = nItemPoint;
				itempoint->ulItemID = (*(ULONG*)(nItemPoint + 0x8));		//$+8      >136628DD  ASCII "seEvent = "ToolTip">\r\n\t\t\t\t\t<Image Name = "Tooltip_Image" Surf = "5488;2;20" Activate = "0">\r\n\t\t\t\t\t\t<"
				itempoint->strName = (char*)(nItemPoint + 0x2B);						//物品的名字
				itempoint->ulCount = (*(ULONG*)(nItemPoint + 0x20));				//物品的數量
				break;
			}
		}
	}
		CATCH(CMemoryException, e)
	{
		delete e;
	}
	END_CATCH

		//返回我們指針的數據
		return itempoint;
}

//獲取低耐久度裝備的物品指針
PITEMPOINT GetLowRepairRoleItem(char* pKeyName/* = _T("揮舞")*/)
{
	ULONG nItemPoint = 0;
	PITEMPOINT itempoint = NULL;
	PULONG pItemsArray = NULL;
	ULONG nItemCount = GetItemsCount();

	TRY
	{
		__asm
		{
			mov eax, GAME_ITEM_ADDRESS;
			mov eax, [eax];
			mov eax, [eax + 0x154];
			mov eax, [eax + 0x150];
			mov pItemsArray, eax;

			// 			mov esi,[nIndex];
			// 			mov eax, [eax+esi*4];
			// 			mov eax, [eax+1ECh];
			// 			mov nItemPoint,eax;	//將指針保存到變量中返回給調用者
		}

		//這裡我們通過遍歷指針來獲取我們指定的對象
		for (int i = 0; i < nItemCount; i++)
		{
			//這裡遍歷的時候需要檢查我們的數據是否發送變更
			RefreshItems();

			//當前物品指針
			nItemPoint = *(PULONG)(pItemsArray[i] + 0x1EC);
			//0x18使我們物品的類型
			if (*(byte*)(nItemPoint + 0x18) == 0x1 && strstr((char*)(nItemPoint + 0x2B), pKeyName))
			{
				//找到了
				if (strstr((char*)(nItemPoint + 0x130), _T("損壞度")))
				{
					//如果這個指針是有效的話		
					itempoint = new ITEMPOINT;
					//ok我們開始把這個結果返回給上傳
					itempoint->ulIndex = nItemPoint;
					itempoint->ulItemID = (*(ULONG*)(nItemPoint + 0x8));		//$+8      >136628DD  ASCII "seEvent = "ToolTip">\r\n\t\t\t\t\t<Image Name = "Tooltip_Image" Surf = "5488;2;20" Activate = "0">\r\n\t\t\t\t\t\t<"
					itempoint->strName = (char*)(nItemPoint + 0x2B);						//物品的名字
					itempoint->ulCount = (*(ULONG*)(nItemPoint + 0x20));				//物品的數量		
				}

				break;
			}
		}
	}
		CATCH(CMemoryException, e)
	{
		delete e;
	}
	END_CATCH

		//返回我們指針的數據
		return itempoint;
}

//獲取當前技能最大數量
UINT32 GetSkillCount()
{
	UINT32 ulCount = 0;

	__try
	{
		__asm
		{
			mov ecx, GAME_SKILL_ADDRESS;
			mov ecx, [ecx];
			mov eax, [ecx + 0x2C];
			mov ulCount, eax;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("獲取技能數量異常，坑爹啊!~");
	}

	return ulCount;
}

//查找技能call
typedef ULONG(FASTCALL* FINDSKILLCALL)(ULONG nObject, int nSkillIndex);
//技能是否學習
typedef ULONG(FASTCALL* STUDYSKILLCALL)(ULONG nObject, int nSkillIndex);
//準備施法指定的技能
typedef ULONG(FASTCALL* ENTRYSKILLCALL)(ULONG nObject, int nSkillIndex);
//進入施法狀態
typedef ULONG(FASTCALL* STARTSKILLCALL)(ULONG nObject);

__inline ULONG FASTCALL Game_FindSkillCall(ULONG nObject, int nSkillIndex)
{
	__asm
	{
		push nSkillIndex;
		mov ecx, nObject;
		mov eax, GAME_SKILL_FINDADDR;
		call eax;
	}
}

__inline byte FASTCALL Game_StudSkillCall(ULONG nObject, int nSkillIndex)
{
	bool bIsStud = false;
	__asm
	{
		push nSkillIndex;
		mov ecx, nObject;
		mov eax, GAME_SKILL_EXTENADDR;
		call eax;
		mov bIsStud, al;
	}

	return bIsStud;
}

__inline ULONG FASTCALL Game_EntrySkillCall(ULONG nObject, int nSkillIndex)
{
	__asm
	{
		push nSkillIndex;
		mov ecx, nObject;
		mov eax, GAME_SKILL_ENTRYADDR;
		call eax;
	}
}
__inline ULONG FASTCALL Game_StartSkillCall(ULONG nObject)
{
	__asm
	{
		mov ecx, nObject;
		mov eax, GAME_SKILL_STARTADDR;
		call eax;
	}
}

//通過指定的下標獲取指定的技能對象，失敗返回NULL
PSKILLPOINT GetSkillByIndex(UINT32 nIndex)
{
	ULONG nSkillPoint = 0;
	PSKILLPOINT skillpoint = NULL;

	TRY
	{
		//查找我們的技能
		nSkillPoint = Game_FindSkillCall(GAME_SKILL_PAMADDR, nIndex);
		//查詢我們技能是否學習了
		if (nSkillPoint)
		{
			skillpoint = new SKILLPOINT;
			skillpoint->ulIndex = nSkillPoint;
			skillpoint->ulSkillID = *(ULONG*)nSkillPoint;
			skillpoint->strName = (char*)(nSkillPoint + 0x6C);
			skillpoint->ulUnkown = 0;
		}
	}
		CATCH(CMemoryException, e)
	{
		delete e;
	}
	END_CATCH

		return skillpoint;
}

//通過指定的名字獲取自定的技能對象，失敗返回NULL
PSKILLPOINT GetSkillByName(char* strName)
{
	ULONG nSkillPoint = 0;
	PSKILLPOINT skillpoint = NULL;
	ULONG nSkillCount = GetSkillCount();

	TRY
	{
		for (int i = 0; i < nSkillCount; i++)
		{
			//查找我們的技能
			nSkillPoint = Game_FindSkillCall(GAME_SKILL_PAMADDR, i);
			if (nSkillPoint == 0)
				continue;

			//否則開始查找我們的技能
			//查詢我們技能是否學習了[並且同時檢測角色是否可以放]
			if (nSkillPoint)
			{
				//查詢一下是否是我們的指定查找的技能
				if (strstr((char*)(nSkillPoint + 0x6C), strName))
				{
					//如果是指定的技能，那麼我們再判定一下是否這個技能我們可以使用
					//if(Game_IsUseSkill(*(ULONG*)nSkillPoint))
					if (Game_StudSkillCall(GAME_SKILL_PAMADDR, *(ULONG*)nSkillPoint))
					{
						//如果可以使用的話，那麼我們就把這個結構信息返回給上層即可
						skillpoint = new SKILLPOINT;
						skillpoint->ulIndex = nSkillPoint;
						skillpoint->ulSkillID = *(ULONG*)nSkillPoint;
						skillpoint->strName = (char*)(nSkillPoint + 0x6C);
						skillpoint->ulUnkown = 0;
					}
					break;
				}
			}

		}

	}
		CATCH(CMemoryException, e)
	{
		delete e;
	}
	END_CATCH

		return skillpoint;
}

//技能是否可以被使用
bool Game_IsUseSkill(UINT32 nIndex)
{
	bool bIsUse = false;
	__asm
	{
		mov eax, nIndex;
		push eax;
		mov ebx, GAME_SKILL_ISUSEADDR;
		call ebx;
		add esp, 0x4;
		mov bIsUse, al;
	}

	return bIsUse;
}

//使用範圍技能call->例如火風暴
void Game_UseRegionSkill(PSKILLPOINT pt)
{
	if (pt == NULL)
		return;

	ULONG ulSkillID = pt->ulSkillID;
	__try
	{
		Game_EntrySkillCall(GAME_SKILL_PAMADDR, ulSkillID);
		Game_StartSkillCall(GAME_SKILL_PAMADDR);

		__asm
		{
			mov		ecx, ulSkillID;
			and		ecx, 7;
			push		ecx;
			mov		edx, ulSkillID;
			sar		edx, 3;
			push		edx;
			push		0x86;
			push		0x103E4E8;			//ccc
			mov eax, GAME_ITEM_FUNCTION;
			call eax;
			add esp, 0x10;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("什麼鬼，使用技能奔潰了，坑爹哇");
	}
}

//使用技能call->用於不需要選人的技能，即放即施法的
void Game_UseSkill(PSKILLPOINT pt)
{
	if (pt == NULL)
		return;

	ULONG ulSkillID = pt->ulSkillID;
	__try
	{
		Game_EntrySkillCall(GAME_SKILL_PAMADDR, ulSkillID);
		Game_StartSkillCall(GAME_SKILL_PAMADDR);

		__asm
		{
			mov		ecx, ulSkillID;
			and		ecx, 7;
			push		ecx;
			mov		edx, ulSkillID;
			sar		edx, 3;
			push		edx;
			push		0x86;
			push		0xFB899C;
			mov eax, GAME_ITEM_FUNCTION;
			call eax;
			add esp, 0x10;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("什麼鬼，使用技能奔潰了，坑爹哇");
	}
}

//使用技能call->用於需要指定對象的
void Game_UseSkill(PSKILLPOINT pt, UINT32 nTrageID)
{
	if (pt == NULL)
		return;

	ULONG ulSkillID = pt->ulSkillID;
	__try
	{
		Game_EntrySkillCall(GAME_SKILL_PAMADDR, ulSkillID);
		Game_StartSkillCall(GAME_SKILL_PAMADDR);

		__asm
		{
			mov		ebx, nTrageID;
			push     ebx;
			mov		ecx, ulSkillID;
			and		ecx, 7;
			push		ecx;
			mov		edx, ulSkillID;
			sar		edx, 3;
			push		edx;
			push		0x86;
			push		0xFB882C;
			mov		eax, GAME_ITEM_FUNCTION;
			call		eax;
			add		esp, 0x14;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("什麼鬼，使用技能奔潰了，坑爹哇");
	}
}
//使用物品call
void Game_UseItem(PITEMPOINT pt)
{
	if (pt == NULL)
		return;
	ULONG ulPoint = pt->ulIndex;
	ULONG ulItemID = pt->ulItemID;
	/*
	0090249A  |> \68 1DFEFE00   push GameClie.00FEFE1D
	0090249F  |.  8B4D 08       mov ecx,[arg.1]                          ;  我們通過這裡知道我們call的參數源自於第一個參數
	009024A2  |.  E8 C9921E00   call GameClie.00AEB770                   ;  我們從這裡得到我們的物品指針
	009024A7  |.  50            push eax                                 ;  從這裡我們可以知道，這是我們物品的一個call
	009024A8  |.  6A 68         push 0x68
	009024AA  |.  68 30FEFE00   push GameClie.00FEFE30                   ;  cds
	009024AF  |.  E8 AC3B0600   call GameClie.00966060                   ;  使用物品call


	*/
	__try
	{
		__asm
		{
			push 0x0FEFE1D;
			mov eax, ulItemID;
			push eax;
			push 0x68;
			push 0x0FEFE30;
			mov eax, GAME_ITEM_FUNCTION;
			call eax;
			add esp, 0x10;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("什麼鬼，使用物品奔潰了，坑爹哇");
	}
}

//使用變身物品call
void Game_UseChangeItem(PITEMPOINT pt, const char* strChangeRole)
{
	if (pt == NULL)
		return;
	ULONG ulPoint = pt->ulIndex;
	ULONG ulItemID = pt->ulItemID;
	/*
	00AE39B9   .  50            push eax
	00AE39BA   .  8B4D FC       mov ecx,dword ptr ss:[ebp-0x4]
	00AE39BD   .  8B89 58010000 mov ecx,dword ptr ds:[ecx+0x158]
	00AE39C3   .  E8 A87D0000   call GameClie.00AEB770
	00AE39C8   .  50            push eax
	00AE39C9   .  6A 68         push 0x68
	00AE39CB   .  68 40230101   push GameClie.01012340                   ;  cds
	00AE39D0   .  E8 8B26E8FF   call GameClie.00966060                   ;  開始變身


	*/
	__try
	{
		__asm
		{
			mov ebx, strChangeRole;
			push ebx;
			mov eax, ulItemID;
			push eax;
			push 0x68;
			push 0x1012340;
			mov eax, GAME_ITEM_FUNCTION;
			call eax;
			add esp, 0x10;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("什麼鬼，使用物品奔潰了，坑爹哇");
	}
}

//使用Oth物品call
void Game_UseOthItem(PITEMPOINT pt, UINT32 nTrageID)
{
	if (pt == NULL)
		return;
	ULONG ulPoint = pt->ulIndex;
	ULONG ulItemID = pt->ulItemID;
	__try
	{
		__asm
		{
			mov ebx, nTrageID;
			push ebx;
			mov eax, ulItemID;
			push eax;
			push 0x68;
			push 0xFB974C;
			mov eax, GAME_ITEM_FUNCTION;
			call eax;
			add esp, 0x10;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("什麼鬼，使用物品奔潰了，坑爹哇");
	}
}

//使用卷軸物品call
void Game_UseTestItem(PITEMPOINT pt, UINT32 nTrageID)
{
	if (pt == NULL)
		return;
	ULONG ulPoint = pt->ulIndex;
	ULONG ulItemID = pt->ulItemID;
	__try
	{
		__asm
		{
			mov ebx, nTrageID;
			push ebx;
			mov eax, ulItemID;
			push eax;
			push 0x68;
			push 0x103F0D4;
			mov eax, GAME_ITEM_FUNCTION;
			call eax;
			add esp, 0x10;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("什麼鬼，使用物品奔潰了，坑爹哇");
	}
}

//使用卷軸物品call
void Game_UseMageItem(PITEMPOINT pt, UINT32 nTrageID)
{
	//80版本已經修復
	if (pt == NULL)
		return;
	ULONG ulPoint = pt->ulIndex;
	ULONG ulItemID = pt->ulItemID;
	__try
	{
		__asm
		{
			mov ebx, nTrageID;
			push ebx;
			mov eax, ulItemID;
			push eax;
			push 0x68;
			push 0xFB974C;			//cdd
			mov eax, GAME_ITEM_FUNCTION;
			call eax;
			add esp, 0x10;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("什麼鬼，使用物品奔潰了，坑爹哇");
	}
}

//使用指定位置傳送
void Game_UsePostionItem(PITEMPOINT pt, PSTATUSPOINT pst)
{
	if (pt == NULL || pst == NULL)
		return;

	ULONG ulItemID = pt->ulItemID;
	ULONG ulposX = pst->ulValueX;						//坐標X
	ULONG ulposY = pst->ulValueY;						//坐標Y
	ULONG ulposscreen = pst->ulTransmitID;		//場景ID

	__try
	{
		__asm
		{
			mov ebx, ulposY;
			push ebx;
			mov ebx, ulposX;
			push ebx;
			mov ebx, ulposscreen;
			push ebx;
			mov eax, ulItemID;
			push eax;
			push 0x68;
			push 0x0FB971C;
			mov eax, GAME_ITEM_FUNCTION;
			call eax;
			add esp, 0x18;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("什麼鬼，使用物品奔潰了，坑爹哇");
	}

}

//刪除物品call
void Game_DelItem(PITEMPOINT pt)
{
	if (pt == NULL)
		return;
	ULONG ulItemID = pt->ulItemID;
	ULONG ulItemNumber = pt->ulCount;

	__try
	{
		__asm
		{
			mov ebx, ulItemNumber;
			push ebx;
			mov eax, ulItemID;
			push eax;
			push 0x1;
			push 0x2D;
			push 0xFED410;			//這個標記是刪除物品
			mov eax, GAME_ITEM_FUNCTION;
			call eax;
			add esp, 0x14;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("什麼鬼，刪除物品奔潰了，坑爹哇");
	}
}

//熔煉物品call->這個call是錯誤的，不要用
void Game_SmeltItem(PITEMPOINT pt)
{
	if (pt == NULL)
		return;
	ULONG ulItemID = pt->ulItemID;
	ULONG ulItemNumber = pt->ulCount;
	__try
	{
		__asm
		{
			mov ebx, ulItemNumber;
			push ebx;
			mov eax, ulItemID;
			push eax;
			push 0x1;
			push 0x2D;
			push 0x0FF0170;		//這個標記是熔煉物品
			mov eax, GAME_ITEM_FUNCTION;
			call eax;
			add esp, 0x14;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("什麼鬼，熔煉物品奔潰了，坑爹哇");
	}
}

//獲取當前祝瞬數量
UINT32 GetStatusCount()
{
	UINT32 ulStatusCount = 0;

	__try
	{
		PULONG ulBase = (PULONG)GAME_STATUS_ADDRESS;
		__asm
		{
			mov eax, ulBase;
			mov eax, [eax];
			mov esi, [eax + 0x170];
			sub esi, [eax + 0x16C];
			sar esi, 2;
			mov ulStatusCount, esi;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("獲取祝瞬數量異常，坑爹啊!~");
	}

	//返回我們獲取到的文件列表數量
	return ulStatusCount;
}

/*
typedef struct GAME_STATUSPOINT
{
GAME_STATUSPOINT()
{
memset(this,0,sizeof(GAME_STATUSPOINT));
}

ULONG		ulIndex;						//移動的指針
ULONG		ulTransmitID;				//這個指針的ID號
ULONG		ulValueX;
ULONG		ulValueY;
char*		strName;
}STATUSPOINT,*PSTATUSPOINT;
*/

//通過指定的下標獲取指定的祝瞬對象
PSTATUSPOINT GetStatusIndex(UINT32 nIndex)
{
	ULONG nStatusPoint = 0;
	PSTATUSPOINT statuspoint = NULL;

	TRY
	{
		__asm
		{
			mov eax, GAME_STATUS_ADDRESS;
			mov eax, [eax];
			mov eax, [eax + 16Ch];
			mov esi, [nIndex];
			mov eax, [eax + esi * 4];
			mov nStatusPoint, eax;	//將指針保存到變量中返回給調用者
		}

		if (nStatusPoint)
		{
			//如果這個指針是有效的話		
			statuspoint = new STATUSPOINT;
			/*這裡還有數據在處理，具體是什麼不知道*/
			statuspoint->ulIndex = nStatusPoint;
			ULONG ulAddress = 0;
			__asm
			{
				mov ebx, nStatusPoint;
				lea   ecx, [ebx + 4];
				mov  ulAddress, ecx;
			}
			statuspoint->ulTransmitID = GetGameRoleHPMP(ulAddress);
			__asm
			{
				mov ebx, nStatusPoint;
				lea   ecx, [ebx + 0x10];
				mov  ulAddress, ecx;
			}
			statuspoint->ulValueX = GetGameRoleHPMP(ulAddress);

			__asm
			{
				mov ebx, nStatusPoint;
				lea   ecx, [ebx + 0x1C];
				mov  ulAddress, ecx;
			}
			statuspoint->ulValueY = GetGameRoleHPMP(ulAddress);
			statuspoint->strName = (char*)(*(ULONG*)(nStatusPoint + 0x28));		//$+28     >19C99FA0  ASCII "\db\fT[55~] ?? ?? ??"

		}
	}
		CATCH(CMemoryException, e)
	{
		delete e;
		TRACE("獲取遊戲物品欄對象異常!\r\n");
	}
	END_CATCH

		return statuspoint;
}

//通過指定的名字獲取指定的祝瞬對象
PSTATUSPOINT	 GetStatusByName(char* strname)
{
	ULONG nStatusPoint = 0;
	PSTATUSPOINT statuspoint = NULL;
	int nCount = GetStatusCount();

	TRY
	{
		for (int i = 0; i < nCount; i++)
		{
			__asm
			{
				mov eax, GAME_STATUS_ADDRESS;
				mov eax, [eax];
				mov eax, [eax + 16Ch];
				mov esi, [i];
				mov eax, [eax + esi * 4];
				mov nStatusPoint, eax;	//將指針保存到變量中返回給調用者
			}

			if (nStatusPoint && stricmp((char*)(*(ULONG*)(nStatusPoint + 0x28)), strname) == 0)
			{
				//如果這個指針是有效的話		
				statuspoint = new STATUSPOINT;
				/*這裡還有數據在處理，具體是什麼不知道*/
				statuspoint->ulIndex = nStatusPoint;
				ULONG ulAddress = 0;
				__asm
				{
					mov ebx, nStatusPoint;
					lea   ecx, [ebx + 4];
					mov  ulAddress, ecx;
				}
				statuspoint->ulTransmitID = GetGameRoleHPMP(ulAddress);
				__asm
				{
					mov ebx, nStatusPoint;
					lea   ecx, [ebx + 0x10];
					mov  ulAddress, ecx;
				}
				statuspoint->ulValueX = GetGameRoleHPMP(ulAddress);

				__asm
				{
					mov ebx, nStatusPoint;
					lea   ecx, [ebx + 0x1C];
					mov  ulAddress, ecx;
				}
				statuspoint->ulValueY = GetGameRoleHPMP(ulAddress);
				statuspoint->strName = (char*)(*(ULONG*)(nStatusPoint + 0x28));		//$+28     >19C99FA0  ASCII "\db\fT[55~] ?? ?? ??"
				break;
			}
		}

	}
		CATCH(CMemoryException, e)
	{
		delete e;
		TRACE("獲取遊戲物品欄對象異常!\r\n");
	}
	END_CATCH

		return statuspoint;
}

//獲取當前角色的血藍信息
UINT32 GetGameRoleHPMP(ULONG ulAddress)
{
	UINT32 ulHPMPValue = 0;

	__try
	{
		__asm
		{
			mov eax, ulAddress;
			mov ecx, [eax];
			xor ecx, 0xC0017921;
			mov ebx, [eax + 0x4];
			mov edx, [ebx + ecx * 4];
			xor edx, [eax + 0x8];
			mov eax, edx;
			mov[ulHPMPValue], eax;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("獲取人物血藍信息異常，坑爹啊!~");
	}

	return ulHPMPValue;
}

//獲取當前角色對象ID
UINT32 GetRoleObjectID()
{
	__asm
	{
		mov eax, GAME_ROLE_BASEADDR;
		mov eax, [eax];
		mov eax, [eax + 0x18];
	}
}

//獲取當前NPC數組數量
UINT32 GetNPCCount()
{
	return *(PULONG)GAME_NPC_NPCNUMBER;
}

//獲取當前NPC對象ID通過指定名字
UINT32 GetNPCObjectIDByName(char* strNPCname)
{
	//獲取比較結果
	//int nCompare = stricmp(_T(""), strNPCname);
	__try
	{
		PULONG pArray = *(ULONG**)GAME_NPC_BASEADDR;
		int nCount = GetNPCCount();
		for (int i = 0; i < nCount; i++)
		{
			char* ulNpcPoint = (char*)pArray[i];
			if (strstr(*(const char **)(ulNpcPoint + 0xE0), strNPCname))
				return *(PULONG)(ulNpcPoint + 0x18);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("又是什麼鬼羅!遍歷NPC異常\r\n");
	}


	return 0;
}

//獲取鼠標指向的對象ID
UINT32 GetMouseTrageObjectID()
{
	UINT32 uTid = 0;

	__try
	{
		__asm
		{
			mov eax, GAME_MOUSE_NPCADDR;
			mov eax, [eax];
			test eax, eax;
			jz zero;
			mov eax, [eax + 0x18];
			mov uTid, eax;
		zero:
			xor eax, eax;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("GetMouseTrageObjectID\r\n");
	}

	return uTid;
}

//調用遊戲函數>>這個接口提供給上上層使用
void CallGameFunction(char* strname)
{
	if (strlen(strname) <= 0)
		return;

	if (strstr(strname, _T("/I")))	//首先判斷是否使用物品
		return UseItem(strname);
	else if (strstr(strname, _T("/M")) && IsUseRoleSkill())// 然後判斷是否可以使用技能
		return UseSkill(strname);
	else	//如果沒有選擇，直接調用物品使用接口
		return UseItem(strname);
}

//使用物品
void UseItem(char* strItemName)
{
	char* pfind = strstr(strItemName, _T("/"));
	char szTextName[MAX_PATH] = { 0 };

	PITEMPOINT pItem = NULL;

	if (pfind == NULL)
	{
		//沒有找到，那麼，直接調用底層的接口使用物品即可
		pItem = GetItemByName(strItemName);
		if (pItem)
		{
			UseItemByRole(pItem);
			delete pItem;
		}
		return;
	}

	//這裡開始處理所有的物品列表[獲取我們的物品名字]
	__try
	{

		strncpy(szTextName, strItemName, pfind - strItemName);

		//獲取我們的物品指針
		pItem = GetItemByName(szTextName);
		if (pItem == NULL)
			goto STOP;

		pfind++;
		char* pszItem = NULL;

		pszItem = strstr(pfind, _T("IME"));
		if (pszItem)
		{
			//對自己使用的物品
			UseItemByRole(pItem);
			goto STOP;
		}

		pszItem = strstr(pfind, _T("IM"));
		if (pszItem)
		{
			//對自己使用物品
			//	UseItemByRole(pItem);
			goto STOP;
		}

		pszItem = strstr(pfind, _T("IN"));
		if (pszItem)
		{
			//我們還要找到目標是誰
			char* ptrage = pszItem + 3;
			UseItemByTrageNPC(pItem, ptrage);
			goto STOP;
		}

		pszItem = strstr(pfind, _T("IT"));
		if (pszItem)
		{
			//對鼠標指定的人使用
			UseItemByMouse(pItem);
			goto STOP;
		}

		pszItem = strstr(pfind, _T("IIW"));
		if (pszItem)
		{
			//對武器附魔
			char* ptrage = pszItem + 4;
			UseItemByRoleWeapons(pItem, ptrage);
			goto STOP;
		}

		pszItem = strstr(pfind, _T("IIA"));
		if (pszItem)
		{
			//對防具進行附魔
			char* ptrage = pszItem + 4;
			UseItemByRoleArmor(pItem, ptrage);
			goto STOP;
		}
		//熔煉劑/II=**裝備
		pszItem = strstr(pfind, _T("II"));
		if (pszItem)
		{
			//對裝備進行熔煉
			char* ptrage = pszItem + 3;
			UseItemBySmeltRoleItem(pItem, ptrage);
			goto STOP;
		}

		pszItem = strstr(pfind, _T("IBM"));	//示例:瞬間移動卷軸/IBM=1234
		if (pszItem)
		{
			//使用飛坐標
			char* ptrage = pszItem + 4;
			UseItemBySavePostionName(pItem, ptrage);
			goto STOP;
		}


	STOP:
		if (pItem)
			delete pItem;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("調用使用物品異常!\r\n");
	}
}


//使用技能
void UseSkill(char* strSkillName)
{

	char* pfind = strstr(strSkillName, _T("/"));
	char szTextName[MAX_PATH] = { 0 };

	PSKILLPOINT pskill = NULL;

	if (pfind == NULL)
		return;

	//這裡開始處理所有的技能列表[獲取我們的物品名字]
	__try
	{

		strncpy(szTextName, strSkillName, pfind - strSkillName);

		//獲取我們的技能指針
		pskill = GetSkillByName(szTextName);
		if (pskill == NULL)
			goto STOP;

		pfind++;
		char* pszskill = NULL;

		pszskill = strstr(pfind, _T("MME"));
		if (pszskill)
		{
			//對自己使用的技能
			UseSkillByRoleEx(pskill);
			goto STOP;
		}

		pszskill = strstr(pfind, _T("ME"));
		if (pszskill)
		{
			//對自己使用的技能
			UseSkillByRoleEx(pskill);
			goto STOP;
		}

		pszskill = strstr(pfind, _T("MM"));
		if (pszskill)
		{
			//對自己使用的技能
			UseSkillByRole(pskill);
			goto STOP;
		}

		pszskill = strstr(pfind, _T("MN"));
		if (pszskill)
		{
			//我們還要找到目標是誰
			char* ptrage = pszskill + 3;
			UseSkillByTrageNPC(pskill, ptrage);
			goto STOP;
		}

		pszskill = strstr(pfind, _T("MT"));
		if (pszskill)
		{
			//對鼠標指定的人使用
			UseSkillByMouse(pskill);
			goto STOP;
		}

		pszskill = strstr(pfind, _T("MIW"));
		if (pszskill)
		{
			//對武器附魔
			char* ptrage = pszskill + 4;
			UseSkillByRoleWeapons(pskill, ptrage);
			goto STOP;
		}

		pszskill = strstr(pfind, _T("MIA"));
		if (pszskill)
		{
			//對防具進行附魔
			char* ptrage = pszskill + 4;
			UseSkillByRoleArmor(pskill, ptrage);
			goto STOP;
		}

		pszskill = strstr(pfind, _T("MIS"));
		if (pszskill)
		{
			//魔石合成
			char* ptrage = pszskill + 4;
			UseSkillByRoleStone(pskill, ptrage);
			goto STOP;
		}


		pszskill = strstr(pfind, _T("M"));
		if (pszskill)
		{
			//使用技能
			UseSkillByRole(pskill);
			goto STOP;
		}
	STOP:
		if (pskill)
			delete pskill;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		TRACE("調用使用技能異常!\r\n");
	}
}

//對角色使用物品
void UseItemByRole(PITEMPOINT pt)
{
	if (pt == NULL)
		return;

	//獲取自己的ID
	UINT32 uRoleID = GetRoleObjectID();
	if (uRoleID != 0)
		Game_UseTestItem(pt, uRoleID);
}

//對鼠標指向的NPC使用物品
void UseItemByMouse(PITEMPOINT pt)
{
	//獲取鼠標指向的目標id
	UINT uTrageID = GetMouseTrageObjectID();
	if (uTrageID != 0)
		Game_UseOthItem(pt, uTrageID);
}

//對指定名字的NPC使用物品
void UseItemByTrageNPC(PITEMPOINT pt, char* strNpcName)
{
	if (strlen(strNpcName) <= 0)
		return;

	//獲取指定角色的uid
	UINT uTrageID = GetNPCObjectIDByName(strNpcName);
	if (uTrageID != 0)
		Game_UseMageItem(pt, uTrageID);
}

//對武器使用物品
void UseItemByRoleWeapons(PITEMPOINT pt, char* strWeaponsName)
{
	if (strlen(strWeaponsName) <= 0)
		return;

	//通過我們的物品欄查找我們的裝備信息
	PITEMPOINT pTrageItem = GetItemByName(strWeaponsName);
	if (pTrageItem)
	{
		Game_UseOthItem(pt, pTrageItem->ulItemID);
		delete pTrageItem;
	}
}

//對防具使用物品
void UseItemByRoleArmor(PITEMPOINT pt, char* strArmorName)
{
	if (strlen(strArmorName) <= 0)
		return;

	//通過我們的物品欄查找我們的裝備信息
	PITEMPOINT pTrageItem = GetItemByName(strArmorName);
	if (pTrageItem)
	{
		Game_UseOthItem(pt, pTrageItem->ulItemID);
		delete pTrageItem;
	}
}

//對裝備使用熔煉物品 參數1 我們熔煉物品的指針 參數2，我們需要熔煉的物品名字
void UseItemBySmeltRoleItem(PITEMPOINT pt, char* strItemName)
{
	//首先進行參數檢查
	if (pt == NULL)
		return;

	if (strlen(strItemName) <= 0)
		return;

	//查找我們需要的物品在不在
	//通過我們的物品欄查找我們的裝備信息
	PITEMPOINT pTrageItem = GetItemByName(strItemName);
	if (pTrageItem)
	{
		//這裡調用即可
		Game_UseOthItem(pt, pTrageItem->ulItemID);
		delete pTrageItem;
	}

}

//對裝備使用磨刀石[參數1 我們磨刀石的指針 參數2我們需要修理裝備的裝備指針]
void UseItemByRepairRoleItem(PITEMPOINT pt, PITEMPOINT pTrage)
{
	//首先進行參數檢查
	if (pt == NULL || pTrage == NULL)
		return;

	//這裡調用即可
	Game_UseOthItem(pt, pTrage->ulItemID);
}

//對角色使用瞬移坐標
void UseItemBySavePostionName(PITEMPOINT pt, char* strposname)
{
	if (pt == NULL)
		return;
	if (strlen(strposname) < 0)
		return;

	//這裡我們查找一下屬於我們坐標的位置
	PSTATUSPOINT pst = GetStatusByName(strposname);
	if (pst)
	{
		//這裡調用我們的接口
		Game_UsePostionItem(pt, pst);
		delete pst;
	}

}

//對角色使用技能
void UseSkillByRole(PSKILLPOINT pt)
{
	if (pt == NULL)
		return;

	//對自己使用技能
	Game_UseSkill(pt);
}

//對於需要二次確定的技能調用這裡
void UseSkillByRoleEx(PSKILLPOINT pt)
{
	if (pt == NULL)
		return;

	//獲取自身對象的id
	UINT uTrageID = GetRoleObjectID();

	if (uTrageID != 0)
		Game_UseSkill(pt, uTrageID);
}

//對鼠標指向的NPC使用技能
void UseSkillByMouse(PSKILLPOINT pt)
{
	//獲取鼠標指向的目標id
	UINT uTrageID = GetMouseTrageObjectID();
	if (uTrageID != 0)
		Game_UseSkill(pt, uTrageID);
}

//對指定名字的NPC使用技能
void UseSkillByTrageNPC(PSKILLPOINT pt, char* strNpcName)
{
	if (strlen(strNpcName) <= 0)
		return;

	//獲取指定角色的uid
	UINT uTrageID = GetNPCObjectIDByName(strNpcName);
	if (uTrageID != 0)
		Game_UseSkill(pt, uTrageID);
}

//對武器使用技能
void UseSkillByRoleWeapons(PSKILLPOINT pt, char* strWeaponsName)
{
	if (strlen(strWeaponsName) <= 0)
		return;

	//通過我們的物品欄查找我們的裝備信息
	PITEMPOINT pTrageItem = GetItemByName(strWeaponsName);
	if (pTrageItem)
	{
		Game_UseSkill(pt, pTrageItem->ulItemID);
		delete pTrageItem;
	}
}

//對防具使用技能
void UseSkillByRoleArmor(PSKILLPOINT pt, char* strArmorName)
{
	if (strlen(strArmorName) <= 0)
		return;

	//通過我們的物品欄查找我們的裝備信息
	PITEMPOINT pTrageItem = GetItemByName(strArmorName);
	if (pTrageItem)
	{
		Game_UseSkill(pt, pTrageItem->ulItemID);
		delete pTrageItem;
	}
}

//對物品使用技能(提煉魔石)
void UseSkillByRoleStone(PSKILLPOINT pt, char* strStoneName)
{
	if (strlen(strStoneName) <= 0)
		return;

	//通過我們的物品欄查找我們的物品信息<指向魔石>
	PITEMPOINT pTrageItem = GetItemByName(strStoneName);
	if (pTrageItem)
	{
		Game_UseSkill(pt, pTrageItem->ulItemID);
		delete pTrageItem;
	}
}
