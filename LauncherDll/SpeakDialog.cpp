// SpeakDialog.cpp : ��{���
//

#include "stdafx.h"
#include "HelpTools.h"
#include "stdafx.h"
#include "SpeakDialog.h"
#include "afxdialogex.h"


// CSpeakDialog ��ܮ�

IMPLEMENT_DYNAMIC(CSpeakDialog, CDialog)

CSpeakDialog::CSpeakDialog(CWnd* pParent /*=NULL*/)
: CPageDialog(CSpeakDialog::IDD, pParent)
, m_nActiveSec(2000)
{
	m_ActiveTimer = false;
}

CSpeakDialog::~CSpeakDialog()
{
	::KillTimer(m_hWnd, TIMERID_SPEAK);
}

void CSpeakDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_SpeakList);
	DDX_Text(pDX, IDC_EDIT1, m_nActiveSec);
	//DDV_MinMaxUInt(pDX, m_nActiveSec, 2000, 60000);
	DDX_Control(pDX, IDC_EDIT10, m_SpeakEdit);
}


BEGIN_MESSAGE_MAP(CSpeakDialog, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK1, &CSpeakDialog::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON1, &CSpeakDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CSpeakDialog::OnBnClickedButton3)
END_MESSAGE_MAP()


// CSpeakDialog �����B�z�{��
//�w�ɾ����B�z���f���
void CSpeakDialog::DispatachTimer()
{
	if (!m_ActiveTimer)
		return;

	//�o�̶}�l�B�z�ڭ̪��޿豵�f
	if (!IsGameInitializeObject() || !QueryCurrentRoleType())
		return;

	//�}�l�i�J�۸ܪ��A
	if (IsDlgButtonChecked(IDC_CHECK1))
		AutoSpeak();
}

//��l�Ƶ��f
void CSpeakDialog::InitializeWindow()
{
	::SetTimer(m_hWnd, TIMERID_SPEAK, 300, NULL);
}

void CSpeakDialog::OnBnClickedCheck1()
{
	//�����ڭ̩w�ɾ��N�X
	m_ActiveTimer = (IsDlgButtonChecked(IDC_CHECK1)) ? true : false;
}

//�۰ʳ۸�
void CSpeakDialog::AutoSpeak()
{
	static DWORD dwTick = GetTickCount();
	static DWORD dwIndex = 0;
	int nCount = 0;

	//UpdateData(true);
	m_nActiveSec = GetDlgItemInt(IDC_EDIT1);

	//�����e�ɶ��M�۸ܶ��j
	DWORD dwcurTick = GetTickCount();

	//�p�G���b�o�Ӯɶ��I�A���򪽱���^�A�_�h�O�s�ܶq��p�Ƥ�
	if (dwcurTick - dwTick > m_nActiveSec && (nCount = m_SpeakList.GetCount(), nCount > 0))
	{
		//�o�̶}�l�ܧ�p��	
		dwIndex %= nCount;
		//����۸ܪ����e
		char szTextData[MAX_COMBOX_LEN * 2] = { 0 };

		m_SpeakList.GetText(dwIndex, szTextData);
		if (strlen(szTextData) <= 0)
			return;

		//�o�̽եΧڭ̪��C���۸�call
		Game_CallSpeak(szTextData);

		//��s�p�ƭ�
		dwIndex++;
		dwTick = dwcurTick;
	}
}

void CSpeakDialog::OnBnClickedButton1()
{
	char szTextSpeakData[MAX_COMBOX_LEN * 2] = { 0 };
	m_SpeakEdit.GetWindowText(szTextSpeakData, MAX_COMBOX_LEN * 2 - 1);
	//�p�G�Τ�~�ާ@�A�]�N�O�S����J�۸ܼƾڡA������^
	if (strlen(szTextSpeakData) <= 0)
		return;

	//�_�h�����K�[��ڭ̪��C���
	m_SpeakList.AddString(szTextSpeakData);
	m_SpeakEdit.SetWindowText(_T(""));
}


void CSpeakDialog::OnBnClickedButton3()
{
	int nSelect = m_SpeakList.GetCurSel();
	//�Τ�S���襤��󪺦�
	if (nSelect == -1)
		return;

	//�R���襤����
	m_SpeakList.DeleteString(nSelect);
}

//Ū���t�m�����f���>>�o�ӱ��f�u�|�եΤ@��
void CSpeakDialog::ReadUserConfig()
{
	//�}�l�g�J�ڭ̪��ƾڤ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	//INI��|�� ��e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	//���U�ӧڭ�Ū���t�m��󪺼ƾ�
	char* pcheckarray[2] = { "0", "1" };
	int nCheck = GetPrivateProfileInt(_T("page6"), _T("check0"), 0, szIniPath);
	((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(nCheck);

	//����ڭ̪��ɶ����j
	char szTextTime[MAX_COMBOX_LEN - 1] = { 0 };
	if (GetPrivateProfileString(_T("page6"), _T("edit0"), "", szTextTime, MAX_COMBOX_LEN - 1, szIniPath))
		SetDlgItemText(IDC_EDIT1, szTextTime);
	else
		SetDlgItemText(IDC_EDIT1, "2000");

	//����ڭ̷s���C��
	int nIndex = 0;
	do
	{
		char szKeyName[30] = { 0 };
		char szItemName[MAX_COMBOX_LEN] = { 0 };
		wsprintfA(szKeyName, _T("item%d"), nIndex++);
		if (GetPrivateProfileString(_T("page6"), szKeyName, "", szItemName, MAX_COMBOX_LEN - 1, szIniPath))
			m_SpeakList.AddString(szItemName);
		else
			break;
	} while (TRUE);

	m_ActiveTimer = (IsDlgButtonChecked(IDC_CHECK1)) ? true : false;
}

//�g�J�t�m�����f���>>�o�ӱ��f�C�����û��U�{�ǳ��|�Q�ե�
void CSpeakDialog::WriteUserConfig()
{
	//�}�l�g�J�ڭ̪��ƾڤ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	//INI��|�� ��e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	//���U�ӧڭ�Ū���t�m��󪺼ƾ�
	char* pcheckarray[2] = { "0", "1" };

	if (IsDlgButtonChecked(IDC_CHECK1))
		WritePrivateProfileString(_T("page6"), _T("check0"), pcheckarray[1], szIniPath);
	else
		WritePrivateProfileString(_T("page6"), _T("check0"), pcheckarray[0], szIniPath);

	char szTextTime[MAX_COMBOX_LEN - 1] = { 0 };
	if (GetDlgItemText(IDC_EDIT1, szTextTime, MAX_COMBOX_LEN - 1))
		WritePrivateProfileString(_T("page6"), _T("edit0"), szTextTime, szIniPath);
	else
		WritePrivateProfileString(_T("page6"), _T("edit0"), _T("2000"), szIniPath);

	//Ū���ڭ̪��C���
	int nListCount = m_SpeakList.GetCount();

	for (int i = 0; i < nListCount; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("item%d"), i);

		char szTextName[MAX_COMBOX_LEN] = { 0 };
		m_SpeakList.GetText(i, szTextName);
		WritePrivateProfileString(_T("page6"), szKeyName, szTextName, szIniPath);
	}
}
