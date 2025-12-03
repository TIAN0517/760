#pragma once
#include "PageDialog.h"


// CStatusDialog 對話框

class CStatusDialog : public CPageDialog
{
	DECLARE_DYNAMIC(CStatusDialog)

public:
	CStatusDialog(CWnd* pParent = NULL);   // 標準構造函數
	virtual ~CStatusDialog();

	// 對話框數據
	enum { IDD = IDD_DIALOG_STATUS };
private:
	//我們從配置列表中讀取到的數據數組
	vector<string> m_vctStatusString;
private:
	//初始化窗口
	void InitializeWindow();
	//定時器的處理接口函數
	void DispatachTimer();
	//自動增加狀態接口
	void AutoAddRoleStatus();
public:
	//讀取配置的接口函數>>這個接口只會調用一次
	virtual void ReadUserConfig();
	//寫入配置的接口函數>>這個接口每次隱藏輔助程序都會被調用
	virtual void WriteUserConfig();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 所有的狀態列表
	CListBox m_AllStatusList;
	CListBox m_ActiveStatusList;

	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton4();
};
