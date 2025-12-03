#pragma once
#include "PageDialog.h"
#include "afxwin.h"

// CChangeDialog 對話框

class CChangeDialog : public CPageDialog
{
	DECLARE_DYNAMIC(CChangeDialog)

public:
	CChangeDialog(CWnd* pParent = NULL);   // 標準構造函數
	virtual ~CChangeDialog();

	// 對話框數據
	enum { IDD = IDD_DIALOG_CHANGE };
private:
	//我們從配置列表中讀取到的怪物數組[只是怪物call名字]
	vector<string> m_vctAllNameString;
	vector<string> m_vctAllMonsterString;
	//變身物品
	vector<string> m_vctAllChangeItemString;
	//解毒物品
	vector<string> m_vctAllPoisonItemString;
	//石頭物品
	vector<string> m_vctAllStoneItemString;
	//娃娃數組
	vector<string> m_vctAllBabyItemString;//這個成員不用
	//提煉魔石的技能名字
	string			  m_strStoneSkillString;
private:
	void InitializeWindow();
	//定時器的處理接口函數
	void DispatachTimer();

	//自動變身處理代碼
	void AutoChangeRole();
	//處理綠毒的代碼
	void RemoveColorPoisoning();
	//處理卡毒的代碼
	void RemoveKoerPoisoning();
	//提煉魔石的代碼
	void LoadStoneItem();
	//處理魔法娃娃的代碼
	void BabyChangeRole();

public:
	//讀取配置的接口函數>>這個接口只會調用一次
	virtual void ReadUserConfig();
	//寫入配置的接口函數>>這個接口每次隱藏輔助程序都會被調用
	virtual void WriteUserConfig();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 變身物品對象
	CComboBox m_ChangeItemCombox;
	// 變身角色對象
	CComboBox m_ChangeRoleCombox;
	// 變身其他道具對象
	CComboBox m_ChangeOthItemCombox;
	// 綠毒combox
	CComboBox m_ColorpoisonCombox;
	// 卡毒對象
	CComboBox m_KoerpoisonCombox;
	// 石頭
	CComboBox m_StoneCombox;
	// 魔法娃娃combox
	CComboBox m_BadyItemCombox;


	afx_msg void OnCbnSetfocusCombo9();
	afx_msg void OnCbnSetfocusCombo13();
	// 自動變身
	CButton m_AutoChangeCheck;
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnDropdownCombo13();
	afx_msg void OnDropdownCombo9();
	afx_msg void OnCbnSelchangeCombo13();
};
