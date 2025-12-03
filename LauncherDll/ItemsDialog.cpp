// ItemsDialog.cpp : ��{���
//

#include "stdafx.h"
#include "HelpTools.h"
#include "stdafx.h"
#include "ItemsDialog.h"
#include "afxdialogex.h"

#include "GameMethods.h"
#include "Util.h"


// CItemsDialog ��ܮ�

IMPLEMENT_DYNAMIC(CItemsDialog, CPageDialog)

CItemsDialog::CItemsDialog(CWnd* pParent /*=NULL*/)
: CPageDialog(CItemsDialog::IDD, pParent)
{
	//���f��l�Ʈɤ��Ұʩw�ɾ�
	m_ActiveTimer = false;
}

CItemsDialog::~CItemsDialog()
{
	::KillTimer(m_hWnd, TIMERID_ITEMS);
}

void CItemsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_ItemsCombox);
	DDX_Control(pDX, IDC_LIST1, m_ActiveItemList);
}


BEGIN_MESSAGE_MAP(CItemsDialog, CDialog)
	ON_CBN_SETFOCUS(IDC_COMBO1, &CItemsDialog::OnCbnSetfocusCombo1)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK1, &CItemsDialog::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON1, &CItemsDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CItemsDialog::OnBnClickedButton2)
END_MESSAGE_MAP()


// CItemsDialog �����B�z�{��
void CItemsDialog::InitializeWindow()
{
	//��l�Ƨڭ̪����Ҿ������~�W�r
	char szTempString[MAX_COMBOX_LEN] = { 0 };
	GetPrivateProfileString(_T("AllSmeltItem"), _T("Item0"), _T("���Ѿ�"), szTempString, MAX_PATH - 1, _T("./LinHelperZ.ini"));
	m_strSmeltItemName = szTempString;

	//��ܧڭ̪��R������
	//��W�襤�ڭ̪��ܧΨ��b
	CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);

	//�Ы��ݩ�ڭ̪��w�ɾ���H
	::SetTimer(m_hWnd, TIMERID_ITEMS, 1000, NULL);
}

//�w�ɾ����B�z���f���
void CItemsDialog::DispatachTimer()
{
	//�p�G�S���ҥΩw�ɾ��ڭ̤]�n��^
	if (!m_ActiveTimer)
		return;

	//�o�̶}�l�B�z�ڭ̪��޿豵�f
	if (!IsGameInitializeObject() || !QueryCurrentRoleType())
		return;

#ifdef DEBUG
	TRACE("���~���f�w�ɾ��Q�ҥ�!time:%d\r\n", GetTickCount());
#endif

	//�o�̪��޿��²��A���O�R���N�O���Ҫ��~
	if (IsDlgButtonChecked(IDC_RADIO1))
		AutoSmeltItemList();
	else if (IsDlgButtonChecked(IDC_RADIO2))
		AutoDelItemList();
}

void CItemsDialog::OnBnClickedCheck1()
{
	//�����ڭ̩w�ɾ��N�X
	m_ActiveTimer = (IsDlgButtonChecked(IDC_CHECK1)) ? true : false;
}

void CItemsDialog::OnCbnSetfocusCombo1()
{
	//�����ڭ̲M�ŦC��
	m_ItemsCombox.ResetContent();

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
				m_ItemsCombox.AddString(szName);
			else
				m_ItemsCombox.AddString(pt->strName);
			//�R���o�ӹ�H
			delete pt;
		}
	}

}

//�s�W���~��R���C��
void CItemsDialog::OnBnClickedButton1()
{
	char szAddItemName[MAX_COMBOX_LEN] = { 0 };

	//�o�̬O�ڭ�combox��ܪ��U�Ф���
	int nSelect = m_ItemsCombox.GetCurSel();
	//�p�G�S���襤���ءA�h���ާ@
	if (nSelect == -1)
		return;

	//�ڭ̲K�[�@�w�n�O�ҲK�[���ƾڬO���Ī�
	if (m_ItemsCombox.GetLBText(nSelect, szAddItemName) && 0 >= strlen(szAddItemName))
		return;

	//���w�g�s�b��C��������~�A�ڭ̫h���K�[
	if (LB_ERR == m_ActiveItemList.FindString(-1, szAddItemName))
	{
		//�ڭ̻ݭn�N�o�ӲK�[��C���
		m_ActiveItemList.AddString(szAddItemName);
	}

	//�̫�N�襤���k�L
	m_ItemsCombox.SetCurSel(-1);
}

//�R���C����襤���U��
void CItemsDialog::OnBnClickedButton2()
{
	//�o�̪��ܧڭ̻ݭn�P�w�O�_�襤�F�R����
	int nSelect = m_ActiveItemList.GetCurSel();
	if (nSelect != -1)
		m_ActiveItemList.DeleteString(nSelect);
}

//�۰ʳB�z�ڭ̧R�������~
void CItemsDialog::AutoDelItemList()
{
	TRY
	{
		int nListCount = m_ActiveItemList.GetCount();
		for (int i = 0; i < nListCount; i++)
		{
			char szTextName[MAX_COMBOX_LEN] = { 0 };
			PITEMPOINT pt = NULL;
			//�d��ڭ̿襤�����奻�P�ɧP�w�o�Ӫ��~�O�_�s�b
			if (m_ActiveItemList.GetText(i, szTextName) && (pt = GetItemByName(szTextName), pt != NULL))
			{
				//�p�G�s�b�h�}�l�R��
				Game_DelItem(pt);
				delete pt;
			}
		}

	}
		CATCH(CMemoryException, e)
	{
		delete e;
	}
	END_CATCH

}

