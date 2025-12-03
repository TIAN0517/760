#pragma once

#include <Windows.h>

//遊戲所有的接口調用方法全在這裡
//這些接口都是不安全的，請繼續用一場保護才可調用，適用於0x76遊戲版本

#define GAME_VERSION_076


#define GAME_BASE_ADDRESS		0x11C6600
#define GAME_ITEM_ADDRESS		0x1308D68
#define GAME_STATUS_ADDRESS	0x135D648
#define GAME_SKILL_ADDRESS		0x135C358
#define GAME_SKILL_PAMADDR		0x135EEC8
#define GAME_SKILL_FINDADDR		0x0CC75A0		//查找技能call
#define GAME_SKILL_EXTENADDR	0x0CC6D00		//是否學習技能call
#define GAME_SKILL_ENTRYADDR	0x0CC8710		//進入施法狀態
#define GAME_SKILL_STARTADDR	0x0CC88A0		//開始施法狀態
#define GAME_SKILL_ISUSEADDR   0x0CC6A90		//技能是否可以被使用
#define GAME_SPEAK_BASEADDR	0x011C78A8		//喊話參數基址
#define GAME_ROLE_LIFECHECK1	0x011A8FDC		//生命的吶喊1
#define GAME_ROLE_LIFECHECK2	0x0135FE58		//生命的吶喊2
#define GAME_ROLE_LIFECALL		0x00A33AF0		//生命的吶喊遊戲功能地址		
#define GAME_INFO_SHOWPARAM	0x0118D138		//我們顯示信息的參數地址
#define GAME_INFO_SHOWFUNC		0x0075C0D0		//我們信息輸出的地址

#define GAME_ROLE_CURRENTHP	0x1308F20
#define GAME_ROLE_CURRENTMP	0x1308F2C
#define GAME_ROLE_CURRENTTYPE 0x1308E3C
#define GAME_ROLE_BASEADDR		0x1359D3C		//自身角色對象基址
#define GAME_ROLE_SATURATE		0x11C78A4		//獲取人物的飽和度
#define GAME_NPC_NPCNUMBER	0x1359D54		//周圍對象數組數量
#define GAME_NPC_BASEADDR		0x1359D5C		//周圍怪物數組，包括自己,如果這個值為0則會掉線
#define GAME_INITIALIZEOBJECT		0x118C108
#define GAME_INITIALIZERELO		    0x1359D54
#define GAME_MOUSE_NPCADDR	0x1360CD8		//鼠標選中的NPC
#define GAME_STATE_FUNADDR		0x0D9DF10
#define GAME_ROLE_BABYCHANGE 0x135F5A9
#define GAME_ROLE_WEAPONDAMAGE	0x11EA5D8			//武器破損狀態檢測基址
#define GAME_ROLE_EXPBASE		 0x11C78A4				//經驗基址
#define GAME_ROLE_LEVELBASE	 0x11EA5D8				//等級基址
#define GAME_SHOW_BUFFERADDR	0x1B5D790			//遊戲輸出的緩衝區

//遊戲076版本所有的功能接口call對象
#define GAME_ITEM_FUNCTION		0x0966060
#define GAME_SPEAK_FUNCTION	0x064CA50

//遊戲狀態下標查詢
#define GAME_STATE_CHANGEROLE			0x27
#define GAME_STATE_COLORPOISON1		0x187
#define GAME_STATE_COLORPOISON2		0x182
#define GAME_STATE_KOERPOISON1			0x17D
#define GAME_STATE_KOERPOISON2			0x1E9
#define GAME_STATE_NORMALROLE			0x52


typedef struct GAME_ITEMPOINT
{
	GAME_ITEMPOINT()
	{
		memset(this, 0, sizeof(GAME_ITEMPOINT));
	}
	ULONG		ulIndex;
	//	char*		strFlags;
	ULONG		ulItemID;
	ULONG		ulCount;
	char*		strName;
}ITEMPOINT, *PITEMPOINT;

typedef struct GAME_SKILLPOINT
{
	GAME_SKILLPOINT()
	{
		memset(this, 0, sizeof(GAME_SKILLPOINT));
	}

	ULONG ulIndex;				//技能指針
	ULONG ulSkillID;			//技能ID
	ULONG ulUnkown;			//未知，暫時不使用
	char*	strName;			//技能名字指針
}SKILLPOINT, *PSKILLPOINT;

