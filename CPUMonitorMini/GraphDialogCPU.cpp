/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphDialogCPU.cpp 116 2008-05-11 09:41:37Z Shiono $
 * @brief	CGraphDialog Çåpè≥ÇµÇΩ CPU èÓïÒÇéÊìæÇ∑ÇÈÉNÉâÉX
 *			CPU èÓïÒÇ∆ Memory èÓïÒÇï‘Ç∑
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#include "StdAfx.h"
#include "GraphDialogCPU.h"
#include "CPUMonitorMini.h"

CGraphDialogCPU::CGraphDialogCPU(void)
{
}


CGraphDialogCPU::~CGraphDialogCPU(void)
{
}


void CGraphDialogCPU::NormalizeUserSettings(void)
{
	// 0Å`100 Ç…êßå¿Ç∑ÇÈ
	m_nMinFreeMemory = min(max(0, m_nMinFreeMemory), 100);
	m_nMaxFreeMemory = min(max(0, m_nMaxFreeMemory), 100);

	// min, max Ç™ãtÇ»ÇÁì¸ÇÍë÷Ç¶ÇƒÇ®Ç≠
	if (m_nMinFreeMemory > m_nMaxFreeMemory) {
		int n = m_nMinFreeMemory;
		m_nMinFreeMemory = m_nMaxFreeMemory;
		m_nMaxFreeMemory = n;
	}

	if (m_nMaxFreeMemory == m_nMinFreeMemory) {
		m_nMaxFreeMemory = 100;
		m_nMinFreeMemory = 0;
	}

	__super::NormalizeUserSettings();
}


BOOL CGraphDialogCPU::GetData(int &nBar, int &nReserved, BOOL bLine, int &nLine)
{
	DWORD dwTick = GetTickCount();
	DWORD dwIdle = GetIdleTime();
	if (dwTick != m_dwTickPrev) {
		int nDT = dwTick - m_dwTickPrev;	// in msec

		// CPU Usage
		nBar = 100 - (100 * (dwIdle - m_dwIdlePrev) / nDT);

		m_dwTickPrev = dwTick;
		m_dwIdlePrev = dwIdle;

		if (bLine) {
			// Memory
			MEMORYSTATUS MemStatus;
			GlobalMemoryStatus(&MemStatus);

			nLine = ((100 - (int) MemStatus.dwMemoryLoad) - m_nMinFreeMemory) * 100 / (m_nMaxFreeMemory - m_nMinFreeMemory);		// dwMemoryLoad 100 = écÇËÉÅÉÇÉäñ≥Çµ
			nLine = min(max(0, nLine), 100);																						// 0Å`100 ÇÃîÕàÕÇ…é˚ÇﬂÇÈ
		}

		return TRUE;
	}
	else
		return FALSE;
}


BOOL CGraphDialogCPU::OnLButtonDown(HWND hWnd, POINT &point)
{
	MEMORYSTATUS MemStatus;
	GlobalMemoryStatus(&MemStatus);

	STORE_INFORMATION StoreInfo;
	GetStoreInformation(&StoreInfo);

	TCHAR szBuf[0x100];
	_stprintf(szBuf, _T("Memory Free   : %.1lfMB\n")
					 _T("Memory Total  : %.1lfMB\n")
					 _T("Storage Free  : %.1lfMB\n")
					 _T("Storage Total : %.1lfMB\n"),
					 (double) MemStatus.dwAvailPhys / 1024 / 1024,
					 (double) MemStatus.dwTotalPhys / 1024 / 1024,
					 (double) StoreInfo.dwFreeSize  / 1024 / 1024,
					 (double) StoreInfo.dwStoreSize / 1024 / 1024);
	MessageBox(hWnd, szBuf, PROGRAM_NAME _T(" ") PROGRAM_VERSION, MB_OK);

	return TRUE;
}
