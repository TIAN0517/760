// MainDialog.cpp : ��{���
//

#include "stdafx.h"
#include "HelpTools.h"
#include "stdafx.h"
#include "MainDialog.h"
#include "afxdialogex.h"


// CMainDialog ��ܮ�

IMPLEMENT_DYNAMIC(CMainDialog, CDialog)

CMainDialog::CMainDialog(CWnd* pParent /*=NULL*/)
: CDialog(CMainDialog::IDD, pParent)
{

}

CMainDialog::~CMainDialog()
{
}

void CMainDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_WINDOW, m_TabPage);
}


BEGIN_MESSAGE_MAP(CMainDialog, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_WINDOW, OnSelchangeTabWindow)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


//��l�Ƶ��f
VOID CMainDialog::OnInitializeWindow()
{
	//inset window
	m_TabPage.InsertItem(CHILD_WINDOW_GENERIC, "�`�W");
	m_TabPage.InsertItem(CHILD_WINDOW_STATE, "�Ĥ�");
	m_TabPage.InsertItem(CHILD_WINDOW_STATUS, "���A");
	m_TabPage.InsertItem(CHILD_WINDOW_CHANGE, "�S��");
	m_TabPage.InsertItem(CHILD_WINDOW_ITEMS, "���~");
	m_TabPage.InsertItem(CHILD_WINDOW_SPEAK, "�۸�");
	m_TabPage.InsertItem(CHILD_WINDOW_HOTKEY, "����");

	//create window
	m_pGenericDlg = new CGenericDialog(this);
	m_pGenericDlg->Create(IDD_DIALOG_GENERIC, GetDlgItem(IDC_TAB_WINDOW));

	m_pStateDlg = new CStateDialog(this);
	m_pStateDlg->Create(IDD_DIALOG_STATE, GetDlgItem(IDC_TAB_WINDOW));

	m_pStatusDlg = new CStatusDialog(this);
	m_pStatusDlg->Create(IDD_DIALOG_STATUS, GetDlgItem(IDC_TAB_WINDOW));

	m_pChangeDlg = new CChangeDialog(this);
	m_pChangeDlg->Create(IDD_DIALOG_CHANGE, GetDlgItem(IDC_TAB_WINDOW));

	m_pItemsDlg = new CItemsDialog(this);
	m_pItemsDlg->Create(IDD_DIALOG_ITEMS, GetDlgItem(IDC_TAB_WINDOW));

	m_pSpeakDlg = new CSpeakDialog(this);
	m_pSpeakDlg->Create(IDD_DIALOG_SPEAK, GetDlgItem(IDC_TAB_WINDOW));

	m_pHotKeyDlg = new CHotKeyDialog(this);
	m_pHotKeyDlg->Create(IDD_DIALOG_HOTKEY, GetDlgItem(IDC_TAB_WINDOW));


	//set window pos
	RECT TabRect = { 0 };
	m_TabPage.GetClientRect(&TabRect);
	TabRect.left += 5;
	TabRect.right -= 5;
	TabRect.top += 25;
	TabRect.bottom -= 25;

	//mov child windos
	m_pGenericDlg->MoveWindow(&TabRect);
	m_pStateDlg->MoveWindow(&TabRect);
	m_pStatusDlg->MoveWindow(&TabRect);
	m_pChangeDlg->MoveWindow(&TabRect);
	m_pItemsDlg->MoveWindow(&TabRect);
	m_pSpeakDlg->MoveWindow(&TabRect);
	m_pHotKeyDlg->MoveWindow(&TabRect);

	//Show First Window
	m_pGenericDlg->ShowWindow(SW_SHOW);
	m_pStateDlg->ShowWindow(SW_HIDE);
	m_pStatusDlg->ShowWindow(SW_HIDE);
	m_pChangeDlg->ShowWindow(SW_HIDE);
	m_pItemsDlg->ShowWindow(SW_HIDE);
	m_pSpeakDlg->ShowWindow(SW_HIDE);
	m_pHotKeyDlg->ShowWindow(SW_HIDE);

	m_TabPage.SetCurSel(CHILD_WINDOW_GENERIC);
}

//��ܫH��
VOID CMainDialog::ShowMessage(LPCTSTR strBuff)
{
	if (IsWindow(hStatusWnd))
	{
		::SetWindowText(hStatusWnd, strBuff);
	}
}

//�۩w�q�����B�z
LRESULT CMainDialog::OnUpdateMessage(WPARAM wParam, LPARAM lParam)
{
	CString strMsg;
	if (wParam)
	{
		strMsg.Format("�w�}�� %s", lParam);
	}
	else
	{
		strMsg.Format("�w���� %s", lParam);
	}
	//WPARAM �}�ҩΪ�����
	ShowMessage(strMsg.GetBuffer(0));
	strMsg.ReleaseBuffer();

	return 0;
}

