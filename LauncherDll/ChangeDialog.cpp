// ChangeDialog.cpp : ��{���?
//

#include "stdafx.h"
#include "HelpTools.h"
#include "ChangeDialog.h"
#include "afxdialogex.h"


// CChangeDialog ��ܮ�?

IMPLEMENT_DYNAMIC(CChangeDialog, CPageDialog)

CChangeDialog::CChangeDialog(CWnd* pParent /*=NULL*/)
: CPageDialog(CChangeDialog::IDD, pParent)
{
	//���f��l�Ʈɤ��Ұʩw�ɾ�
	m_ActiveTimer = false;
}

CChangeDialog::~CChangeDialog()
{
}

void CChangeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ChangeItemCombox);
	DDX_Control(pDX, IDC_COMBO2, m_ChangeRoleCombox);
	DDX_Control(pDX, IDC_COMBO9, m_ChangeOthItemCombox);
	DDX_Control(pDX, IDC_COMBO10, m_ColorpoisonCombox);
	DDX_Control(pDX, IDC_COMBO11, m_KoerpoisonCombox);
	DDX_Control(pDX, IDC_COMBO12, m_StoneCombox);
	DDX_Control(pDX, IDC_COMBO13, m_BadyItemCombox);
	DDX_Control(pDX, IDC_CHECK1, m_AutoChangeCheck);
}


BEGIN_MESSAGE_MAP(CChangeDialog, CDialog)
	ON_CBN_SETFOCUS(IDC_COMBO9, &CChangeDialog::OnCbnSetfocusCombo9)
	ON_CBN_SETFOCUS(IDC_COMBO13, &CChangeDialog::OnCbnSetfocusCombo13)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON5, &CChangeDialog::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_CHECK1, &CChangeDialog::OnBnClickedCheck1)
	ON_CBN_DROPDOWN(IDC_COMBO13, &CChangeDialog::OnDropdownCombo13)
	ON_CBN_DROPDOWN(IDC_COMBO9, &CChangeDialog::OnDropdownCombo9)
	ON_CBN_SELCHANGE(IDC_COMBO13, &CChangeDialog::OnCbnSelchangeCombo13)
END_MESSAGE_MAP()


// CChangeDialog �����B�z�{��

void CChangeDialog::InitializeWindow()
{
	char szKeyName[30] = { 0 };
	char szTempString[MAX_PATH] = { 0 };
	int nIndex = 0;
	//Ū���ڭ̪��t�m���?
	do
	{
		wsprintfA(szKeyName, _T("Item%d"), nIndex++);
		if (0 >= GetPrivateProfileString(_T("AllPolymorphs"), szKeyName, _T(""), szTempString, MAX_PATH - 1, _T("./LinHelperZ.ini")))
			break;

		//��r�Ŧ���ζ}��
		int nTotalLen = strlen(szTempString);
		//�d���m
		char* ppos = strstr(szTempString, "_lv");

		size_t pos = (ppos - szTempString);
		//pos--;

		CHAR szName[100] = { 0 };
		char szNames[100] = { 0 };
		strncpy(szName, szTempString, pos);

		pos++;
		strcpy(szNames, szTempString + pos);

		m_vctAllNameString.push_back(szName);
		m_vctAllMonsterString.push_back(szNames);
		memset(szKeyName, 0, 30);
		memset(szTempString, 0, MAX_PATH);

	} while (TRUE);

	nIndex = 0;
	do
	{
		wsprintfA(szKeyName, _T("PolyItem%d"), nIndex++);
		if (0 >= GetPrivateProfileString(_T("AllPolymorphs"), szKeyName, _T(""), szTempString, MAX_PATH - 1, _T("./LinHelperZ.ini")))
			break;

		//�o�̼W�[���O�ܨ����~
		m_vctAllChangeItemString.push_back(szTempString);
	} while (TRUE);

	nIndex = 0;
	do
	{
		wsprintfA(szKeyName, _T("Item%d"), nIndex++);
		if (0 >= GetPrivateProfileString(_T("AllAntidote"), szKeyName, _T(""), szTempString, MAX_PATH - 1, _T("./LinHelperZ.ini")))
			break;

		//�o�̼W�[���O�Ѭr���~
		m_vctAllPoisonItemString.push_back(szTempString);
	} while (TRUE);

	//�W�[���Y
	m_vctAllStoneItemString.push_back(_T("�@�Ŷ��]��"));
	m_vctAllStoneItemString.push_back(_T("�G�Ŷ��]��"));
	m_vctAllStoneItemString.push_back(_T("�T�Ŷ��]��"));
	m_vctAllStoneItemString.push_back(_T("�|�Ŷ��]��"));



	//�}�l��l�ƦC��
	int nSize = m_vctAllChangeItemString.size();
	//�K�[�ڭ̪��ܨ����~
	for (int i = 0; i < nSize; i++)
		m_ChangeItemCombox.AddString(m_vctAllChangeItemString[i].c_str());

	//�K�[�ڭ̪��ܨ�����
	nSize = m_vctAllNameString.size();
	for (int i = 0; i < nSize; i++)
		m_ChangeRoleCombox.AddString(m_vctAllNameString[i].c_str());

	//�K�[�Ѭr���C��
	nSize = m_vctAllPoisonItemString.size();
	for (int i = 0; i < nSize; i++)
	{
		m_ColorpoisonCombox.AddString(m_vctAllPoisonItemString[i].c_str());
		m_KoerpoisonCombox.AddString(m_vctAllPoisonItemString[i].c_str());
	}

	nSize = m_vctAllStoneItemString.size();
	for (int i = 0; i < nSize; i++)
		m_StoneCombox.AddString(m_vctAllStoneItemString[i].c_str());

	memset(szTempString, 0, MAX_PATH);
	GetPrivateProfileString(_T("AllLoadStone"), _T("Item0"), _T("�����]��"), szTempString, MAX_PATH - 1, _T("./LinHelperZ.ini"));
	m_strStoneSkillString = szTempString;

	//��W�襤�ڭ̪��ܧΨ��b
	CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);


	//�Ы��ݩ�ڭ̪��w�ɾ���H
	::SetTimer(m_hWnd, TIMERID_CHANGE, 1000, NULL);
}

