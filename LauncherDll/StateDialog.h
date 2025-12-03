#pragma once
#include "afxwin.h"
#include "PageDialog.h"


// CStateDialog 對話框

class CStateDialog : public CPageDialog
{
	DECLARE_DYNAMIC(CStateDialog)

public:
	CStateDialog(CWnd* pParent = NULL);   // 標準構造函數
	virtual ~CStateDialog();

	// 對話框數據
	enum { IDD = IDD_DIALOG_STATE };
private:
	//我們從配置列表中讀取到的數據數組
	vector<string> m_vctAllHPString;

	vector<string> m_vctAllHPMPString;
	//瞬間移動卷軸名字
	string			  m_strMoveItemString;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	//初始化窗口
	void InitializeWindow();
	//定時器的處理接口函數
	void DispatachTimer();

	//啟用保護調用例程
	void EnableProtectRole();
	//啟用生存的吶喊調用例程
	void EnableSurvivalRole();
	//啟用HP第一個條件判斷
	void EnableHP1Protect();
	//啟用HP第一個條件判斷
	void EnableHP2Protect();
	//啟用HP第一個條件判斷
	void EnableHP3Protect();
	//啟用HP第一個條件判斷
	void EnableHP4Protect();
	//啟用HP第一個條件判斷
	void EnableHP5Protect();
	//啟用洗魔調用例程
	void EnableHPMPProtect();
private:
	ULONG32		m_nRoleHP;
	ULONG32		m_nRoleMP;
public:
	//讀取配置的接口函數>>這個接口只會調用一次
	virtual void ReadUserConfig();
	//寫入配置的接口函數>>這個接口每次隱藏輔助程序都會被調用
	virtual void WriteUserConfig();

public:
	// 保護欄中的物品combox對象
	CComboBox m_ProtectItemCombox;
	// 祝瞬的下拉框
	CComboBox m_ProtectStatusCombox;

	CComboBox m_AllHPCombox1;
	CComboBox m_AllHPCombox2;
	CComboBox m_AllHPCombox3;
	CComboBox m_AllHPCombox4;
	CComboBox m_AllHPCombox5;
	CComboBox m_AllHPCombox6;

	afx_msg void OnBnClickedButton1();
	afx_msg void OnCbnDropdownCombo1();
	afx_msg void OnCbnDropdownCombo2();
};
