#pragma once

struct MD5_CTX
{
	ULONG num[2];   /* number of _bits_ handled mod 2^64 */
	ULONG buf[4];   /* scratch buffer */
	BYTE  input[64];  /* input buffer */
	BYTE  digest[16];  /* actual digest after MD5Final call */
};

void WINAPI MD5Init(MD5_CTX* lpCtx);
void WINAPI MD5Update(MD5_CTX* lpCtx, const void * lpBuf, unsigned int nLen);
void WINAPI MD5Final(MD5_CTX* lpCtx);

//����MD5 ���뻺����
void md5_buffer(_TCHAR* md5, const unsigned char* buffer, size_t len);
//����MD5 �����ļ�·��[ȫԱ����]
bool md5_file(_TCHAR* md5, const _TCHAR* filename);

//#pragma comment(lib, "md5.lib")

#include <cstdio>
#include <string>
using namespace std;

class CFILEMD5
{
public:
	CFILEMD5(){Init();}
	//��ʼ��
	void Init()
	{
		MD5Init(&context);
	}
	//����
	void Update(const void* lpBuff,unsigned int nLen)
	{
		MD5Update(&context,lpBuff,nLen);
	}
	//���
	string Final()
	{
		char result[33] = {0};
		Final(result);
		return string(result);
	}
	void Final(char* pmd5)
	{
		if (pmd5 == NULL)
			return;
		MD5Final(&context);
		char* write = pmd5;
		for (int i = 0; i < 16; ++i)
		{
			sprintf(write, "%02x", context.digest[i]);
			write += 2;
		}
		*write = '\0';
	}
protected:
	MD5_CTX context;
};