//�w�ɾ����B�z���f���?
void CChangeDialog::DispatachTimer()
{
	//�p�G�S���ҥΩw�ɾ��ڭ̤]�n��^
	// 	if(!m_ActiveTimer)
	// 		return;

#ifdef DEBUG
	TRACE("�ܨ����f�w�ɾ��Q�ҥ�!time:%d\r\n", GetTickCount());
#endif

	//�p�G�C����l�Ʋ��`�]�n��^�_�h�e���b��
	if (!IsGameInitializeObject() || !QueryCurrentRoleType())
		return;

	//�o�̶}�l�B�z�ڭ̪��޿�N�X
	if (IsDlgButtonChecked(IDC_CHECK1))
		//�o�̶}�l�B�z�ڭ̪��۰��ܨ����N�X
		AutoChangeRole();

	if (IsDlgButtonChecked(IDC_CHECK5))
		//�o�̳B�z�Ѱ���r���N�X
		RemoveColorPoisoning();

	if (IsDlgButtonChecked(IDC_CHECK9))
		//�o�̳B�z�Ѱ��d�r���N�X
		RemoveKoerPoisoning();


	if (IsDlgButtonChecked(IDC_CHECK12))
		//�o�̳B�z�����]�۪��N�X
		LoadStoneItem();

	if (IsDlgButtonChecked(IDC_CHECK14))
		//�o�̳B�z�]�k�������N�X
		BabyChangeRole();
}

void CChangeDialog::OnCbnSetfocusCombo9()
{

}


void CChangeDialog::OnCbnSetfocusCombo13()
{

}