typedef struct GAME_STATUSPOINT
{
	GAME_STATUSPOINT()
	{
		memset(this, 0, sizeof(GAME_STATUSPOINT));
	}

	ULONG		ulIndex;						//移動的指針
	ULONG		ulTransmitID;				//這個指針的ID號
	ULONG		ulValueX;					//這裡是坐標X
	ULONG		ulValueY;					//這裡是坐標Y
	char*		strName;
}STATUSPOINT, *PSTATUSPOINT;

//顯示白天
void SwitchAllDay(bool bIsShow = true);

//cpu hook
void SwitchCPU(bool bIsLow = true);

//海底抽水
void SwitchWater(bool bIsWater = true);

//怪物等級色彩
void SwitchMonsterLevelColor(bool bIsShow = true);

//顯示天堂時間
void SwitchGameTimer(bool bIsShow = true);

//顯示傷害值
void SwitchAttackLevelColor(bool bIsShow = true);

//解除右鍵鎖定
void SwitchRButtonLock(bool bIsNoLock = true);

//顯示經驗
void SwitchEXPLevelColor(bool bIsShow = true);

//自動吃肉

//自動修刀

//遊戲喊話call
void Game_CallSpeak(char* strSpeakData);
//遊戲吶喊call
void Game_CallRoleLife();
//遊戲輸出call
void Game_ShowMessage(char* strShowText);


//輔助接口

//遊戲是否已經初始化
bool IsGameInitializeObject();

//當前遊戲角色類型[戰士 騎士 精靈 等等]
UINT QueryCurrentRoleType();
//獲取當前遊戲角色名字
char* GetCurrentRoleName();
//獲取當前角色狀態（其他狀態）
byte GetCurrentRoleOthState();
//獲取當前遊戲角色經驗
UINT32 GetCurrentRoleExp();
//獲取當前遊戲角色等級
UINT32 GetCurrentRoleLevel();

//獲取人物的飽和度
byte GetRoleSaturate();

//獲取當前遊戲角色狀態
BOOL GetRoleState(UINT nStateIndex);
//獲取當前角色變身狀態
UINT32 GetRoleChangeState();
//是否可以合成魔石
bool IsUseStoneItem();
//是否可以使用技能
bool IsUseRoleSkill();
//是否處於破損武器狀態
bool IsWeaponsDamage();
//是否可以使用魔法娃娃
bool IsUseBabyItem();


//獲取當前物品欄最大數量
UINT32 GetItemsCount();

//通過指定的下標獲取指定的物品對象,失敗返回NULL
PITEMPOINT GetItemByIndex(UINT32 nIndex);

//通過指定的物品名獲取指定的物品對象,失敗返回NULL
PITEMPOINT GetItemByName(char* strName);

//獲取物品欄中裝備的武器的物品對象，失敗返回NULL 當前接口不使用
PITEMPOINT GetItemByRoleAttachWeapons();

//獲取物品欄中指定類型的物品對象，失敗返回NULL
PITEMPOINT GetItemBySpecifiedTypeAndKeyName(byte nItemType, char* strKeyName);



//獲取低耐久度裝備的物品指針
PITEMPOINT GetLowRepairRoleItem(char* pKeyName = _T("揮舞"));

//底層接口，一般不建議直接使用
//使用物品call
void Game_UseItem(PITEMPOINT pt);
//使用變身物品call
void Game_UseChangeItem(PITEMPOINT pt, const char* strChangeRole);
//使用Oth物品call
void Game_UseOthItem(PITEMPOINT pt, UINT32 nTrageID);
//使用卷軸物品call
void Game_UseTestItem(PITEMPOINT pt, UINT32 nTrageID);
//使用指定位置傳送
void Game_UsePostionItem(PITEMPOINT pt, PSTATUSPOINT pst);
//刪除物品call
void Game_DelItem(PITEMPOINT pt);
//熔煉物品call
void Game_SmeltItem(PITEMPOINT pt);


//使用技能call->用於不需要選人的技能，即放即施法的
void Game_UseSkill(PSKILLPOINT pt);
//使用技能call->用於需要指定對象的
void Game_UseSkill(PSKILLPOINT pt, UINT32 nTrageID);



