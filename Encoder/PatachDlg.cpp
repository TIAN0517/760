// PatachDlg.cpp : ��{���
//

#include "stdafx.h"
#include "Encoder.h"
#include "PatachDlg.h"
#include "afxdialogex.h"

#include "md5.h"
#include "../zlib-1.1.3/zlib.h"

using namespace std;
// CPatachDlg ��ܮ�

IMPLEMENT_DYNAMIC(CPatachDlg, CPropertyPage)

	CPatachDlg::CPatachDlg(CWnd* pParent /*=NULL*/)
	: CPropertyPage(CPatachDlg::IDD)
{

}

CPatachDlg::~CPatachDlg()
{
}

void CPatachDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_LIST2, m_list2);
}


BEGIN_MESSAGE_MAP(CPatachDlg, CPropertyPage)
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BUTTON1, &CPatachDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON5, &CPatachDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON2, &CPatachDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON6, &CPatachDlg::OnBnClickedButton6)
END_MESSAGE_MAP()


// CPatachDlg �����B�z�{��


BOOL CPatachDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//��l�ƦC��
	m_list.SetExtendedStyle(m_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list.InsertColumn(1,_T("�ʱ����Ҷ����|"),LVCFMT_LEFT,500);
	//=====m_list2
	m_list2.SetExtendedStyle(m_list2.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_list2.InsertColumn(1,_T("TITLE�W��"),LVCFMT_LEFT,200);
	return TRUE;  // return TRUE unless you set the focus to a control
	// ���`: OCX �ݩʭ�����^ FALSE
}


void CPatachDlg::OnDropFiles(HDROP hDropInfo)
{
	TCHAR szPath[MAX_PATH] = {0};
	UINT nCount = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	for(UINT idx = 0; idx < nCount; ++idx) {
		DragQueryFile(hDropInfo, idx, szPath, MAX_PATH);
		OnAddFiles(szPath);
	}
	DragFinish(hDropInfo);

	CPropertyPage::OnDropFiles(hDropInfo);
}


//�T�|�M�����
bool EnumChildFiles(TCHAR* szRoot,list<wstring>& strPath)
{
	CString strRootPath(szRoot);
	if (strRootPath[strRootPath.GetLength()-1] != '\\')
		strRootPath = strRootPath + _T("\\");

	CFileFind mFinder;
	BOOL bFind = mFinder.FindFile(strRootPath + _T("*.*"), 0); //�p�G���w����������ܡA�u��b���ؿ����M��
	while (bFind){
		bFind = mFinder.FindNextFile();
		if (mFinder.IsDots()){
			continue;
		}else{
			//				string strTemp = .GetBuffer(0);
			CString strTemp = mFinder.GetFilePath();
			if (mFinder.IsDirectory()){
				//�ؼЬO�ؿ�,�~��M��
				EnumChildFiles(strTemp.GetBuffer(0),strPath);
			}else{
				//�ؼЬO���
				strPath.push_back(strTemp.GetBuffer(0));
			}
		}
	}
	mFinder.Close();

	return TRUE;
}

//�K�[�j�q���
VOID CPatachDlg::OnAddFiles(TCHAR* szNewFilePath)
{
	//���P�_�O�_�����
	DWORD dwRetn = GetFileAttributes(szNewFilePath);

	if((dwRetn != INVALID_FILE_ATTRIBUTES) && (dwRetn & FILE_ATTRIBUTE_DIRECTORY))
	{
		//��󧨪��B�z��k
		list<wstring> newFiles;

		if(EnumChildFiles(szNewFilePath,newFiles))
		{
			//�}�l�B�z�a
			for (list<wstring>::iterator ir =newFiles.begin(); ir!=newFiles.end();ir++) 
			{
				CString strAddFile = (*ir).c_str();
				m_list.InsertItem(0,strAddFile);
			}

			newFiles.clear();
		}

	}
	else
	{
		//�p�G���O�A�����K�[
		m_list.InsertItem(0,szNewFilePath);
	}
}
char gchrxor[]="xor";
int intEncoder[1000]={-1};
int intDecoder[1000]={-1};
char chrDecoder[1000];
int XorEncoder(char *chrS)
{

    int i,j;
    int intlenxor=0;
    int intlenS=0;
    intlenxor=strlen(gchrxor);
    intlenS=strlen(chrS);
    char chrinput=0;
    char chrxor=0;
    j=0;
    for(i=0;i<intlenS;i++)
    {
        chrinput=*(chrS+i);
        if(j<intlenxor)
        {
            chrxor=*(gchrxor+j);
            j++;
        }
        else
        {
            j=0;
            chrxor=*(gchrxor+j);
        }
        intEncoder[i]=chrinput^chrxor;
    }
    return intlenS;
}
#define __CheckXor(a) (a ^ 0x00FFFFFF)
void CPatachDlg::OnBnClickedButton1()
{
	int nIndex = m_list.GetItemCount();
	int nIndex2 = m_list2.GetItemCount();

	if(nIndex <= 0){
		AfxMessageBox(_T("�ж�g�쥻���ϥ~��"));
		return;
	}
		

	//���P�@�ӿ��
	if(IDNO == ::MessageBox(m_hWnd, _T("�`�N�A���ާ@�|�л\�����e�ؿ��U���¯S���X���,�A�T�w�n�����?"),_T("���ɴ���"),MB_YESNO))
		//�Τᤣ�P�N
		return;

	//�p���{�ɽw�R�Ϫ��� [�ƶq * MD5�C�@�Ӫ�����]�w�d4�Ӧr�`�A����X
	PCHAR lpTempBuff = new CHAR[nIndex * 32 + 4];
	PCHAR lpArray = lpTempBuff;
	PCHAR NameArray = lpTempBuff;

	char szMD5[260] = {0};
	int nNumber = 0;

	CString tzFilePath_all = _T("");
	for(int j = 0; j < nIndex2 ; j++){
		CString tzFilePath = _T("");
		tzFilePath = m_list2.GetItemText(j,0);
		tzFilePath_all = tzFilePath_all + tzFilePath + _T(" ");
	}
    int i=0;
    int intlenS=0;
    int data=-1;
	USES_CONVERSION;
	char* psz = T2A((LPTSTR)(LPCTSTR)tzFilePath_all);
    intlenS=XorEncoder(psz);
    printf("in=%d\n",intlenS);
    FILE *pf='\0';
    pf=fopen("patach.txt","w");
    for(i=0;i<intlenS;i++)
    {
        fprintf(pf,"%d,",intEncoder[i]);
    }
	fprintf(pf,"###");
    fclose(pf);
    printf("\n");


	//===========�H�U�O�쥻md5
	for (int i = 0; i < nIndex; i++)
	{

		TCHAR tzFilePath[MAX_PATH] = {0};
		m_list.GetItemText(i,0,tzFilePath,MAX_PATH);

		TCHAR szMd5[64] = {0};
		//�ե�MD5������
		if(md5_file(szMd5,tzFilePath))
		{
			//�եΦ��\�A�O���ƶq
			nNumber++;
			//			TRACE("���e���:%S MD5:%S\r\n",tzFilePath,szMd5);
			wsprintfA(lpArray,"%S",szMd5);
			lpArray+= 32;
		}

	}

	//fclose(fp);
	//�ƾ�[�[�K]
	//��l����
	int nDecryLen = nNumber * 32;
	//�w�����
	DWORD en_size = nDecryLen * 1.001 + 12;
	//�w�⪺�w�R�Ϥj�p
	LPBYTE lpDecryBuff = new BYTE[en_size];
	//�g�J�ƾڤ��
	int ret = compress(lpDecryBuff, &en_size, (LPBYTE)lpTempBuff, nDecryLen);
	if(ret == Z_OK)
	{
		//�����d�w�F �g�J���a�ƾ�
		CHAR szPath[MAX_PATH] = {0};
		GetModuleFileNameA(NULL,szPath,MAX_PATH);
		PCHAR pos = strrchr(szPath,'\\');
		pos++;
		ZeroMemory(pos, MAX_PATH - (pos - szPath));
		strcat(pos,"patach.txt");

		FILE * fp = fopen( "patach.txt", "ab+");
		if(fp != NULL)
		{
			//�g�J��ڼƾ�
			fwrite(lpDecryBuff, 1, en_size, fp);
			nDecryLen = __CheckXor(nDecryLen);
			//�g�J����4�Ӧr�`
			fwrite((LPBYTE)&nDecryLen,4,1,fp);
			fclose(fp);
		}

	}
	delete[] lpDecryBuff;
	delete[] lpTempBuff;

	MessageBox(_T("�S���X���w�g��X��ؿ�,���a���:patach.txt �W�ǪA�Ⱦ�:�ۤv���w"), _T("�S���X�ͦ�����"), MB_ICONINFORMATION);
}


void CPatachDlg::OnBnClickedButton5()
{
	m_list.DeleteAllItems();
}


void CPatachDlg::OnBnClickedButton2()
{
	CString strSource;
	CEdit *editdlg;
	editdlg = (CEdit*)GetDlgItem(IDC_EDIT1);
	editdlg->GetWindowText(strSource); //Ū��
	m_list2.InsertItem(0,strSource);
}


void CPatachDlg::OnBnClickedButton6()
{
	int nItem = m_list2.GetSelectionMark();
	m_list2.DeleteItem(nItem);
}