//�۰��ܨ��B�z�N�X
void CChangeDialog::AutoChangeRole()
{
	//	CheckRadioButton(IDC_RADIO1,IDC_RADIO2,)
	char szItemName[MAX_COMBOX_LEN] = { 0 };

	PITEMPOINT pt = NULL;

	//�o�̬O���`�O�@���f�A����եΥX�{���`�C���b��
	__try
	{
		char szChangeRole[MAX_COMBOX_LEN] = { 0 };
		char szChangeType[MAX_COMBOX_LEN] = { 0 };


		//�o�̧P�_�H����O�ȵ���	 ���⦺�F���ٷd�ӻL(nMP = GetGameRoleHPMP(GAME_ROLE_CURRENTMP),!(nMP & 0xF6) && !(nMP & 0xD00))
		if (!GetGameRoleHPMP(GAME_ROLE_CURRENTHP) || !GetGameRoleHPMP(GAME_ROLE_CURRENTMP))
			__leave;

		//�����ڭ̪��r�Ŧ����w���w�İ�
		int nIndex = m_ChangeRoleCombox.GetCurSel();
		if (nIndex == -1)
			__leave;

		wsprintfA(szChangeRole, m_vctAllMonsterString[nIndex].c_str());

		//�}�l���Τ奻
		char* pos = strstr(szChangeRole, "_");
		if (pos == NULL)
			__leave;

		//�o�̶}�l�B�z�ڭ̪��d�߽X
		strcpy(szChangeType, pos + 1);
		*pos = '\0';


		//�o�̥[�J����P�w�A�p�G���e�B���ܨ����A�A�h�}�l�ˬd�O�_�j��Ҧ�?
		if (GetRoleState(GAME_STATE_CHANGEROLE))
		{
			//�o�̧P�_�@�U�O�_�j����w
			if (!IsDlgButtonChecked(IDC_CHECK2))
				__leave;

			//�o�̶}�l�ˬd�O�_���ڭ̫��w���Ǫ�
			if (GetRoleChangeState() == _ttoi(szChangeType))
				__leave;

		}
		// 		else
		// 		{
		// 			__leave;
		// 		}


		if (IsDlgButtonChecked(IDC_RADIO1))
		{
			//�p�G�ڭ̥��ܧΨ��b�ܨ�����[�o������ܨ����U��]
			m_ChangeItemCombox.GetWindowText(szItemName, MAX_COMBOX_LEN - 1);
			if (strlen(szItemName) > 0 && (pt = GetItemByName(szItemName), pt != NULL))
			{

				Game_UseChangeItem(pt, szChangeRole);
			}
		}

		if (IsDlgButtonChecked(IDC_RADIO2))
		{
			//�p�G�ڭ̥Ψ�L���~�i���ܨ�����[�o������ܨ����U��]
			m_ChangeOthItemCombox.GetWindowText(szItemName, MAX_COMBOX_LEN - 1);
			if (strlen(szItemName) > 0 && (pt = GetItemByName(szItemName), pt != NULL))
			{
				int nIndex = m_ChangeRoleCombox.GetCurSel();
				Game_UseChangeItem(pt, szChangeRole);
			}
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		if (pt)
			delete pt;
	}

}


//�B�z��r���N�X
void CChangeDialog::RemoveColorPoisoning()
{
	char szItemName[MAX_COMBOX_LEN] = { 0 };
	//�o�̬O���`�O�@���f�A����եΥX�{���`�C���b��
	__try
	{
		//�o�̧P�_�H����O�ȵ���	 ���⦺�F���ٷd�ӻL(nMP = GetGameRoleHPMP(GAME_ROLE_CURRENTMP),!(nMP & 0xF6) && !(nMP & 0xD00))
		if (!GetGameRoleHPMP(GAME_ROLE_CURRENTHP) || !GetGameRoleHPMP(GAME_ROLE_CURRENTMP))
			__leave;


		//�P�_�O�_���r�F
		if (GetRoleState(GAME_STATE_COLORPOISON1) || GetRoleState(GAME_STATE_COLORPOISON2))
		{
			m_ColorpoisonCombox.GetWindowText(szItemName, MAX_COMBOX_LEN - 1);
			if (strlen(szItemName) > 0)
			{
				//�ѼƬO���Ī�
				UseItem(szItemName);
			}
		}
		else if (GetCurrentRoleOthState())	//�I�q���r
		{
			TRACE("�I�q���r���A��..\r\n");
			m_ColorpoisonCombox.GetWindowText(szItemName, MAX_COMBOX_LEN - 1);
			if (strlen(szItemName) > 0)
			{
				//�ѼƬO���Ī�
				UseItem(szItemName);
			}
		}

	}
	__finally
	{

	}
}

//�B�z�d�r���N�X
void CChangeDialog::RemoveKoerPoisoning()
{
	char szItemName[MAX_COMBOX_LEN] = { 0 };
	//�o�̬O���`�O�@���f�A����եΥX�{���`�C���b��
	__try
	{
		//�o�̧P�_�H����O�ȵ���	 ���⦺�F���ٷd�ӻL(nMP = GetGameRoleHPMP(GAME_ROLE_CURRENTMP),!(nMP & 0xF6) && !(nMP & 0xD00))
		if (!GetGameRoleHPMP(GAME_ROLE_CURRENTHP) || !GetGameRoleHPMP(GAME_ROLE_CURRENTMP))
			__leave;


		//�P�_�O�_���r�F
		if (GetRoleState(GAME_STATE_KOERPOISON1) || GetRoleState(GAME_STATE_KOERPOISON2))
		{
			m_KoerpoisonCombox.GetWindowText(szItemName, MAX_COMBOX_LEN - 1);
			if (strlen(szItemName) > 0)
			{
				//�ѼƬO���Ī�
				UseItem(szItemName);
			}
		}

	}
	__finally
	{

	}
}


//�����]�۪��N�X
void  CChangeDialog::LoadStoneItem()
{
	char szItemName[MAX_COMBOX_LEN] = { 0 };
	__try
	{
		//�o�̧P�_�H����O�ȵ���	 ���⦺�F���ٷd�ӻL(nMP = GetGameRoleHPMP(GAME_ROLE_CURRENTMP),!(nMP & 0xF6) && !(nMP & 0xD00))
		if (!GetGameRoleHPMP(GAME_ROLE_CURRENTHP) || !GetGameRoleHPMP(GAME_ROLE_CURRENTMP))
			__leave;

		TRACE("�i�J���ҼҦ����q...\r\n");
		//�O�_���A����4>>�·t����
		if (IsUseStoneItem())
		{
			m_StoneCombox.GetWindowText(szItemName, MAX_COMBOX_LEN - 1);
			if (strlen(szItemName) > 0)
			{
				//�ѼƬO���Ī�
				char szBuffer[MAX_PATH] = { 0 };
				wsprintfA(szBuffer, _T("%s/MIS=%s"), m_strStoneSkillString.c_str(), szItemName);
				UseSkill(szBuffer);
			}
		}
	}
	__finally
	{

	}
}

//�B�z�]�k�������N�X
void CChangeDialog::BabyChangeRole()
{
	char szItemName[MAX_COMBOX_LEN] = { 0 };
	//�o�̬O���`�O�@���f�A����եΥX�{���`�C���b��
	static DWORD dwTickCount = 0;

	if (dwTickCount > 1000)
		dwTickCount = 0;
	else
		dwTickCount += 100;


	__try
	{
		//�o�̧P�_�H����O�ȵ���	 ���⦺�F���ٷd�ӻL(nMP = GetGameRoleHPMP(GAME_ROLE_CURRENTMP),!(nMP & 0xF6) && !(nMP & 0xD00))
		if (!GetGameRoleHPMP(GAME_ROLE_CURRENTHP) || !GetGameRoleHPMP(GAME_ROLE_CURRENTMP))
			__leave;

		//�o�̥[�J����P�w�A�C���P�w�Ҧ���10��
		if (!(dwTickCount % 500) && !GetRoleState(GAME_STATE_NORMALROLE))
		{
			m_BadyItemCombox.GetWindowText(szItemName, MAX_COMBOX_LEN - 1);
			if (strlen(szItemName) > 0)
			{
				//�ѼƬO���Ī�
				UseItem(szItemName);
			}
		}
	}
	__finally
	{

	}
}

void CChangeDialog::OnBnClickedButton5()
{
	DispatachTimer();
}


void CChangeDialog::OnBnClickedCheck1()
{
	m_ActiveTimer = (IsDlgButtonChecked(IDC_CHECK1)) ? true : false;
}


//Ū���t�m�����f���?>�o�ӱ��f�u�|�եΤ@��
void CChangeDialog::ReadUserConfig()
{
	//�}�l�g�J�ڭ̪��ƾڤ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	//INI���|�� ���e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	//���U�ӧڭ�Ū���t�m��󪺼ƾ�?
	char* pcheckarray[2] = { "0", "1" };


	vector<UINT> vctCheck;
	vctCheck.push_back(IDC_CHECK1);
	vctCheck.push_back(IDC_CHECK2);
	vctCheck.push_back(IDC_CHECK5);
	vctCheck.push_back(IDC_CHECK9);
	vctCheck.push_back(IDC_CHECK12);
	vctCheck.push_back(IDC_CHECK14);

	//�h�ﳡ���w�q
	int nSize = vctCheck.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("check%d"), i);

		int nCheck = GetPrivateProfileInt(_T("page4"), szKeyName, 0, szIniPath);
		((CButton*)GetDlgItem(vctCheck[i]))->SetCheck(nCheck);
	}

	vctCheck.clear();
	vctCheck.push_back(IDC_RADIO1);
	vctCheck.push_back(IDC_RADIO2);
	nSize = vctCheck.size();
	//����radio
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("radio%d"), i);

		int nCheck = GetPrivateProfileInt(_T("page4"), szKeyName, 0, szIniPath);
		((CButton*)GetDlgItem(vctCheck[i]))->SetCheck(nCheck);
	}


	//���combox��ܦC���U��
	vector<UINT> vctCombox;
	vctCombox.push_back(IDC_COMBO1);
	vctCombox.push_back(IDC_COMBO2);
	vctCombox.push_back(IDC_COMBO9);
	vctCombox.push_back(IDC_COMBO10);
	vctCombox.push_back(IDC_COMBO11);
	vctCombox.push_back(IDC_COMBO12);
	vctCombox.push_back(IDC_COMBO13);

	char szTextName[MAX_COMBOX_LEN] = { 0 };
	nSize = vctCombox.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("combox%d"), i);

		if (GetPrivateProfileString(_T("page4"), szKeyName, "", szTextName, MAX_COMBOX_LEN -1, szIniPath))
		{
			//int nIndex = ((CComboBox*)GetDlgItem(vctCombox[i]))->AddString(szTextName);
			//�ץ��۰��ܨ����~

			if (i == 6){
				int nIndex = ((CComboBox*)GetDlgItem(vctCombox[i]))->AddString(szTextName);
			}
				int nIndex = ((CComboBox*)GetDlgItem(vctCombox[i]))->FindString(-1, szTextName);
			
			
			
			
			if (nIndex != LB_ERR)
				((CComboBox*)GetDlgItem(vctCombox[i]))->SetCurSel(nIndex);
		}

	}

	m_ActiveTimer = (IsDlgButtonChecked(IDC_CHECK1)) ? true : false;
}