//獲取當前技能最大數量
UINT32 GetSkillCount();

//通過指定的下標獲取指定的技能對象，失敗返回NULL[內部自動判定技能是否學會]
PSKILLPOINT GetSkillByIndex(UINT32 nIndex);

//通過指定的名字獲取自定的技能對象，失敗返回NULL[內部自動判定技能是否學會]
PSKILLPOINT GetSkillByName(char* strName);

//技能是否可以被使用
bool Game_IsUseSkill(UINT32 nIndex);


//獲取當前祝瞬數量
UINT32 GetStatusCount();

//通過指定的下標獲取指定的祝瞬對象
PSTATUSPOINT GetStatusIndex(UINT32 nIndex);

//通過指定的名字獲取指定的祝瞬對象
PSTATUSPOINT	 GetStatusByName(char* strname);

//獲取當前角色的血藍信息
UINT32 GetGameRoleHPMP(ULONG ulAddress);

//獲取當前角色對象ID
UINT32 GetRoleObjectID();
//獲取當前NPC數組數量
UINT32 GetNPCCount();
//獲取當前NPC對象ID[非自己]
UINT32 GetNPCObjectIDByName(char* strNPCname);
//獲取鼠標指向的對象ID
UINT32 GetMouseTrageObjectID();


//封裝接口

//調用遊戲函數>>這個接口提供給上上層使用
void CallGameFunction(char* strname);

//使用物品
void UseItem(char* strItemName);
//使用技能
void UseSkill(char* strSkillName);

//---------------->邏輯來說以下接口不可直接調用------------->||
//對角色使用物品
void UseItemByRole(PITEMPOINT pt);
//對鼠標指向的NPC使用物品
void UseItemByMouse(PITEMPOINT pt);
//對指定名字的NPC使用物品
void UseItemByTrageNPC(PITEMPOINT pt, char* strNpcName);
//對武器使用物品
void UseItemByRoleWeapons(PITEMPOINT pt, char* strWeaponsName);
//對防具使用物品
void UseItemByRoleArmor(PITEMPOINT pt, char* strArmorName);
//對裝備使用熔煉物品 參數1 我們熔煉物品的指針 參數2，我們需要熔煉的物品名字
void UseItemBySmeltRoleItem(PITEMPOINT pt, char* strItemName);
//對裝備使用磨刀石[參數1 我們磨刀石的指針 參數2我們需要修理裝備的裝備指針]
void UseItemByRepairRoleItem(PITEMPOINT pt, PITEMPOINT pTrage);
//對角色使用瞬移坐標[]
void UseItemBySavePostionName(PITEMPOINT pt, char* strposname);

//對角色使用技能
void UseSkillByRole(PSKILLPOINT pt);
//對於需要二次確定的技能調用這裡
void UseSkillByRoleEx(PSKILLPOINT pt);
//對鼠標指向的NPC使用技能
void UseSkillByMouse(PSKILLPOINT pt);
//對指定名字的NPC使用技能
void UseSkillByTrageNPC(PSKILLPOINT pt, char* strNpcName);
//對武器使用技能
void UseSkillByRoleWeapons(PSKILLPOINT pt, char* strWeaponsName);
//對防具使用技能
void UseSkillByRoleArmor(PSKILLPOINT pt, char* strArmorName);
//對物品使用技能(提煉魔石)
void UseSkillByRoleStone(PSKILLPOINT pt, char* strStoneName);

/*
76版本的技能數據
$ ==>    >00000010  技能ID
$+4      >00000010
$+8      >00000014	技能消耗的體力
$+C      >00000000   技能消耗的魔力
$+10     >00000000
$+14     >00000001
$+18     >00000001
$+1C     >00000002
$+20     >00000000
$+24     >00000000
$+28     >00000000
$+2C     >00000006  技能攻擊的目標最大數
$+30     >00000189
$+34     >00000157
$+38     >000001F4
$+3C     >00010001
$+40     >00000000
$+44     >00000000
$+48     >00000000
$+4C     >00000000
$+50     >00000000
$+54     >00000000
$+58     >00000000
$+5C     >00000000
$+60     >00000000
$+64     >11F75248
$+68     >00000000
$+6C     >E2B9ABBC 技能的名字


*/


