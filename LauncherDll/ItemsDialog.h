#pragma once
#include "afxwin.h"
#include "PageDialog.h"

// CItemsDialog 對話框

class CItemsDialog : public CPageDialog
{
	DECLARE_DYNAMIC(CItemsDialog)

public:
	CItemsDialog(CWnd* pParent = NULL);   // 標準構造函數
	virtual ~CItemsDialog();

	// 對話框數據
	enum { IDD = IDD_DIALOG_ITEMS };

private:
	//我們從配置列表中讀取到的數據數組
	//vector<string> m_vctAllHPString;
private:
	void InitializeWindow();
	//定時器的處理接口函數
	void DispatachTimer();

	//自動處理我們刪除的物品
	void AutoDelItemList();
	//自動處理我們熔煉的物品
	void AutoSmeltItemList();

	//這裡保存一下我們熔煉劑的名字
	string m_strSmeltItemName;
public:
	//讀取配置的接口函數>>這個接口只會調用一次
	virtual void ReadUserConfig();
	//寫入配置的接口函數>>這個接口每次隱藏輔助程序都會被調用
	virtual void WriteUserConfig();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSetfocusCombo1();
	// 當前物品列表
	CComboBox m_ItemsCombox;
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	// 我們活動的物品列表
	CListBox m_ActiveItemList;
};
