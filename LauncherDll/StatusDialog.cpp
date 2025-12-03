// StatusDialog.cpp : ��{���
//

#include "stdafx.h"
#include "HelpTools.h"
#include "stdafx.h"
#include "StatusDialog.h"
#include "afxdialogex.h"


// CStatusDialog ��ܮ�

IMPLEMENT_DYNAMIC(CStatusDialog, CDialog)

CStatusDialog::CStatusDialog(CWnd* pParent /*=NULL*/)
: CPageDialog(CStatusDialog::IDD, pParent)
{
	m_ActiveTimer = false;
}

CStatusDialog::~CStatusDialog()
{
	::KillTimer(m_hWnd, TIMERID_STATUS);
}

void CStatusDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_AllStatusList);
	DDX_Control(pDX, IDC_LIST3, m_ActiveStatusList);
}


BEGIN_MESSAGE_MAP(CStatusDialog, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK1, &CStatusDialog::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON1, &CStatusDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CStatusDialog::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON2, &CStatusDialog::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON4, &CStatusDialog::OnBnClickedButton4)
END_MESSAGE_MAP()


// CStatusDialog �����B�z�{��


//��l�Ƶ��f
void CStatusDialog::InitializeWindow()
{
	char szKeyName[30] = { 0 };
	char szTempString[MAX_PATH] = { 0 };
	int nIndex = 0;

	//Ū���ڭ̪��t�m���
	do
	{
		wsprintfA(szKeyName, _T("Item%d"), nIndex++);
		if (0 >= GetPrivateProfileString(_T("AllState"), szKeyName, _T(""), szTempString, MAX_PATH - 1, _T("./LinHelperZ.ini")))
			break;

		m_vctStatusString.push_back(szTempString);
		memset(szKeyName, 0, 30);
		memset(szTempString, 0, MAX_PATH);

	} while (TRUE);

	//�[�J��ڭ̦ۤv���t�m���
	int nSize = m_vctStatusString.size();
	for (int i = 0; i < nSize; i++)
		m_AllStatusList.AddString(m_vctStatusString[i].c_str());

	//�Ы��ݩ�ڭ̪��w�ɾ���H
	::SetTimer(m_hWnd, TIMERID_STATUS, 1000, NULL);
}

//�w�ɾ����B�z���f���
void CStatusDialog::DispatachTimer()
{
	//�p�G�S���ҥΩw�ɾ��ڭ̤]�n��^
	if (!m_ActiveTimer)
		return;

	//�o�̶}�l�B�z�ڭ̪��޿豵�f
	if (!IsGameInitializeObject() || !QueryCurrentRoleType())
		return;

	TRACE("���A���f�w�ɾ��w�g�E��..\r\n");
	if (IsDlgButtonChecked(IDC_CHECK1))
		AutoAddRoleStatus();
}

