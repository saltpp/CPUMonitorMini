/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphTRDialogRAS.h 133 2008-06-15 06:23:53Z Salt $
 * @brief	CGraphTRDialog を継承した RAS 情報を取得するクラス
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#pragma once

#include "GraphTRDialog.h"
#include <ras.h>
#include <regext.h>

class CGraphTRDialogRAS : public CGraphTRDialog {

private:
	int m_nMaxTransmitted;
	int m_nMaxReceived;

	HREGNOTIFY m_hrnRFStrength;
	HREGNOTIFY m_hrnModemCount;
	HREGNOTIFY m_hrnCellularCount;
	DWORD m_dwTickPrev;
	BOOL m_bDisabledPrev;
    HRASCONN m_hrasconn;
	DWORD m_dwRcvedPrev, m_dwXmitedPrev;
	void Disconnect();
	
	int m_nLastTransmitted;		// tap での情報表示用
	int m_nLastReceived;

protected:
	// message handlers
	virtual BOOL GetData(int &nBarTransmitted, int &nBarReceived, BOOL bLine, int &nLine);		// have to return 0..100 for nBarXxxx and nLine
	virtual BOOL OnInitDialog(HWND hWnd);
	virtual BOOL OnDestroy(HWND hWnd);
	virtual BOOL OnLButtonDown(HWND hWnd, POINT &point);


public:
	CGraphTRDialogRAS(void);
	virtual ~CGraphTRDialogRAS(void);

	// for callback
	static int m_nModemCount;
	static void RegistryNotifyCallbackFuncModemCount(HREGNOTIFY hNotify, DWORD dwUserData, const PBYTE pData, const UINT cbData);
	static int m_nCellularCount;
	static void RegistryNotifyCallbackFuncCellularCount(HREGNOTIFY hNotify, DWORD dwUserData, const PBYTE pData, const UINT cbData);
	static DWORD m_dwRFStrength;
	static void RegistryNotifyCallbackFuncRFStrength(HREGNOTIFY hNotify, DWORD dwUserData, const PBYTE pData, const UINT cbData);

	// for user settings
	void SetMaxTransmitted(int n) { m_nMaxTransmitted = n; }
	void SetMaxReceived(int n)    { m_nMaxReceived    = n; }
	void NormalizeUserSettings(void);
};
