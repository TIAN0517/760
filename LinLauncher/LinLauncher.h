#if !defined(AFX_LOGIN_H__9445BE80_959E_43C6_9E8A_3373838242C0__INCLUDED_)
#define AFX_LOGIN_H__9445BE80_959E_43C6_9E8A_3373838242C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "../Share/ShareMemory.h"
#include <vector>

struct Server_Info;

typedef struct
{
	TCHAR filename[MAX_PATH];
	int iIndex;
} Update_Info;

class CLinLauncherDlg : public CWindowWnd, public INotifyUI
{
public:
	CLinLauncherDlg();
	~CLinLauncherDlg();
public:
	static bool LoadConfig();
public:
	LPCTSTR GetWindowClassName() const;
	UINT GetClassStyle();
	void OnFinalMessage(HWND hWnd);
	void Init();
	void OnPrepare();
	void Notify(TNotifyUI& msg);
	void OnSelectServer(CControlUI* pSender);
	void StartGame();
	void GetHttpFile(const TCHAR* url, const TCHAR* filename, TCHAR* result);
	void GetServerList();
	void LoadServerList(const TCHAR* filename);
	void RebuildGameResource();
	void GetUpdateList();
	int CheckUpdateFile();
	void LoadUpdateList(const TCHAR* filename);
	bool unZipPackage(TCHAR* pszFilePath, TCHAR* pszOutputPath);
	bool UpdateFile();
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	CPaintManagerUI m_pm;

	CLabelUI* m_pLabelCaption;

	CButtonUI* m_pBtnMin;
	CButtonUI* m_pBtnClose;
	CButtonUI* m_pBtnStart;
	CButtonUI* m_pBtnExit;

	// 舊版 XML 可能提供 5 個連結按鈕，原陣列長度 4 會造成越界
	CButtonUI* m_pBtnLinks[5];
	CButtonUI* m_pBtnServer[8];

	CProgressUI* m_pProgressCur;
	CProgressUI* m_pProgressAll;
	CProgressUI* m_pProgressState;

	CActiveXUI* m_pWeb;

	int m_nVer;

	int m_nSelectIndex;
	int m_nServerCount;
	Server_Info si[8];

	int m_nUpdateCount;
	TCHAR m_szUpdateBase[MAX_PATH];
	Update_Info* m_vtAllFile;
	std::vector<Update_Info*> m_vtNeedUpdateFile;
};

#endif // !defined(AFX_LOGIN_H__9445BE80_959E_43C6_9E8A_3373838242C0__INCLUDED_)