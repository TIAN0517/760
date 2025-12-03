#pragma once
#include "PageDialog.h"
#include "afxwin.h"

// CHotKeyDialog 對話框

class CHotKeyDialog : public CPageDialog
{
	DECLARE_DYNAMIC(CHotKeyDialog)

public:
	CHotKeyDialog(CWnd* pParent = NULL);   // 標準構造函數
	virtual ~CHotKeyDialog();

	// 對話框數據
	enum { IDD = IDD_DIALOG_HOTKEY };
private:
	//初始化窗口
	void InitializeWindow();
public:
	//分發派遣我們快捷鍵窗口
	bool DispatachSysHotKey(int nIndex);
private:
	//是否激活我們的熱鍵消息
	bool m_bIsHotKey[5];
public:
	//讀取配置的接口函數>>這個接口只會調用一次
	virtual void ReadUserConfig();
	//寫入配置的接口函數>>這個接口每次隱藏輔助程序都會被調用
	virtual void WriteUserConfig();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedCheck5();
	afx_msg void OnBnClickedCheck7();
	afx_msg void OnBnClickedCheck9();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnEnChangeEdit14();
	afx_msg void OnEnChangeEdit13();
};