void CStatusDialog::AutoAddRoleStatus()
{
	int nCount = m_ActiveStatusList.GetCount();
	if (nCount == 0)
		return;
	__try
	{
		//�}�l�M���Ҧ������A�H��
		for (int i = 0; i < nCount; i++)
		{
			char szStatusKey[30] = { 0 };
			char szStatusName[MAX_COMBOX_LEN] = { 0 };
			m_ActiveStatusList.GetText(i, szStatusName);
			char* pos = strstr(szStatusName, "_");
			if (pos == NULL)
				continue;

			//�o�ˤl�ڭ̴N�o��F���A�X
			strncpy(szStatusKey, szStatusName, pos - szStatusName);

			//�ڭ̴N�n���եh�����e�o�Ӫ��A�O�_�s�b
			if (!GetRoleState(_ttoi(szStatusKey)))
				CallGameFunction(++pos);	//�p�G�o�Ӫ��A���s�b���ܡA����ڭ̴N�n�ϥΫ��w�����~�W�[�ڭ̪����A
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{

	}

}

//�E���ڭ̩w�ɾ����ҵ{
void CStatusDialog::OnBnClickedCheck1()
{
	//�����ڭ̩w�ɾ��N�X
	m_ActiveTimer = (IsDlgButtonChecked(IDC_CHECK1)) ? true : false;
}


//�s�W���A
void CStatusDialog::OnBnClickedButton1()
{
	/*
	�޿��{�A�����P�_�ڭ̩Ҧ����A�O�_�S�Q�襤���A�p�G���A�h�ڭ��~��P�_�O�_�w�g�K�[��F�ڭ̪��E���C����A�̫�ާ@�ڭ̪��N�X
	*/
	char szCurSelTextStatus[MAX_COMBOX_LEN] = { 0 };

	int nSelect = m_AllStatusList.GetCurSel();
	if (nSelect == -1)
		return;

	if (0 >= m_AllStatusList.GetText(nSelect, szCurSelTextStatus))
		return;

	if (LB_ERR == m_ActiveStatusList.FindString(-1, szCurSelTextStatus))
		m_ActiveStatusList.AddString(szCurSelTextStatus);

	m_AllStatusList.SetCurSel(-1);

}

//�R�����A
void CStatusDialog::OnBnClickedButton3()
{
	int nSelect = m_ActiveStatusList.GetCurSel();
	if (nSelect == -1)
		return;

	//�p�G���īh�����襤�����A
	m_ActiveStatusList.DeleteString(nSelect);
	m_ActiveStatusList.SetCurSel(-1);

}

//�W��
void CStatusDialog::OnBnClickedButton2()
{
	char szCurSelTextStatus[MAX_COMBOX_LEN] = { 0 };

	int nSelect = m_ActiveStatusList.GetCurSel();
	if (nSelect <= 0)
		return;

	m_ActiveStatusList.GetText(nSelect, szCurSelTextStatus);

	//���R���A�M��A���J
	m_ActiveStatusList.DeleteString(nSelect);
	m_ActiveStatusList.InsertString(--nSelect, szCurSelTextStatus);
	m_ActiveStatusList.SetCurSel(nSelect);
}

//�U��
void CStatusDialog::OnBnClickedButton4()
{
	char szCurSelTextStatus[MAX_COMBOX_LEN] = { 0 };
	int nCount = m_ActiveStatusList.GetCount();

	int nSelect = m_ActiveStatusList.GetCurSel();
	//�p�G�S���襤�Ϊ̿襤����w�g�O�̫᪺��A���򪽱���^
	if (nSelect == -1 || nSelect == nCount - 1)
		return;

	//�_�h�ڭ̱N�o�ӦC��R����
	m_ActiveStatusList.GetText(nSelect, szCurSelTextStatus);

	//���R���A�M��A���J
	m_ActiveStatusList.DeleteString(nSelect);
	m_ActiveStatusList.InsertString(++nSelect, szCurSelTextStatus);
	m_ActiveStatusList.SetCurSel(nSelect);
}


//Ū���t�m�����f���>>�o�ӱ��f�u�|�եΤ@��
void CStatusDialog::ReadUserConfig()
{
	//�}�l�g�J�ڭ̪��ƾڤ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	//INI��|�� ��e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	//���U�ӧڭ�Ū���t�m��󪺼ƾ�
	int nCheck = GetPrivateProfileInt(_T("page3"), _T("check0"), 0, szIniPath);
	((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(nCheck);

	int nIndex = 0;
	do
	{
		char szKeyName[30] = { 0 };
		char szItemName[MAX_COMBOX_LEN] = { 0 };
		wsprintfA(szKeyName, _T("item%d"), nIndex++);
		if (GetPrivateProfileString(_T("page3"), szKeyName, "", szItemName, MAX_COMBOX_LEN - 1, szIniPath))
			m_ActiveStatusList.AddString(szItemName);
		else
			break;
	} while (TRUE);

	m_ActiveTimer = (IsDlgButtonChecked(IDC_CHECK1)) ? true : false;
}

//�g�J�t�m�����f���>>�o�ӱ��f�C�����û��U�{�ǳ��|�Q�ե�
void CStatusDialog::WriteUserConfig()
{
	//�}�l�g�J�ڭ̪��ƾڤ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	//INI��|�� ��e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);
	char* pcheckarray[2] = { "0", "1" };

	if (IsDlgButtonChecked(IDC_CHECK1))
		WritePrivateProfileString(_T("page3"), _T("check0"), pcheckarray[1], szIniPath);
	else
		WritePrivateProfileString(_T("page3"), _T("check0"), pcheckarray[0], szIniPath);

	//Ū���ڭ̪��C���
	int nListCount = m_ActiveStatusList.GetCount();

	for (int i = 0; i < nListCount; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("item%d"), i);

		char szTextName[MAX_COMBOX_LEN] = { 0 };
		m_ActiveStatusList.GetText(i, szTextName);
		WritePrivateProfileString(_T("page3"), szKeyName, szTextName, szIniPath);
	}

}
