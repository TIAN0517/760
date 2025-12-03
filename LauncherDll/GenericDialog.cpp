// GenericDialog.cpp : ??{???
//

#include "stdafx.h"
#include "HelpTools.h"
#include "GenericDialog.h"
#include "afxdialogex.h"

// CGenericDialog ????

IMPLEMENT_DYNAMIC(CGenericDialog, CPageDialog)

CGenericDialog::CGenericDialog(CWnd* pParent /*=NULL*/)
: CPageDialog(CGenericDialog::IDD, pParent)
{

}

CGenericDialog::~CGenericDialog()
{
	::KillTimer(m_hWnd, TIMERID_GENERIC);
}

void CGenericDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGenericDialog, CPageDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK1, &CGenericDialog::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, &CGenericDialog::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, &CGenericDialog::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK4, &CGenericDialog::OnBnClickedCheck4)
	ON_BN_CLICKED(IDC_CHECK5, &CGenericDialog::OnBnClickedCheck5)
	ON_BN_CLICKED(IDC_CHECK6, &CGenericDialog::OnBnClickedCheck6)
	ON_BN_CLICKED(IDC_CHECK7, &CGenericDialog::OnBnClickedCheck7)
	//ON_BN_CLICKED(IDC_BUTTON5, &CGenericDialog::OnBnClickedButton5)
	//ON_BN_CLICKED(IDC_BUTTON6, &CGenericDialog::OnBnClickedButton6)
	//ON_BN_CLICKED(IDC_BUTTON7, &CGenericDialog::OnBnClickedButton7)
	ON_EN_CHANGE(IDC_EDIT3, &CGenericDialog::OnEnChangeEdit3)
	ON_BN_CLICKED(IDC_CHECK10, &CGenericDialog::OnBnClickedCheck10)
	ON_BN_CLICKED(IDC_CHECK9, &CGenericDialog::OnBnClickedCheck9)
END_MESSAGE_MAP()


// CGenericDialog ?????B?z?{??


void CGenericDialog::OnBnClickedCheck1()
{
    if (IsDlgButtonChecked(IDC_CHECK1))
        SwitchAllDay();
    else
        SwitchAllDay(false);
}


void CGenericDialog::OnBnClickedCheck2()
{
    if (IsDlgButtonChecked(IDC_CHECK2))
        SwitchCPU();
    else
        SwitchCPU(false);
}


void CGenericDialog::OnBnClickedCheck3()
{
    if (IsDlgButtonChecked(IDC_CHECK3))
        SwitchWater();
    else
        SwitchWater(false);
}


void CGenericDialog::OnBnClickedCheck4()
{
    if (IsDlgButtonChecked(IDC_CHECK4))
        SwitchMonsterLevelColor();
    else
        SwitchMonsterLevelColor(false);
}


void CGenericDialog::OnBnClickedCheck5()
{
    if (IsDlgButtonChecked(IDC_CHECK5))
        SwitchGameTimer();
    else
        SwitchGameTimer(false);
}


void CGenericDialog::OnBnClickedCheck6()
{
    if (IsDlgButtonChecked(IDC_CHECK6))
        SwitchAttackLevelColor();
    else
        SwitchAttackLevelColor(false);
}


void CGenericDialog::OnBnClickedCheck7()
{
    if (IsDlgButtonChecked(IDC_CHECK7))
        SwitchRButtonLock();
    else
        SwitchRButtonLock(false);
}

// �۰ʸ��J�Ҧ��\���?
void CGenericDialog::LoadUserCheck()
{
	OnBnClickedCheck1();
	OnBnClickedCheck2();
	OnBnClickedCheck3();
	OnBnClickedCheck4();
	OnBnClickedCheck5();
	OnBnClickedCheck6();
	OnBnClickedCheck7();
}

// �w�ɾ��B�z��ܮ�?
void CGenericDialog::DispatachTimer()
{
	static bool bloadCheck = false;

	// �p�G�C�������٤��s�b�N���n�^�h����
	if (!IsGameInitializeObject() || !QueryCurrentRoleType())
		return;

	// �p�G�H�����F�A���N���n�ާڭ̪��Ʊ�
	if (!GetGameRoleHPMP(GAME_ROLE_CURRENTHP) || !GetGameRoleHPMP(GAME_ROLE_CURRENTMP))
		return;

	// �o�̧ڭ̲Ĥ@�����J�A�B�z�@�U�ڭ̪��ﶵ
	if (!bloadCheck)
	{
		bloadCheck = true;
		LoadUserCheck();
	}

	// ���U�ӧڭ̶}�l�B�z�ڭ̪��ƿ�ص{��[�H�U�o�X����ܮس��S���g�L�A�u�O���w�d��]
	if (IsDlgButtonChecked(IDC_CHECK9))
		AutoRepairRoleItem();

	if (IsDlgButtonChecked(IDC_CHECK10))
		AutoSaturateRole();

	if (IsDlgButtonChecked(IDC_CHECK8))
		AutoShowExpRole();

}

