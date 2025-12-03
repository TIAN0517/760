#include "StdAfx.h"
#include "PageDialog.h"

IMPLEMENT_DYNAMIC(CPageDialog, CDialog)

CPageDialog::CPageDialog(void)
{
	m_ActiveTimer = false;
}


CPageDialog::~CPageDialog(void)
{
}

BEGIN_MESSAGE_MAP(CPageDialog, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CPageDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	//��l�Ƥl���f
	InitializeWindow();

	//��l�Ƥ���N�i�H�}�l�եΧڭ̪�Ū�����f�F
	char* pRoleName = GetCurrentRoleName();
	if (pRoleName != NULL && strlen(pRoleName))
		ReadUserConfig();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CPageDialog::OnTimer(UINT_PTR nIDEvent)
{
	DispatachTimer();
	CDialog::OnTimer(nIDEvent);
}