//Ū���t�m�����f���?>�o�ӱ��f�u�|�եΤ@��
void CChangeDialog::WriteUserConfig()
{
	//�}�l�g�J�ڭ̪��ƾڤ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();
	char* pcheckarray[2] = { "0", "1" };

	//INI���|�� ���e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	//���U�ӧڭ̼g�J�t�m��󪺼ƾ�page4
	vector<UINT> vctCheck;
	vctCheck.push_back(IDC_CHECK1);
	vctCheck.push_back(IDC_CHECK2);
	vctCheck.push_back(IDC_CHECK5);
	vctCheck.push_back(IDC_CHECK9);
	vctCheck.push_back(IDC_CHECK12);
	vctCheck.push_back(IDC_CHECK14);

	int nSize = vctCheck.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("check%d"), i);

		if (IsDlgButtonChecked(vctCheck[i]))
			WritePrivateProfileString(_T("page4"), szKeyName, pcheckarray[1], szIniPath);
		else
			WritePrivateProfileString(_T("page4"), szKeyName, pcheckarray[0], szIniPath);
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
			WritePrivateProfileString(_T("page4"), szKeyName, pcheckarray[1], szIniPath);
		else
			WritePrivateProfileString(_T("page4"), szKeyName, pcheckarray[0], szIniPath);
	}



	//���combox��ܦC���U��
	vector<UINT> vctCombox;
	vctCombox.push_back(IDC_COMBO1);
	vctCombox.push_back(IDC_COMBO2);
	vctCombox.push_back(IDC_COMBO9);
	vctCombox.push_back(IDC_COMBO10);
	vctCombox.push_back(IDC_COMBO11);
	vctCombox.push_back(IDC_COMBO12);
	vctCombox.push_back(IDC_COMBO13);

	char szTextName[MAX_COMBOX_LEN] = { 0 };
	nSize = vctCombox.size();
	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("combox%d"), i);

		if (GetDlgItemText(vctCombox[i], szTextName, MAX_COMBOX_LEN - 1))
			WritePrivateProfileString(_T("page4"), szKeyName, szTextName, szIniPath);
	}

}

