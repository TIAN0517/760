#pragma once
//#include "i:\microsoft visual studio 10.0\vc\atlmfc\include\afxcmn.h"

#include <string>
#include <iostream> 
#include <list> 
#include <numeric> 
#include <algorithm>
#include <Commdlg.h>

using namespace std;

// CPatachDlg 對話框

class CPatachDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(CPatachDlg)

public:
	CPatachDlg(CWnd* pParent = NULL);   // 標準構造函數
	virtual ~CPatachDlg();

	// 對話框數據
	enum { IDD = IDD_PATACH_DIALOG };
private:
	//添加大量文件
	VOID OnAddFiles(TCHAR* szNewFilePath);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 文件列表
	CListCtrl m_list;
	virtual BOOL OnInitDialog();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton5();
	CListCtrl m_list2;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton6();
};
