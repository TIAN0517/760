// StateDialog.cpp : ��{���
//

#include "stdafx.h"
#include "HelpTools.h"
#include "stdafx.h"
#include "StateDialog.h"
#include "afxdialogex.h"

#include "GameMethods.h"
#include "Util.h"
#include "Projectdef.h"

// CStateDialog ��ܮ�

IMPLEMENT_DYNAMIC(CStateDialog, CDialog)

CStateDialog::CStateDialog(CWnd* pParent /*=NULL*/)
: CPageDialog(CStateDialog::IDD, pParent)
{
}

CStateDialog::~CStateDialog()
{
	::KillTimer(m_hWnd, TIMERID_STATE);
}

void CStateDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ProtectItemCombox);
	DDX_Control(pDX, IDC_COMBO2, m_ProtectStatusCombox);
	DDX_Control(pDX, IDC_COMBO3, m_AllHPCombox1);
	DDX_Control(pDX, IDC_COMBO4, m_AllHPCombox2);
	DDX_Control(pDX, IDC_COMBO5, m_AllHPCombox3);
	DDX_Control(pDX, IDC_COMBO6, m_AllHPCombox4);
	DDX_Control(pDX, IDC_COMBO7, m_AllHPCombox5);
	DDX_Control(pDX, IDC_COMBO8, m_AllHPCombox6);
}


BEGIN_MESSAGE_MAP(CStateDialog, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CStateDialog::OnBnClickedButton1)
	ON_CBN_DROPDOWN(IDC_COMBO1, &CStateDialog::OnCbnDropdownCombo1)
	ON_CBN_DROPDOWN(IDC_COMBO2, &CStateDialog::OnCbnDropdownCombo2)
END_MESSAGE_MAP()

//�w�ɾ����B�z���f���
void CStateDialog::DispatachTimer()
{
	static CWnd* hRoleNameWnd = GetDlgItem(IDC_STC_ROLE);
	static CWnd* hRoleHPMPWnd = GetDlgItem(IDC_STC_HPMP);
	//�o�̶}�l�B�z�ڭ̪��޿豵�f
	if (!IsGameInitializeObject() || !QueryCurrentRoleType())
		return;
#ifdef DEBUG
	//TRACE("�ܤ�w�ɾ��E���ɶ�:%d ms\r\n",GetTickCount());
#endif

	//�o�̶}�l�B�z�ڭ̤H���W�r�H��
	m_nRoleHP = GetGameRoleHPMP(GAME_ROLE_CURRENTHP);
	m_nRoleMP = GetGameRoleHPMP(GAME_ROLE_CURRENTMP);

	char szText[100] = { 0 };
	wsprintfA(szText, _T("%04d/%04d"), m_nRoleHP, m_nRoleMP);
	hRoleHPMPWnd->SetWindowText(szText);

	char* strRoleName = GetCurrentRoleName();
	hRoleNameWnd->SetWindowText(strRoleName);

	//�ҰʫO�@
	if (IsDlgButtonChecked(IDC_CHECK1))
		EnableProtectRole();

	//�Ұʥͦs���o��
	if (IsDlgButtonChecked(IDC_CHECK8))
		EnableSurvivalRole();

	//�ҥθɦ�^�]1
	if (IsDlgButtonChecked(IDC_CHECK10))
		EnableHP1Protect();

	//�ҥθɦ�^�]2
	if (IsDlgButtonChecked(IDC_CHECK11))
		EnableHP2Protect();

	//�ҥθɦ�^�]3
	if (IsDlgButtonChecked(IDC_CHECK12))
		EnableHP3Protect();

	//�ҥθɦ�^�]4
	if (IsDlgButtonChecked(IDC_CHECK13))
		EnableHP4Protect();

	//�ҥθɦ�^�]5
	if (IsDlgButtonChecked(IDC_CHECK14))
		EnableHP5Protect();

	//�ҥά~�]�ҵ{
	if (IsDlgButtonChecked(IDC_CHECK15))
		EnableHPMPProtect();

}

