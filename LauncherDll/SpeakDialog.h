#pragma once
#include "PageDialog.h"
#include "afxwin.h"

// CSpeakDialog 對話框

class CSpeakDialog : public CPageDialog
{
	DECLARE_DYNAMIC(CSpeakDialog)

public:
	CSpeakDialog(CWnd* pParent = NULL);   // 標準構造函數
	virtual ~CSpeakDialog();

	// 對話框數據
	enum { IDD = IDD_DIALOG_SPEAK };
private:
	//初始化窗口
	void InitializeWindow();
	//定時器的處理接口函數
	void DispatachTimer();
	//自動喊話
	void AutoSpeak();
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
	// 喊話列表
	CListBox m_SpeakList;
	// 激活的秒數
	DWORD m_nActiveSec;
	// 添加喊話的編輯框
	CEdit m_SpeakEdit;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
};
