// HotKeyDialog.cpp : ��{���
//

#include "stdafx.h"
#include "HelpTools.h"
#include "stdafx.h"
#include "HotKeyDialog.h"
#include "afxdialogex.h"


// CHotKeyDialog ��ܮ�

IMPLEMENT_DYNAMIC(CHotKeyDialog, CDialog)

CHotKeyDialog::CHotKeyDialog(CWnd* pParent /*=NULL*/)
: CPageDialog(CHotKeyDialog::IDD, pParent)
{
	memset(m_bIsHotKey, 0, sizeof(m_bIsHotKey));
}

CHotKeyDialog::~CHotKeyDialog()
{
}

void CHotKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CHotKeyDialog, CDialog)
	ON_MESSAGE(WM_HOTKEY, OnHotKey)
	ON_BN_CLICKED(IDC_CHECK1, &CHotKeyDialog::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK3, &CHotKeyDialog::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK5, &CHotKeyDialog::OnBnClickedCheck5)
	ON_BN_CLICKED(IDC_CHECK7, &CHotKeyDialog::OnBnClickedCheck7)
	//ON_BN_CLICKED(IDC_CHECK9, &CHotKeyDialog::OnBnClickedCheck9)
	//ON_EN_CHANGE(IDC_EDIT14, &CHotKeyDialog::OnEnChangeEdit14)
	ON_EN_CHANGE(IDC_EDIT13, &CHotKeyDialog::OnEnChangeEdit13)
END_MESSAGE_MAP()


// CHotKeyDialog �����B�z�{��
//��l�Ƶ��f
void CHotKeyDialog::InitializeWindow()
{
	//�o�̽եΪ�l�Ʊ��f
	//??�n���o�̨S������ݭn��l�ƪ��a
}

//���o�����ڭֱ̧��䵡�f
bool CHotKeyDialog::DispatachSysHotKey(int nIndex)
{
	//�W�L�ڭ̼ƲդU�Фj�p�A������^�Y�i
	if (nIndex > sizeof(m_bIsHotKey) / sizeof(bool))
		return false;

	//�o�ӿ�ܥ��S���襤�A�]��^
	if (m_bIsHotKey[nIndex] == false)
		return false;

	char szUseText[MAX_COMBOX_LEN] = { 0 };
	switch (nIndex)
	{
	case 0:
		GetDlgItemText(IDC_EDIT1, szUseText, MAX_COMBOX_LEN - 1);
		break;
	case 1:
		GetDlgItemText(IDC_EDIT11, szUseText, MAX_COMBOX_LEN - 1);
		break;
	case 2:
		GetDlgItemText(IDC_EDIT12, szUseText, MAX_COMBOX_LEN - 1);
		break;
	case 3:
		GetDlgItemText(IDC_EDIT13, szUseText, MAX_COMBOX_LEN - 1);
		break;
	//case 4:
	//	GetDlgItemText(IDC_EDIT14, szUseText, MAX_COMBOX_LEN - 1);	//F5���ҥ�
	//	return false;
	//	break;
	default:
		return false;
		break;
	}

	if (strlen(szUseText) > 0)
	{
		TRACE("�ϥΧֱ���I�X:%s\r\n", szUseText);
		CallGameFunction(szUseText);
		return true;
	}

	return false;
}


LRESULT CHotKeyDialog::OnHotKey(WPARAM wParam, LPARAM lParam)
{
	//�o�̳B�z�t�ΩҦ���L�����t�μ���
	TRACE("�t�μ���Q�ե�,wparam = 0x%08X lparam = 0x%08X", wParam, lParam);
	return 1;
}

void CHotKeyDialog::OnBnClickedCheck1()
{
	// �t�μ���1
	m_bIsHotKey[0] = (IsDlgButtonChecked(IDC_CHECK1)) ? true : false;

}

void CHotKeyDialog::OnBnClickedCheck3()
{
	m_bIsHotKey[1] = (IsDlgButtonChecked(IDC_CHECK3)) ? true : false;
}

void CHotKeyDialog::OnBnClickedCheck5()
{
	m_bIsHotKey[2] = (IsDlgButtonChecked(IDC_CHECK5)) ? true : false;
}

void CHotKeyDialog::OnBnClickedCheck7()
{
	m_bIsHotKey[3] = (IsDlgButtonChecked(IDC_CHECK7)) ? true : false;
}

//void CHotKeyDialog::OnBnClickedCheck9()
//{
//	m_bIsHotKey[4] = (IsDlgButtonChecked(IDC_CHECK9)) ? true : false;
//}


