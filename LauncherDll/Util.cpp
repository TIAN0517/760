#include "StdAfx.h"
#include "stdafx.h"
#include "Util.h"
#include <cstring>

//���R�r�Ŧ�->�h���ڭ̬A���X��
bool AnalysisString(char* strsource, char strdest[MAX_PATH], int nmake)
{
	bool bIsFind = false;

	char szTemp[MAX_PATH] = { 0 };
	char szNumber[32] = { 0 };
	int nTempLen = 0;
	//�Q�i���ഫ
	itoa(nmake, szNumber, 10);
	nTempLen = strlen(szNumber);

	TRY
	{
		memset(strdest, 0, MAX_PATH);
		char* pos = strrchr(strsource, '(');//�q�奻�᭱�˧�
		if (pos)
		{
			pos++;
			//�����A����m�᪺�ƾ�
			strcpy(szTemp, pos);
			CString strTemp = szTemp;
			strTemp.Remove(',');
			strTemp.Remove(')');
			memset(szTemp, 0, MAX_PATH);
			strcpy(szTemp, strTemp.GetBuffer(0));

			if (0 == memicmp(szTemp, szNumber, nTempLen))
			{
				pos--;
				nTempLen = pos - strsource;
				memcpy(strdest, strsource, nTempLen);
				bIsFind = true;
			}
		}

		if (!bIsFind)
		{
			nTempLen = strlen(strsource);
			memcpy(strdest, strsource, nTempLen);
			bIsFind = true;
		}

	}
		CATCH(CMemoryException, e)
	{
		TRACE("�r�Ŧ�ѪR����!\r\n");
	}
	END_CATCH

		return bIsFind;
}