//��l�Ƶ��f
void CStateDialog::InitializeWindow()
{
	char szKeyName[30] = { 0 };
	char szTempString[MAX_PATH] = { 0 };
	int nIndex = 0;

	//Ū���ڭ̪��t�m���
	do
	{
		wsprintfA(szKeyName, _T("Item%d"), nIndex++);
		if (0 >= GetPrivateProfileString(_T("AllHP"), szKeyName, _T(""), szTempString, MAX_PATH - 1, _T("./LinHelperZ.ini")))
			break;

		m_vctAllHPString.push_back(szTempString);
		memset(szKeyName, 0, 30);
		memset(szTempString, 0, MAX_PATH);

	} while (TRUE);

	nIndex = 0;
	do
	{
		wsprintfA(szKeyName, _T("HPMP%d"), nIndex++);
		if (0 >= GetPrivateProfileString(_T("AllHP"), szKeyName, _T(""), szTempString, MAX_PATH - 1, _T("./LinHelperZ.ini")))
			break;

		m_vctAllHPMPString.push_back(szTempString);
		memset(szKeyName, 0, 30);
		memset(szTempString, 0, MAX_PATH);
	} while (TRUE);

	//�o�̥[�J��Ҧ�������W
	int nSize = m_vctAllHPString.size();
	for (int i = 0; i < nSize; i++)
	{
		m_AllHPCombox1.AddString(m_vctAllHPString[i].c_str());
		m_AllHPCombox2.AddString(m_vctAllHPString[i].c_str());
		m_AllHPCombox3.AddString(m_vctAllHPString[i].c_str());
		m_AllHPCombox4.AddString(m_vctAllHPString[i].c_str());
		m_AllHPCombox5.AddString(m_vctAllHPString[i].c_str());
	}

	nSize = m_vctAllHPMPString.size();
	for (int i = 0; i < nSize; i++)
	{
		m_AllHPCombox6.AddString(m_vctAllHPMPString[i].c_str());
	}

	memset(szTempString, 0, MAX_PATH);
	GetPrivateProfileString(_T("AllMoveItem"), _T("Item0"), _T("�������ʨ��b"), szTempString, MAX_PATH - 1, _T("./LinHelperZ.ini"));
	m_strMoveItemString = szTempString;

	//��W�襤�ڭ̪��ܧΨ��b
	CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
	//��l�Ʃw�ɾ�
	::SetTimer(m_hWnd, TIMERID_STATE, 300, NULL);
}

//�o�ӧ@���ոի��s�n�F
void CStateDialog::OnBnClickedButton1()
{
	//	DispatachTimer();
	char szTextName[MAX_COMBOX_LEN] = { 0 };

	if (GetDlgItemText(IDC_COMBO2, szTextName, MAX_COMBOX_LEN - 1))
	{
		char szBuffer[MAX_PATH] = { 0 };
		wsprintfA(szBuffer, _T("%s/IBM=%s"), m_strMoveItemString.c_str(), szTextName);
		//�Τ��ܤF���w�����~
		CallGameFunction(szBuffer);
	}
}

void CStateDialog::OnCbnDropdownCombo1()
{
	//�����ڭ̲M�ŦC��
	m_ProtectItemCombox.ResetContent();

	//�����e���~�ƶq
	int nItemcount = GetItemsCount();
	for (int i = 0; i < nItemcount; i++)
	{
		//�o�̧ڭ̱o��F���~���W�r�F
		PITEMPOINT pt = GetItemByIndex(i);
		if (pt)
		{
			char szName[MAX_PATH] = { 0 };
			if (AnalysisString(pt->strName, szName, pt->ulCount))
				m_ProtectItemCombox.AddString(szName);
			else
				m_ProtectItemCombox.AddString(pt->strName);
			//�R���o�ӹ�H
			delete pt;
		}
	}
}