void CChangeDialog::OnDropdownCombo13()
{
	//�����ڭ̲M�ŦC��
	m_BadyItemCombox.ResetContent();

	//������e���~�ƶq
	int nItemcount = GetItemsCount();
	for (int i = 0; i < nItemcount; i++)
	{
		//�o�̧ڭ̱o��F���~���W�r�F
		PITEMPOINT pt = GetItemByIndex(i);
		if (pt)
		{
			char szName[MAX_PATH] = { 0 };
			if (AnalysisString(pt->strName, szName, pt->ulCount))
				m_BadyItemCombox.AddString(szName);
			else
				m_BadyItemCombox.AddString(pt->strName);
			//�R���o�ӹ�H
			delete pt;
		}
	}
}


void CChangeDialog::OnDropdownCombo9()
{
	//�����ڭ̲M�ŦC��
	m_ChangeOthItemCombox.ResetContent();

	//������e���~�ƶq
	int nItemcount = GetItemsCount();
	for (int i = 0; i < nItemcount; i++)
	{
		//�o�̧ڭ̱o��F���~���W�r�F
		PITEMPOINT pt = GetItemByIndex(i);
		if (pt)
		{
			char szName[MAX_PATH] = { 0 };
			if (AnalysisString(pt->strName, szName, pt->ulCount))
				m_ChangeOthItemCombox.AddString(szName);
			else
				m_ChangeOthItemCombox.AddString(pt->strName);
			//�R���o�ӹ�H
			delete pt;
		}
	}
}


void CChangeDialog::OnCbnSelchangeCombo13()
{
	// TODO:  �b���[�J����i���B�z�`���{���X
}
