#pragma once
#include "afxwin.h"

#include <string>
#include <vector>
#include "GameMethods.h"
#include "Util.h"
#include "Projectdef.h"

using namespace std;

class CPageDialog : public CDialog
{
	DECLARE_DYNAMIC(CPageDialog)
public:
	CPageDialog(void);
	CPageDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL)
	{
		m_ActiveTimer = false;
		__super::CDialog(nIDTemplate, pParentWnd);
	}

	virtual ~CPageDialog(void);

public:
	//定時器的處理接口函數
	virtual void DispatachTimer(){ NULL; }
	//初始化窗口的接口函數
	virtual void InitializeWindow(){ NULL; }
	//讀取配置的接口函數>>這個接口只會調用一次
	virtual void ReadUserConfig(){ NULL; }
	//寫入配置的接口函數>>這個接口每次隱藏輔助程序都會被調用
	virtual void WriteUserConfig(){ NULL; }
public:
	//是否激活定時器
	bool				m_ActiveTimer;

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

