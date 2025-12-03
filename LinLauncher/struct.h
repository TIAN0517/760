#pragma once

// 基本數據類型定義
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long long QWORD;

// 遊戲相關結構體定義
struct GAME_INFO {
    DWORD dwGameBase;
    DWORD dwGameSize;
    char szGamePath[MAX_PATH];
};

// 其他必要的結構體定義
struct ITEM_INFO {
    DWORD dwItemID;
    char szItemName[64];
    DWORD dwItemCount;
};

// 如果需要更多結構體定義，請在此添加
