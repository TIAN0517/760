#pragma once
#include "PageDialog.h"
#include "afxwin.h"

// CGenericDialog 對話框

class CGenericDialog : public CPageDialog
{
	DECLARE_DYNAMIC(CGenericDialog)

public:
	CGenericDialog(CWnd* pParent = NULL);   // 標準構造函數
	virtual ~CGenericDialog();

	// 對話框數據
	enum { IDD = IDD_DIALOG_GENERIC };
private:
	//定時器的處理接口函數
	virtual void DispatachTimer();
	//初始化窗口的接口函數
	virtual void InitializeWindow();

	//自動修理裝備
	void AutoRepairRoleItem();
	//自動吃肉
	void AutoSaturateRole();
	//自動顯示經驗
	void AutoShowExpRole();

	//自動加載所有用戶選擇項目
	void LoadUserCheck();

	//自動修理裝備的物品和肉的物品
	string m_RepairItemName;
	string m_SaturateItemName;
public:
	//讀取配置的接口函數>>這個接口只會調用一次
	virtual void ReadUserConfig();
	//寫入配置的接口函數>>這個接口每次隱藏輔助程序都會被調用
	virtual void WriteUserConfig();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnBnClickedCheck5();
	afx_msg void OnBnClickedCheck6();
	afx_msg void OnBnClickedCheck7();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnBnClickedCheck10();
	afx_msg void OnBnClickedCheck9();
};
