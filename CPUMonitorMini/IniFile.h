/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: IniFile.h 118 2008-05-11 11:26:18Z Shiono $
 * @brief	Ini File ��ǂނ��߂̃N���X
 *			�Ƃ肠�����ǂ߂�΂�����I�Ȃ���
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#pragma once

class CIniFile
{
public:
	static const int MAX_KEY_LENGTH = 0x100;

	typedef struct {
		TCHAR *szKey;
		int *pnVariable;
		int nBase;
	} INI_KEY;

private:
	HANDLE m_hFile;
	TCHAR m_szFileName[MAX_PATH];
	const INI_KEY *m_pKeyTable;

	BOOL IsValidChar(TCHAR c);		// ���ʂ� int �ɂ��邩��
	BOOL IsSeparatorChar(TCHAR c);	// ���ʂ� int �ɂ��邩��

	void SkipToNextLine();
	void GetParam(TCHAR tcFirstChar);

public:
	CIniFile(void);
	virtual ~CIniFile(void);

	void SetFileName(const TCHAR *pFileName)   { _tcscat_s(m_szFileName, ARRAY_SIZE_OF(m_szFileName), pFileName); }
	void SetKeyTable(const INI_KEY *pKeyTable) { m_pKeyTable = pKeyTable; }
	BOOL Read();


};