void CMainDialog::OnSelchangeTabWindow(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	RECT TabRect = { 0 };
	m_TabPage.GetClientRect(&TabRect);
	TabRect.left += 5;
	TabRect.right -= 5;
	TabRect.top += 25;
	TabRect.bottom -= 25;

	//�����ǱƤU�h �`�W �Ĥ� ���A �S�� ���~ �۸� ����
	switch (m_TabPage.GetCurSel())
	{
	case CHILD_WINDOW_GENERIC:
		m_pGenericDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_SHOWWINDOW);
		m_pStateDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pStatusDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pChangeDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pItemsDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pSpeakDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pHotKeyDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		break;
	case CHILD_WINDOW_STATE:
		m_pGenericDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pStateDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_SHOWWINDOW);
		m_pStatusDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pChangeDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pItemsDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pSpeakDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pHotKeyDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		break;
	case CHILD_WINDOW_STATUS:
		m_pGenericDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pStateDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pStatusDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_SHOWWINDOW);
		m_pChangeDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pItemsDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pSpeakDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pHotKeyDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		break;
	case CHILD_WINDOW_CHANGE:
		m_pGenericDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pStateDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pStatusDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pChangeDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_SHOWWINDOW);
		m_pItemsDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pSpeakDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pHotKeyDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		break;
	case CHILD_WINDOW_ITEMS:
		m_pGenericDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pStateDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pStatusDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pChangeDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pItemsDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_SHOWWINDOW);
		m_pSpeakDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pHotKeyDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		break;
	case CHILD_WINDOW_SPEAK:
		m_pGenericDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pStateDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pStatusDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pChangeDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pItemsDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pSpeakDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_SHOWWINDOW);
		m_pHotKeyDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		break;
	case CHILD_WINDOW_HOTKEY:
		m_pGenericDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pStateDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pStatusDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pChangeDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pItemsDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pSpeakDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_HIDEWINDOW);
		m_pHotKeyDlg->SetWindowPos(NULL, TabRect.left, TabRect.top, TabRect.right - TabRect.left, TabRect.bottom - TabRect.top, SWP_SHOWWINDOW);
		break;
	}

	*pResult = 0;
}


BOOL CMainDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	//�o�̽եΧڭ̵��f�i���l��
	OnInitializeWindow();

	return TRUE;  // return TRUE unless you set the focus to a control
	// ���`: OCX �ݩʭ�����^ FALSE
}


void CMainDialog::OnClose()
{
	//�o�̧ڭ̻ݭn�B�z�@�U�A���n���Τ�P�����U
	//�o�̧ڭ̻ݭn�����ާ@�O���g�J�ڭ̷�e�Τ᪺�t�m���ƾ�
	ShowWindow(SW_HIDE);
	TRACE("�ǳƨ�s���a�t�m���!\r\n");
	//���U�Ӷ}�l�g�t�m���

	return SaveUserConfig();
	//	CDialog::OnClose();
}


void CMainDialog::OnDestroy()
{
	CDialog::OnDestroy();
	TRACE("�D���f�w�g����!�}�l�h�X���U�t��\r\n");
	SaveUserConfig();

	if (m_pGenericDlg)
		delete m_pGenericDlg;

	if (m_pStateDlg)
		delete m_pStateDlg;

	if (m_pStatusDlg)
		delete m_pStatusDlg;

	if (m_pChangeDlg)
		delete m_pChangeDlg;

	if (m_pItemsDlg)
		delete m_pItemsDlg;

	if (m_pSpeakDlg)
		delete m_pSpeakDlg;

	if (m_pHotKeyDlg)
		delete m_pHotKeyDlg;

}

//�O�s�Τ�t�m���
void CMainDialog::SaveUserConfig(void)
{
	//���U�Ӷ}�l�g�t�m���
	char* pRoleName = GetCurrentRoleName();
	if (pRoleName && strlen(pRoleName))
	{
		char szIniPath[MAX_PATH] = { 0 };
		char* pstrRoleName = GetCurrentRoleName();

		//INI��|�� ��e�ؿ�/Me(����W�r).ini
		wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);
		DeleteFile(szIniPath);

		TRACE("�}�l��s���a�t�m���!\r\n");
		//�p�G�s�b�o�Ө���A�ڭ̤~�}�l�]�m
		m_pGenericDlg->WriteUserConfig();
		m_pStateDlg->WriteUserConfig();
		m_pStatusDlg->WriteUserConfig();
		m_pChangeDlg->WriteUserConfig();
		m_pItemsDlg->WriteUserConfig();
		m_pSpeakDlg->WriteUserConfig();
		m_pHotKeyDlg->WriteUserConfig();
	}
}