void CStateDialog::OnCbnDropdownCombo2()
{
	m_ProtectStatusCombox.ResetContent();

	//����ڭ̼ƶq
	int nStatusCount = GetStatusCount();

	for (int i = 0; i < nStatusCount; i++)
	{
		//�o�̧ڭ̱o�쪬�A���W�r
		PSTATUSPOINT pt = GetStatusIndex(i);
		if (pt)
		{
			m_ProtectStatusCombox.AddString(pt->strName);
			delete pt;
		}
	}
}


//�ҥΫO�@�եΨҵ{
void CStateDialog::EnableProtectRole()
{
	/*
	��{�޿�:
	1.�����ˬd�O�_�C��ڭ̳]�w����,�p�G���C��A�N������^
	2.�ˬd�Τ�O�_�ݭn�ϥΪ��~���A�p�G�O���ܡA�}�l�ϥΧڭ̪����~
	3.�ˬd�O�_�ݭn�����СA�O���ܡA�}�l��������
	*/
	UINT nCheckValue = GetDlgItemInt(IDC_EDIT1);
	char	szTextName[MAX_COMBOX_LEN] = { 0 };

	if (nCheckValue == 0)
		return;

	//�Τ��ܥΪ��~
	if (IsDlgButtonChecked(IDC_RADIO1))
	{
		//�p�G��e��HP�p���J��HP�A��������s��ت����~�A�}�l�ϥ�
		if (m_nRoleHP < nCheckValue &&
			GetDlgItemText(IDC_COMBO1, szTextName, MAX_COMBOX_LEN - 1))
		{
			//�Τ��ܤF���w�����~
			CallGameFunction(szTextName);
		}

	}
	else if (IsDlgButtonChecked(IDC_RADIO2))
	{
		//�Τ��ܨϥΧ��Э���
		if (m_nRoleHP < nCheckValue && GetDlgItemText(IDC_COMBO2, szTextName, MAX_COMBOX_LEN - 1))
		{
			char szBuffer[MAX_PATH] = { 0 };
			wsprintfA(szBuffer, _T("%s/IBM=%s"), m_strMoveItemString.c_str(), szTextName);
			//�Τ��ܤF���w�����~
			CallGameFunction(szBuffer);
		}
	}

}

//�ҥΥͦs���o�۽եΨҵ{
void CStateDialog::EnableSurvivalRole()
{
	/*
	��{�޿�:
	1.�����ˬd�O�_�C��ڭ̳]�w����,�p�G���C��A�N������^
	2.�}�l�եΥͦs���o��
	*/
	UINT nCheckValue = GetDlgItemInt(IDC_EDIT2);
	char	szTextName[MAX_COMBOX_LEN] = { 0 };

	if (nCheckValue == 0)
		return;

	//�W�[�@�B�P�w�A�p�G�����פp��100�A�h��Ĳ�o�ĪG
	if (GetRoleSaturate() < 100)
		return;


	//�p�G��e��HP�p���J��HP�A��������s��ت����~�A�}�l�ϥ�
	if (m_nRoleHP < nCheckValue)
	{
		//�Τ��ܤF�ϥΥͦs���o��
		TRACE("�P�w�ϥΥͦs���o��!\r\n");
		Game_CallRoleLife();
	}
}

//�ҥ�HP�Ĥ@�ӱ���P�_
void CStateDialog::EnableHP1Protect()
{
	/*
	��{�޿�:
	1.�����ˬd�O�_�C��ڭ̳]�w����,�p�G���C��A�N������^
	2.�}�l�ե�combox��ܪ��ƾ�
	*/
	UINT nCheckValue = GetDlgItemInt(IDC_EDIT3);
	char	szTextName[MAX_COMBOX_LEN] = { 0 };

	if (nCheckValue == 0)
		return;

	//�p�G��e��HP�p���J��HP�åB�U�Юؿ�ܤF�ڭ̪��ƾ�
	if (m_nRoleHP < nCheckValue && GetDlgItemText(IDC_COMBO3, szTextName, MAX_COMBOX_LEN - 1))
	{
		CallGameFunction(szTextName);
	}

}