// ��l�ƹ�ܮص��f
void CGenericDialog::InitializeWindow()
{
	// ��l�Ʀ۰ʭײz�˳ƪ����~�M�����ת��~
	char szTempString[MAX_COMBOX_LEN] = { 0 };
	GetPrivateProfileString(_T("AllRepairItem"), _T("Item0"), _T("\xBF\xEF\xAB\xD7\xB2\x70"), szTempString, MAX_PATH - 1, _T("./LinHelperZ.ini"));
	m_RepairItemName = szTempString;

	memset(szTempString, 0, MAX_COMBOX_LEN);
	GetPrivateProfileString(_T("AllSaturateItem"), _T("Item0"), _T("\xA6\xD7"), szTempString, MAX_PATH - 1, _T("./LinHelperZ.ini"));
	m_SaturateItemName = szTempString;

	// �]�m���e�w�ɾ��B�z�Ʊ���ܮ�?�������@��
	::SetTimer(m_hWnd, TIMERID_GENERIC, 1000, NULL);
}

// �۰ʭײz�˳�
void CGenericDialog::AutoRepairRoleItem()
{

}

// �۰ʦY��
void CGenericDialog::AutoSaturateRole()
{

}

// �۰���ܸg��
void CGenericDialog::AutoShowExpRole()
{
    SwitchEXPLevelColor();
}
//
//void CGenericDialog::OnBnClickedButton5()
//{
//	//  	char szText[MAX_PATH] = {0};
//	// 	GetDlgItemText(IDC_EDIT3,szText,MAX_PATH);
//	// 	if(strlen(szText) <= 0)
//	// 		return;
//	// 
//	// 	CallGameFunction(szText);
//	// 	return;
//
//	//?o?????~?i??P?w
//	// 	int nRoleType = GetGameRoleHPMP(GAME_ROLE_CURRENTTYPE);
//	// 
//	// 	//????F?M?]?k?v?i??P?w
//	// 	if((nRoleType & 0xF6) || (nRoleType & 0xD00))
//	// 	{
//	// 		TRACE("???e??~???]?k?v?????F%d!\r\n",nRoleType);
//	// 	}
//	// 	else
//	// 	{
//	// 		TRACE("???e??~???D?]?k?v???D???F%d!\r\n",nRoleType);
//	// 	}
//	// 	return;
//	// 
//	// 	UseItem(szText);
//
//	/*
//
//	PITEMPOINT pt = GetItemByName(szText);
//	if(pt)
//	{
//
//	//	Game_UseItem(pt);
//	//	Game_UseChangeItem(pt,_T("lv1 skeleton"));
//
//	delete pt;
//	}*/
//
//	// 	int nState = _ttoi(szText);
//	// 	//0x27 == ???????????A
//	// 	if(!GetRoleState(nState))
//	// 		TRACE("???e???A?S???F!\r\n");
//	// 	else
//	// 		TRACE("???e???A?w?g?s?b!\r\n");
//	for (int idx = 0; idx < 0x300; idx++)
//	{
//		if (GetRoleState(idx))
//		{
//			TRACE("?????A???\!???Aid:%d!\r\n", idx);
//			break;
//		}
//	}
//
//
//}

//
//void CGenericDialog::OnBnClickedButton6()
//{
//#ifdef DEBUG
//	char szFileName[MAX_PATH] = { 0 };
//
//	TRY
//	{
//		//??????e????W?r
//		char* strname = GetCurrentRoleName();
//		wsprintfA(szFileName, _T("./%sSkill.ini"), strname);
//
//		//??????e????j??q
//		UINT32 nSkillCount = GetSkillCount();
//		for (int i = 0; i < nSkillCount; i++)
//		{
//			PSKILLPOINT pt = GetSkillByIndex(i);
//			if (pt)
//			{
//				TRACE("???W??:%s ???ID:%d ?????w:0x%08X \r\n", pt->strName, pt->ulSkillID, pt->ulIndex);
//				char szKeyName[32] = { 0 };
//				wsprintfA(szKeyName, _T("skill%d"), i);
//				WritePrivateProfileString(_T("RoleAllSkill"), szKeyName, pt->strName, szFileName);
//				delete pt;
//			}
//		}
//
//	}
//		CATCH(CMemoryException, e)
//	{
//		delete e;
//	}
//	END_CATCH
//
//		::MessageBoxA(NULL, _T("????????????"), _T("????"), MB_OK);
//#endif
//}


