/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphTRDialogRAS.cpp 131 2008-05-18 10:56:20Z Shiono $
 * @brief	CGraphTRDialog ���p������ RAS �����擾����N���X
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#include "StdAfx.h"
#include "GraphTRDialogRAS.h"
#include "CPUMonitorMini.h"
#include <ras.h>
#include <regext.h>



// static �ϐ���������
int CGraphTRDialogRAS::m_nModemCount    = 0;
int CGraphTRDialogRAS::m_nCellularCount = 0;
DWORD CGraphTRDialogRAS::m_dwRFStrength = 0;


CGraphTRDialogRAS::CGraphTRDialogRAS(void)
: m_bDisabledPrev(TRUE)
, m_hrasconn(0)
, m_hrnModemCount(NULL)
, m_hrnCellularCount(NULL)
, m_hrnRFStrength(NULL)
{
}


CGraphTRDialogRAS::~CGraphTRDialogRAS(void)
{
}


void CGraphTRDialogRAS::RegistryNotifyCallbackFuncModemCount(HREGNOTIFY hNotify, DWORD dwUserData, const PBYTE pData, const UINT cbData)
{
	m_nModemCount = *((DWORD *) pData);
}

void CGraphTRDialogRAS::RegistryNotifyCallbackFuncCellularCount(HREGNOTIFY hNotify, DWORD dwUserData, const PBYTE pData, const UINT cbData)
{
	m_nCellularCount = *((DWORD *) pData);
}


void CGraphTRDialogRAS::RegistryNotifyCallbackFuncRFStrength(HREGNOTIFY hNotify, DWORD dwUserData, const PBYTE pData, const UINT cbData)
{
	m_dwRFStrength = *((DWORD *) pData) * 100 / 6;	// ades �� 6 �i�K
}


BOOL CGraphTRDialogRAS::OnInitDialog(HWND hWnd)
{

	// Registory �ɕω����������� notify �����悤�ɓo�^
	DWORD dwTemp;

	// Modem\Count
	RegistryNotifyCallback(HKEY_LOCAL_MACHINE,
							_T("System\\State\\Connections\\Modem"),	// hTc-Z (X01HT �����H�j�� Cellular, ZERO3, EM-1, X01T �� Modemo �� Count �� 1 �ɂȂ�
							_T("Count"),
							RegistryNotifyCallbackFuncModemCount,
							0,
							NULL,
							&m_hrnModemCount);				// handle
	// �����l��ǂݍ���ł���
	RegistryGetDWORD(HKEY_LOCAL_MACHINE,
						_T("System\\State\\Connections\\Modem"),
						_T("Count"),
						&dwTemp);
	m_nModemCount = (int) dwTemp;

	// Modem\Cellular
	RegistryNotifyCallback(HKEY_LOCAL_MACHINE,
							_T("System\\State\\Connections\\Cellular"),
							_T("Count"),
							RegistryNotifyCallbackFuncCellularCount,
							0,
							NULL,
							&m_hrnModemCount);				// handle
	// �����l��ǂݍ���ł���
	RegistryGetDWORD(HKEY_LOCAL_MACHINE,
						_T("System\\State\\Connections\\Cellular"),
						_T("Count"),
						&dwTemp);
	m_nCellularCount = (int) dwTemp;


	// RFStrength
	if (m_bPlotLineGraph) {
		HRESULT hResult = RegistryNotifyCallback(HKEY_CURRENT_USER,
													_T("Software\\Sharp\\ShPhoneLib"),
													_T("RFStrength"),
													RegistryNotifyCallbackFuncRFStrength,
													0,
													NULL,
													&m_hrnRFStrength);	// handle
		if (hResult == S_OK) {
			// �����l��ǂݍ���ł���
			RegistryGetDWORD(HKEY_CURRENT_USER,
								_T("Software\\Sharp\\ShPhoneLib"),
								_T("RFStrength"),
								&dwTemp);
			RegistryNotifyCallbackFuncRFStrength(NULL, 0L, (const PBYTE) &dwTemp, 0L);	// callback �֐��������ŌĂ�Őݒ�
		}
		else {
			// ���Ԃ� ZERO3 ����Ȃ�
			m_bPlotLineGraph = FALSE;
		}
	}

	return __super::OnInitDialog(hWnd);
}


BOOL CGraphTRDialogRAS::OnDestroy(HWND hWnd)
{
	// RFStrength
	if (m_bPlotLineGraph) {
		if (m_hrnRFStrength) {
			RegistryCloseNotification(m_hrnRFStrength);
			m_hrnRFStrength = NULL;
		}
	}

	if (m_hrnCellularCount) {
		RegistryCloseNotification(m_hrnCellularCount);
		m_hrnCellularCount = NULL;
	}

	if (m_hrnModemCount) {
		RegistryCloseNotification(m_hrnModemCount);
		m_hrnModemCount = NULL;
	}

	return __super::OnDestroy(hWnd);
}


void CGraphTRDialogRAS::NormalizeUserSettings(void)
{
	m_nMaxTransmitted = min(max(1, m_nMaxTransmitted), 10000);
	m_nMaxReceived    = min(max(1, m_nMaxReceived),    10000);

	__super::NormalizeUserSettings();
}