//�ҥ�HP�Ĥ@�ӱ���P�_
void CStateDialog::EnableHP2Protect()
{
	/*
	��{�޿�:
	1.�����ˬd�O�_�C��ڭ̳]�w����,�p�G���C��A�N������^
	2.�}�l�ե�combox��ܪ��ƾ�
	*/
	UINT nCheckValue = GetDlgItemInt(IDC_EDIT4);
	char	szTextName[MAX_COMBOX_LEN] = { 0 };

	if (nCheckValue == 0)
		return;

	//�p�G��e��HP�p���J��HP�åB�U�Юؿ�ܤF�ڭ̪��ƾ�
	if (m_nRoleHP < nCheckValue && GetDlgItemText(IDC_COMBO4, szTextName, MAX_COMBOX_LEN - 1))
	{
		CallGameFunction(szTextName);
	}
}

//�ҥ�HP�Ĥ@�ӱ���P�_
void CStateDialog::EnableHP3Protect()
{
	/*
	��{�޿�:
	1.�����ˬd�O�_�C��ڭ̳]�w����,�p�G���C��A�N������^
	2.�}�l�ե�combox��ܪ��ƾ�
	*/
	UINT nCheckValue = GetDlgItemInt(IDC_EDIT5);
	char	szTextName[MAX_COMBOX_LEN] = { 0 };

	if (nCheckValue == 0)
		return;

	//�p�G��e��HP�p���J��HP�åB�U�Юؿ�ܤF�ڭ̪��ƾ�
	if (m_nRoleHP < nCheckValue && GetDlgItemText(IDC_COMBO5, szTextName, MAX_COMBOX_LEN - 1))
	{
		CallGameFunction(szTextName);
	}
}

//�ҥ�HP�Ĥ@�ӱ���P�_
void CStateDialog::EnableHP4Protect()
{
	/*
	��{�޿�:
	1.�����ˬd�O�_�C��ڭ̳]�w����,�p�G���C��A�N������^
	2.�}�l�ե�combox��ܪ��ƾ�
	*/
	UINT nCheckValue = GetDlgItemInt(IDC_EDIT6);
	char	szTextName[MAX_COMBOX_LEN] = { 0 };

	if (nCheckValue == 0)
		return;

	//�p�G��e��HP�p���J��HP�åB�U�Юؿ�ܤF�ڭ̪��ƾ�
	if (m_nRoleHP < nCheckValue && GetDlgItemText(IDC_COMBO6, szTextName, MAX_COMBOX_LEN - 1))
	{
		CallGameFunction(szTextName);
	}
}

//�ҥ�HP�Ĥ@�ӱ���P�_
void CStateDialog::EnableHP5Protect()
{
	/*
	��{�޿�:
	1.�����ˬd�O�_�C��ڭ̳]�w����,�p�G���C��A�N������^
	2.�}�l�ե�combox��ܪ��ƾ�
	*/
	UINT nCheckValue = GetDlgItemInt(IDC_EDIT7);
	char	szTextName[MAX_COMBOX_LEN] = { 0 };

	if (nCheckValue == 0)
		return;

	//�p�G��e��HP�p���J��HP�åB�U�Юؿ�ܤF�ڭ̪��ƾ�
	if (m_nRoleHP < nCheckValue && GetDlgItemText(IDC_COMBO7, szTextName, MAX_COMBOX_LEN - 1))
	{
		CallGameFunction(szTextName);
	}
}