BOOL CHotKeyDialog::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_F1)
	{
		TRACE("f1�Q���U!\r\n");
	}

	return CPageDialog::PreTranslateMessage(pMsg);
}

//Ū���t�m�����f���>>�o�ӱ��f�u�|�եΤ@��
void CHotKeyDialog::ReadUserConfig()
{
	//�}�l�g�J�ڭ̪��ƾڤ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	//INI��|�� ��e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	//���U�ӧڭ�Ū���t�m��󪺼ƾ�
	char* pcheckarray[2] = { "0", "1" };


	vector<UINT> vctCheck;
	vctCheck.push_back(IDC_CHECK1);
	vctCheck.push_back(IDC_CHECK3);
	vctCheck.push_back(IDC_CHECK5);
	vctCheck.push_back(IDC_CHECK7);
	//vctCheck.push_back(IDC_CHECK9);

	//�h�ﳡ���w�q
	int nSize = vctCheck.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("check%d"), i);

		int nCheck = GetPrivateProfileInt(_T("page7"), szKeyName, 0, szIniPath);
		((CButton*)GetDlgItem(vctCheck[i]))->SetCheck(nCheck);
		m_bIsHotKey[i] = nCheck;
	}

	vector<UINT> vctEdit;
	vctEdit.push_back(IDC_EDIT1);
	vctEdit.push_back(IDC_EDIT11);
	vctEdit.push_back(IDC_EDIT12);
	vctEdit.push_back(IDC_EDIT13);
	//vctEdit.push_back(IDC_EDIT14);
	nSize = vctEdit.size();

	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		char szTextName[MAX_COMBOX_LEN] = { 0 };

		wsprintfA(szKeyName, _T("edit%d"), i);
		if (GetPrivateProfileString(_T("page7"), szKeyName, "", szTextName, MAX_COMBOX_LEN - 1, szIniPath))
			SetDlgItemText(vctEdit[i], szTextName);
	}

}

//�g�J�t�m�����f���>>�o�ӱ��f�C�����û��U�{�ǳ��|�Q�ե�
void CHotKeyDialog::WriteUserConfig()
{
	//�}�l�g�J�ڭ̪��ƾڤ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	//INI��|�� ��e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	//���U�ӧڭ�Ū���t�m��󪺼ƾ�
	char* pcheckarray[2] = { "0", "1" };


	vector<UINT> vctCheck;
	vctCheck.push_back(IDC_CHECK1);
	vctCheck.push_back(IDC_CHECK3);
	vctCheck.push_back(IDC_CHECK5);
	vctCheck.push_back(IDC_CHECK7);
	//vctCheck.push_back(IDC_CHECK9);

	//�h�ﳡ���w�q
	int nSize = vctCheck.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("check%d"), i);

		if (IsDlgButtonChecked(vctCheck[i]))
			WritePrivateProfileString(_T("page7"), szKeyName, pcheckarray[1], szIniPath);
		else
			WritePrivateProfileString(_T("page7"), szKeyName, pcheckarray[0], szIniPath);
	}


	vector<UINT> vctEdit;
	vctEdit.push_back(IDC_EDIT1);
	vctEdit.push_back(IDC_EDIT11);
	vctEdit.push_back(IDC_EDIT12);
	vctEdit.push_back(IDC_EDIT13);
	//vctEdit.push_back(IDC_EDIT14);
	nSize = vctEdit.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		char szTextName[MAX_COMBOX_LEN] = { 0 };

		wsprintfA(szKeyName, _T("edit%d"), i);

		if (GetDlgItemText(vctEdit[i], szTextName, MAX_COMBOX_LEN - 1))
			WritePrivateProfileString(_T("page7"), szKeyName, szTextName, szIniPath);
	}

}

//void CHotKeyDialog::OnEnChangeEdit14()
//{
//	// TODO:  �p�G�o�O RICHEDIT ����A����N���|
//	// �ǰe���i���A���D�z�мg CPageDialog::OnInitDialog()
//	// �禡�M�I�s CRichEditCtrl().SetEventMask()
//	// ���㦳 ENM_CHANGE �X�� ORed �[�J�B�n�C
//
//	// TODO:  �b���[�J����i���B�z�`���{���X
//}


void CHotKeyDialog::OnEnChangeEdit13()
{
	// TODO:  �p�G�o�O RICHEDIT ����A����N���|
	// �ǰe���i���A���D�z�мg CPageDialog::OnInitDialog()
	// �禡�M�I�s CRichEditCtrl().SetEventMask()
	// ���㦳 ENM_CHANGE �X�� ORed �[�J�B�n�C

	// TODO:  �b���[�J����i���B�z�`���{���X
}