BOOL CGraphTRDialogRAS::GetData(int &nBarTransmitted, int &nBarReceived, BOOL bLine, int &nLine)
{
	if (m_nModemCount + m_nCellularCount > 0) {

		BOOL bFound = FALSE;

		DWORD dwTick = GetTickCount();

		if (dwTick != m_dwTickPrev) {
			int nDT = dwTick - m_dwTickPrev;	// in msec
			m_dwTickPrev = dwTick;

			// RAS
			RASCONN rasconn[2];
			rasconn[0].dwSize = sizeof(RASCONN);	// 52
			DWORD dwCB = sizeof(rasconn);			// �����͑S�̕����w��
			DWORD dwConnections = 0;
			RasEnumConnections(rasconn, &dwCB, &dwConnections);

			for (int i = 0; i < (int) dwConnections; ++i) {
				if (_tcsncmp(rasconn[i].szEntryName, _T("`USB"), 4)) {	// registry ������ׂ����ȁB `USB �� ZERO3 �����H�@HKCU/ControlPanel/Comm/Cnct="`USB"
					// "'USB" �łȂ���΁AWAN ���낤
					m_hrasconn = rasconn[i].hrasconn;
					RAS_STATS rasstats;
					rasstats.dwSize = sizeof(RAS_STATS);
					RasGetLinkStatistics(m_hrasconn, 0, &rasstats);


					int n;
					// Byte �P�ʂŕԂ��Ă���̂� �~8 �� bit �P��
					// nDT �� ms �P�ʂȂ̂ŁA����Ŋ��邽�߂ɁA��� * 1000 ���Ă����inDT �� 1000 �Ŋ����āAdouble �Ƃ��ɓ���Ă��������BARM �� double �͒x�����Ȃ̂ł�߂Ă����B�j
					// Kbps �P�ʂɂ������̂ŁA1000 �Ŋ���B�i�덷�����炷���߂ɍŌ�Ŋ���Bk=1000 ����Ȃ��� K=1024 ����ʓI���Ǝv�������Abps �n�� k, K �����Ƃ� 1000 ����ˁH�j
					// m_nMax �Ŏw�肳�ꂽ�l�� 100% �ɂ������̂� 100 / m_nMax ���|����

					// n = ((rasstats.dwBytesXmited - m_dwXmitedPrev) * 8 * 1000 / (nDT * m_nMaxTransmitted)) * 100 / 1000;	// 1024;		// zero divide ���ӁANormalize �� 0 �ȊO�ɂȂ�͂�   // Byte �ŕԂ��Ă���
					m_nLastTransmitted = (rasstats.dwBytesXmited - m_dwXmitedPrev) * 8 * 1000 / nDT;	// tap �����Ƃ��̏��\���p�ɁA��U�ۑ�
					n = m_nLastTransmitted * 100 / m_nMaxTransmitted / 1000;	// 1024;
					n = min(max(0, n), 100);
					nBarTransmitted = n;

					// n = ((rasstats.dwBytesRcved - m_dwRcvedPrev)   * 8 * 1000 / (nDT * m_nMaxReceived))    * 100 / 1000;	// 1024;		// 1000�`3000 ���炢�����o�Ȃ���
					m_nLastReceived = (rasstats.dwBytesRcved - m_dwRcvedPrev) * 8 * 1000 / nDT;			// tap �����Ƃ��̏��\���p�ɁA��U�ۑ�
					n = m_nLastReceived * 100 / m_nMaxReceived / 1000;	// 1024;
					n = min(max(0, n), 100);
					nBarReceived = n;

					m_dwRcvedPrev  = rasstats.dwBytesRcved;
					m_dwXmitedPrev = rasstats.dwBytesXmited;

					// �P��ڂ́A���������������ĂȂ��̂ŏ���
					if (m_bDisabledPrev) {
						m_bDisabledPrev = FALSE;
						nBarTransmitted = 0;
						nBarReceived    = 0;
					}

					// Line Graph
					// if (bLine)	// �v��Ȃ���B��r���邾�����ʂ��B
						nLine = m_dwRFStrength;

					bFound = TRUE;
				}
			}

		}
		if (bFound) {
			return TRUE;
		}
		else {
			m_bDisabledPrev = TRUE;
			m_hrasconn = NULL;
			return FALSE;
		}

	}
	else
		return FALSE;
}


BOOL CGraphTRDialogRAS::OnLButtonDown(HWND hWnd, POINT &point)
{
	TCHAR szBuf[0x100];		/// @TODO �o�b�t�@�T�C�Y�͂ǂ����� #define ���悤
	_stprintf(szBuf, _T("Transmitted : %.1lfKbps\n")
					 _T("Received    : %.1lfKbps\n")
					 _T("\n\n")
					 _T("Do you want to disconnect RAS connection?"),
					 (double) m_nLastTransmitted / 1000,	// 1024,
					 (double) m_nLastReceived    / 1000);	// 1024);

	int nResult = MessageBox(hWnd, szBuf, PROGRAM_NAME _T(" ") PROGRAM_VERSION, MB_OKCANCEL);
	
	if (nResult == IDOK) {
		if (m_hrasconn) {
			RasHangUp(m_hrasconn);
		}
	}
	return TRUE;
}