//�ҥά~�]�եΨҵ{
void CStateDialog::EnableHPMPProtect()
{
	/*
	��{�޿�:
	1.�����ˬd�O�_�C��ڭ̳]�w����,�p�G���C��A�N������^
	2.�}�l�ե�combox��ܪ��ƾ�
	3.�}�l�˴���q�M��O�O�_���`
	*/
	UINT nCheckHPValue = GetDlgItemInt(IDC_EDIT8);
	UINT nCheckMPValue = GetDlgItemInt(IDC_EDIT9);

	char	szTextName[MAX_COMBOX_LEN] = { 0 };

	if (nCheckHPValue == 0 && nCheckMPValue == 0)
		return;

	//�p�G��e��HP�j���e�]�w���ȨåBMP�p��]�w���ȨåBcombox���Ȧ���
	if (m_nRoleHP > nCheckHPValue  && m_nRoleMP < nCheckMPValue &&
		GetDlgItemText(IDC_COMBO8, szTextName, MAX_COMBOX_LEN - 1))
	{
		//�o�˧ڭ̤~�եΧڭ̨ϥΪ����~
		CallGameFunction(szTextName);
	}
}

//Ū���t�m�����f���>>�o�ӱ��f�u�|�եΤ@��
void CStateDialog::ReadUserConfig()
{
	//�}�l�g�J�ڭ̪��ƾڤ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	//INI��|�� ��e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	//���U�ӧڭ�Ū���t�m��󪺼ƾ�
	//�}�l�g�J�ڭ̪��ƾڤ��e

	char* pcheckarray[2] = { "0", "1" };

	vector<UINT> vctCheck;
	vctCheck.push_back(IDC_CHECK1);
	vctCheck.push_back(IDC_CHECK8);
	vctCheck.push_back(IDC_CHECK10);
	vctCheck.push_back(IDC_CHECK11);
	vctCheck.push_back(IDC_CHECK12);
	vctCheck.push_back(IDC_CHECK13);
	vctCheck.push_back(IDC_CHECK14);
	vctCheck.push_back(IDC_CHECK15);

	int nSize = vctCheck.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("check%d"), i);

		int nCheck = GetPrivateProfileInt(_T("page2"), szKeyName, 0, szIniPath);
		((CButton*)GetDlgItem(vctCheck[i]))->SetCheck(nCheck);
	}

	vctCheck.clear();
	vctCheck.push_back(IDC_RADIO1);
	vctCheck.push_back(IDC_RADIO2);
	nSize = vctCheck.size();

	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("radio%d"), i);

		int nCheck = GetPrivateProfileInt(_T("page2"), szKeyName, 0, szIniPath);
		((CButton*)GetDlgItem(vctCheck[i]))->SetCheck(nCheck);
	}

	vector<UINT> vctEdit;
	vctEdit.push_back(IDC_EDIT1);
	vctEdit.push_back(IDC_EDIT2);
	vctEdit.push_back(IDC_EDIT3);
	vctEdit.push_back(IDC_EDIT4);
	vctEdit.push_back(IDC_EDIT5);
	vctEdit.push_back(IDC_EDIT6);
	vctEdit.push_back(IDC_EDIT7);
	vctEdit.push_back(IDC_EDIT8);
	vctEdit.push_back(IDC_EDIT9);
	nSize = vctEdit.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("edit%d"), i);
		char szEditText[MAX_COMBOX_LEN] = { 0 };
		GetPrivateProfileString(_T("page2"), szKeyName, "", szEditText, MAX_COMBOX_LEN - 1, szIniPath);
		SetDlgItemText(vctEdit[i], szEditText);
	}

	vector<UINT> vctCombox;
	vctCombox.push_back(IDC_COMBO1);
	vctCombox.push_back(IDC_COMBO2);
	vctCombox.push_back(IDC_COMBO3);
	vctCombox.push_back(IDC_COMBO4);
	vctCombox.push_back(IDC_COMBO5);
	vctCombox.push_back(IDC_COMBO6);
	vctCombox.push_back(IDC_COMBO7);
	vctCombox.push_back(IDC_COMBO8);
	nSize = vctCombox.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("combox%d"), i);
		char szComboxText[MAX_COMBOX_LEN] = { 0 };

		if (GetPrivateProfileString(_T("page2"), szKeyName, "", szComboxText, MAX_COMBOX_LEN - 1, szIniPath))
		{
			int nIndex = ((CComboBox*)GetDlgItem(vctCombox[i]))->FindString(-1, szComboxText);
			if (nIndex != LB_ERR)
				((CComboBox*)GetDlgItem(vctCombox[i]))->SetCurSel(nIndex);
			else
				((CComboBox*)GetDlgItem(vctCombox[i]))->SetWindowText(szComboxText);
		}
	}

}

