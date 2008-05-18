/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: IniFile.cpp 118 2008-05-11 11:26:18Z Shiono $
 * @brief	Ini File ��ǂނ��߂̃N���X
 *			�Ƃ肠�����ǂ߂�΂�����I�Ȃ���
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#include "StdAfx.h"
#include "IniFile.h"

CIniFile::CIniFile(void)
: m_pKeyTable(NULL)
{
	m_szFileName[0] = _T('\0');
}

CIniFile::~CIniFile(void)
{
}

BOOL CIniFile::IsValidChar(TCHAR c)
{
	return _istalnum(c) || c == _T('_');
}

BOOL CIniFile::IsSeparatorChar(TCHAR c)
{
	return c == _T(' ') || c == _T('\t') || c == _T('=');
}


// �� read ������s�̐擪
void CIniFile::SkipToNextLine()
{
	TCHAR tcBuf;
	DWORD dwRead;
	ReadFile(m_hFile, &tcBuf, sizeof(tcBuf), &dwRead, NULL);
	while (dwRead == sizeof(tcBuf)) {
		if (tcBuf == _T('\n')) {
			break;
		}
		ReadFile(m_hFile, &tcBuf, sizeof(tcBuf), &dwRead, NULL);
	}
}


void CIniFile::GetParam(TCHAR tcFirstChar)
{
	// �܂��́A�L�[�����
	TCHAR szKey[MAX_KEY_LENGTH];
	TCHAR *p = szKey;
	*p++ = tcFirstChar;
	
	DWORD dwRead;
	TCHAR tcBuf;
	ReadFile(m_hFile, &tcBuf, sizeof(tcBuf), &dwRead, NULL);
	while (dwRead == sizeof(tcBuf)) {
		if (IsValidChar(tcBuf)) {
			if (p - szKey < ARRAY_SIZE_OF(szKey) - 1)	// ���ӂ�Ȃ��悤�Ƀ`�F�b�N
				*p++ = tcBuf;
		}
		else {
			break;
		}

		ReadFile(m_hFile, &tcBuf, sizeof(tcBuf), &dwRead, NULL);
	}
	*p = _T('\0');

	// ���́A�X�y�[�X�ƃ^�u��'='���΂�
	ReadFile(m_hFile, &tcBuf, sizeof(tcBuf), &dwRead, NULL);
	while (dwRead == sizeof(tcBuf)) {
		if (IsSeparatorChar(tcBuf))
			ReadFile(m_hFile, &tcBuf, sizeof(tcBuf), &dwRead, NULL);
		else
			break;
	}

	// ���́A�l�̕���������
	TCHAR szValue[MAX_KEY_LENGTH];
	p = szValue;
	*p++ = tcBuf;	// �P�����ڂ͊��ɓǂ�ł�

	ReadFile(m_hFile, &tcBuf, sizeof(tcBuf), &dwRead, NULL);
	while (dwRead == sizeof(tcBuf)) {
		if (_istalnum(tcBuf)) {
			*p++ = tcBuf;
		}
		else {
			// �s���܂Ŕ�΂�
			SkipToNextLine();
			break;
		}

		ReadFile(m_hFile, &tcBuf, sizeof(tcBuf), &dwRead, NULL);
	}
	*p = _T('\0');

	const INI_KEY *pTable = m_pKeyTable;
	BOOL bFound = FALSE;
	while (pTable->szKey && *(pTable->szKey)) {

		if (_tcscmp(szKey, pTable->szKey) == 0) {
			 *(pTable->pnVariable) = (int) _tcstol(szValue, NULL, pTable->nBase);
			 bFound = TRUE;
		}
		++pTable;
	}
}


BOOL CIniFile::Read()
{
	if (m_szFileName[0] == _T('\0'))
		return FALSE;

	if (!m_pKeyTable)
		return FALSE;

	m_hFile = CreateFile(m_szFileName,
						GENERIC_READ, 0, NULL,
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	TCHAR tcBuf;
	DWORD dwRead;
	ReadFile(m_hFile, &tcBuf, sizeof(tcBuf), &dwRead, NULL);
	while (dwRead == sizeof(tcBuf)) {
		if (tcBuf == _T(';')) {
			SkipToNextLine();
		} else if (IsValidChar(tcBuf)) {
			GetParam(tcBuf);
		}

		ReadFile(m_hFile, &tcBuf, sizeof(tcBuf), &dwRead, NULL);
	}

	CloseHandle(m_hFile);

	return TRUE;
}