//�۰ʳB�z�ڭ̺��Ҫ����~
void CItemsDialog::AutoSmeltItemList()
{
	TRY
	{
		int nListCount = m_ActiveItemList.GetCount();
		for (int i = 0; i < nListCount; i++)
		{
			char szTextName[MAX_COMBOX_LEN] = { 0 };
			PITEMPOINT pt = NULL;
			//�d��ڭ̿襤�����奻�P�ɧP�w�o�Ӫ��~�O�_�s�b
			if (m_ActiveItemList.GetText(i, szTextName) && (pt = GetItemByName((char*)m_strSmeltItemName.c_str()), pt != NULL))
			{
#ifdef _SMELTITEM
				//�o���N�X�M�U�������G�O�@�˪�
				char szTextCmd[MAX_PATH] = { 0 };
				wsprintfA(szTextCmd, _T("%s/II=%s"), m_strSmeltItemName.c_str(), szTextName);
				UseItem(szTextCmd);
#endif
				//�p�G�s�b�h�}�l����
				UseItemBySmeltRoleItem(pt, szTextName);
				//				Game_SmeltItem(pt);
				delete pt;
			}
		}

	}
		CATCH(CMemoryException, e)
	{
		delete e;
	}
	END_CATCH
}

//Ū���t�m�����f���>>�o�ӱ��f�u�|�եΤ@��
void CItemsDialog::ReadUserConfig()
{
	//�}�l�g�J�ڭ̪��ƾڤ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	//INI��|�� ��e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	//���U�ӧڭ�Ū���t�m��󪺼ƾ�
	char* pcheckarray[2] = { "0", "1" };

	int nCheck = GetPrivateProfileInt(_T("page5"), _T("check0"), 0, szIniPath);
	((CButton*)GetDlgItem(IDC_CHECK1))->SetCheck(nCheck);

	vector<UINT> vctCheck;
	vctCheck.push_back(IDC_RADIO1);
	vctCheck.push_back(IDC_RADIO2);
	int nSize = vctCheck.size();

	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("radio%d"), i);

		//�����e�t�m�ﶵ�A�M���ܥ�
		int nCheck = GetPrivateProfileInt(_T("page5"), szKeyName, 0, szIniPath);
		((CButton*)GetDlgItem(vctCheck[i]))->SetCheck(nCheck);
	}

	int nIndex = 0;
	do
	{
		char szKeyName[30] = { 0 };
		char szItemName[MAX_COMBOX_LEN] = { 0 };
		wsprintfA(szKeyName, _T("item%d"), nIndex++);
		if (GetPrivateProfileString(_T("page5"), szKeyName, "", szItemName, MAX_COMBOX_LEN - 1, szIniPath))
			m_ActiveItemList.AddString(szItemName);
		else
			break;
	} while (TRUE);

	m_ActiveTimer = (IsDlgButtonChecked(IDC_CHECK1)) ? true : false;
}

//�g�J�t�m�����f���>>�o�ӱ��f�C�����û��U�{�ǳ��|�Q�ե�
void CItemsDialog::WriteUserConfig()
{
	//�}�l�g�J�ڭ̪��ƾڤ��e
	char szIniPath[MAX_PATH] = { 0 };
	char* pstrRoleName = GetCurrentRoleName();

	//INI��|�� ��e�ؿ�/Me(����W�r).ini
	wsprintfA(szIniPath, _T("./Me%s.ini"), pstrRoleName);

	//���U�ӧڭ�Ū���t�m��󪺼ƾ�
	char* pcheckarray[2] = { "0", "1" };

	if (IsDlgButtonChecked(IDC_CHECK1))
		WritePrivateProfileString(_T("page5"), _T("check0"), pcheckarray[1], szIniPath);
	else
		WritePrivateProfileString(_T("page5"), _T("check0"), pcheckarray[0], szIniPath);

	vector<UINT> vctCheck;
	vctCheck.push_back(IDC_RADIO1);
	vctCheck.push_back(IDC_RADIO2);
	int nSize = vctCheck.size();

	for (int i = 0; i < nSize; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("radio%d"), i);

		if (IsDlgButtonChecked(vctCheck[i]))
			WritePrivateProfileString(_T("page5"), szKeyName, pcheckarray[1], szIniPath);
		else
			WritePrivateProfileString(_T("page5"), szKeyName, pcheckarray[0], szIniPath);
	}

	//Ū���ڭ̪��C���
	int nListCount = m_ActiveItemList.GetCount();

	for (int i = 0; i < nListCount; i++)
	{
		char szKeyName[30] = { 0 };
		wsprintfA(szKeyName, _T("item%d"), i);

		char szTextName[MAX_COMBOX_LEN] = { 0 };
		m_ActiveItemList.GetText(i, szTextName);
		WritePrivateProfileString(_T("page5"), szKeyName, szTextName, szIniPath);
	}
}