//void CGenericDialog::OnBnClickedButton7()
//{
//#ifdef DEBUG
//	char szFileName[MAX_PATH] = { 0 };
//
//	TRY
//	{
//		//??????e????W?r
//		char* strname = GetCurrentRoleName();
//		wsprintfA(szFileName, _T("./%sItem.ini"), strname);
//
//		//??????e????j??q
//		UINT32 nItemCount = GetItemsCount();
//		for (int i = 0; i < nItemCount; i++)
//		{
//			PITEMPOINT pt = GetItemByIndex(i);
//			if (pt)
//			{
//				TRACE("???~?W??:%s ???~ID:%d ???~???w:0x%08X ???~??q:%d\r\n", pt->strName, pt->ulItemID, pt->ulIndex, pt->ulCount);
//				char szKeyName[32] = { 0 };
//				wsprintfA(szKeyName, _T("Item%d"), i);
//				WritePrivateProfileString(_T("ItemAllItems"), szKeyName, pt->strName, szFileName);
//				delete pt;
//			}
//		}
//
//	}
//		CATCH(CMemoryException, e)
//	{
//		delete e;
//	}
//	END_CATCH
//
//		::MessageBoxA(NULL, _T("???????~??????"), _T("????"), MB_OK);
//
//#endif
//}

// Ū���t�ΰt�m��ܮ�?>�o�ӹ�ܮإu�|�ϥΤ@��
void CGenericDialog::ReadUserConfig()
{
	UINT ulCheckArray[10] = { 0 };

	// �}�l�g�J�ڭ̪���Ƥ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	// INI�ɸ��| ���e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	// ���U�ӧڭ̶}�l�g�J�t�ΰt�m
	int checkCount = (int)(sizeof(ulCheckArray) / sizeof(UINT));
	int index = 0;
	while (index < checkCount)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("check%d"), index);
		ulCheckArray[index] = GetPrivateProfileInt(_T("page1"), szKeyName, 0, szIniPath);
		// �N�ڭ̪��ﶵ���A�A�qŪ����ƨӨM�w�O�_�Ŀ�
		((CButton*)GetDlgItem(IDC_CHECK1 + index))->SetCheck(ulCheckArray[index]);
		index++;
	}
}

// �g�J�t�ΰt�m��ܮ�?>�o�ӹ�ܮبC�@�������������|�Q�ϥ�
void CGenericDialog::WriteUserConfig()
{
	UINT ulCheckArray[10] = { 0 };
	int checkCount = (int)(sizeof(ulCheckArray) / sizeof(UINT));
	int index = 0;
	while (index < checkCount)
	{
		ulCheckArray[index] = IsDlgButtonChecked(IDC_CHECK1 + index) ? true : false;
		index++;
	}

	// �}�l�g�J�ڭ̪���Ƥ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	// INI�ɸ��| ���e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	// ���U�ӧڭ̶}�l�g�J�t�ΰt�m
	index = 0;
	while (index < checkCount)
	{
		char szKeyName[30] = { 0 };
		char szValue[30] = { 0 };
		wsprintfA(szKeyName, _T("check%d"), index);
		_itoa_s(ulCheckArray[index], szValue, sizeof(szValue), 10);
		WritePrivateProfileString(_T("page1"), szKeyName, szValue, szIniPath);
		index++;
	}

}

void CGenericDialog::OnEnChangeEdit3()
{
	// TODO:  ?p?G?o?O RICHEDIT ???�w?A???�w?N???|
	// ??e???i???A???D?z??g CPageDialog::OnInitDialog()
	// ?��?M?I?s CRichEditCtrl().SetEventMask()
	// ???? ENM_CHANGE ?X?? ORed ?[?J?B?n?C

	// TODO:  ?b???[?J???�w?i???B?z?`???{???X
}


void CGenericDialog::OnBnClickedCheck10()
{
	// TODO:  ?b???[?J???�w?i???B?z?`???{???X
}


void CGenericDialog::OnBnClickedCheck9()
{
	// TODO:  ?b???[?J???�w?i???B?z?`???{???X
}
