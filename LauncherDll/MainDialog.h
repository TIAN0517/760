#pragma once
#include "afxcmn.h"
#include "Projectdef.h"

//子窗口對象

#include "GenericDialog.h"
#include "StateDialog.h"
#include "StatusDialog.h"
#include "ChangeDialog.h"
#include "ItemsDialog.h"
#include "SpeakDialog.h"
#include "HotKeyDialog.h"


class CMainDialog : public CDialog
{
	DECLARE_DYNAMIC(CMainDialog)

public:
	CMainDialog(CWnd* pParent = NULL);   // 標準構造函數
	virtual ~CMainDialog();

	// 對話框數據
	enum { IDD = IDD_DIALOG_MAIN };
public:
	//初始化窗口
	VOID OnInitializeWindow();
	//顯示信息
	VOID ShowMessage(LPCTSTR strBuff);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	afx_msg void OnSelchangeTabWindow(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnUpdateMessage(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	// 我們分頁切換的窗口控件
	CTabCtrl m_TabPage;

	//常規窗口
	CGenericDialog* m_pGenericDlg;
	//任務狀態窗口
	CStateDialog*	  m_pStateDlg;
	//任務狀態窗口
	CStatusDialog*	  m_pStatusDlg;
	//人物變身窗口
	CChangeDialog* m_pChangeDlg;
	//任務物品窗口
	CItemsDialog*	  m_pItemsDlg;
	//任務喊話窗口
	CSpeakDialog*	  m_pSpeakDlg;
	//系統熱鍵窗口
	CHotKeyDialog*	  m_pHotKeyDlg;
public:
	//保存用戶配置文件
	void SaveUserConfig(void);
private:
	//狀態條窗口
	HWND hStatusWnd;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
};