//�g�J�t�m�����f���>>�o�ӱ��f�C�����û��U�{�ǳ��|�Q�ե�
void CStateDialog::WriteUserConfig()
{
	//�}�l�g�J�ڭ̪��ƾڤ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	//INI��|�� ��e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	//���U�ӧڭ�Ū���t�m��󪺼ƾ�
	//�}�l�g�J�ڭ̪��ƾڤ��e

	char* pcheckarray[2] = { "0", "1" };


	vector<UINT> vctCheck;
	vctCheck.push_back(IDC_CHECK1);
	vctCheck.push_back(IDC_CHECK8);
	vctCheck.push_back(IDC_CHECK10);
	vctCheck.push_back(IDC_CHECK11);
	vctCheck.push_back(IDC_CHECK12);
	vctCheck.push_back(IDC_CHECK13);
	vctCheck.push_back(IDC_CHECK14);
	vctCheck.push_back(IDC_CHECK15);

	int nSize = vctCheck.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("check%d"), i);

		if (IsDlgButtonChecked(vctCheck[i]))
			WritePrivateProfileString(_T("page2"), szKeyName, pcheckarray[1], szIniPath);
		else
			WritePrivateProfileString(_T("page2"), szKeyName, pcheckarray[0], szIniPath);
	}

	vctCheck.clear();
	vctCheck.push_back(IDC_RADIO1);
	vctCheck.push_back(IDC_RADIO2);
	nSize = vctCheck.size();

	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("radio%d"), i);

		if (IsDlgButtonChecked(vctCheck[i]))
			WritePrivateProfileString(_T("page2"), szKeyName, pcheckarray[1], szIniPath);
		else
			WritePrivateProfileString(_T("page2"), szKeyName, pcheckarray[0], szIniPath);
	}

	vector<UINT> vctEdit;
	vctEdit.push_back(IDC_EDIT1);
	vctEdit.push_back(IDC_EDIT2);
	vctEdit.push_back(IDC_EDIT3);
	vctEdit.push_back(IDC_EDIT4);
	vctEdit.push_back(IDC_EDIT5);
	vctEdit.push_back(IDC_EDIT6);
	vctEdit.push_back(IDC_EDIT7);
	vctEdit.push_back(IDC_EDIT8);
	vctEdit.push_back(IDC_EDIT9);
	nSize = vctEdit.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("edit%d"), i);
		char szEditText[MAX_COMBOX_LEN] = { 0 };
		if (GetDlgItemText(vctEdit[i], szEditText, MAX_COMBOX_LEN - 1))
			WritePrivateProfileString(_T("page2"), szKeyName, szEditText, szIniPath);
	}

	vector<UINT> vctCombox;
	vctCombox.push_back(IDC_COMBO1);
	vctCombox.push_back(IDC_COMBO2);
	vctCombox.push_back(IDC_COMBO3);
	vctCombox.push_back(IDC_COMBO4);
	vctCombox.push_back(IDC_COMBO5);
	vctCombox.push_back(IDC_COMBO6);
	vctCombox.push_back(IDC_COMBO7);
	vctCombox.push_back(IDC_COMBO8);
	nSize = vctCombox.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("combox%d"), i);
		char szComboxText[MAX_COMBOX_LEN] = { 0 };
		if (GetDlgItemText(vctCombox[i], szComboxText, MAX_COMBOX_LEN - 1))
			WritePrivateProfileString(_T("page2"), szKeyName, szComboxText, szIniPath);
	}

	m_ActiveTimer = (IsDlgButtonChecked(IDC_CHECK1)) ? true : false;
}
