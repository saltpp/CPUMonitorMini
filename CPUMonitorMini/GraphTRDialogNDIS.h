/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphTRDialogNDIS.h 133 2008-06-15 06:23:53Z Salt $
 * @brief	CGraphTRDialog を継承した NDIS 情報を取得するクラス
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#pragma once

#include "GraphTRDialog.h"
#include <regext.h>

class CGraphTRDialogNDIS : public CGraphTRDialog {

private:
	int m_nMaxTransmitted;
	int m_nMaxReceived;

	HANDLE m_hNDIS;
	TCHAR m_szWlanDeviceName[0x100];	/// @TODO サイズはどっかで指定されているか？
	BOOL SearchWlanDevice();
	void CloseNdisUio();
	BOOL OpenNdisUio();
	//BOOL GetStatistics();
	LONG GetSignalLevel();
	void TurnWLANPowerOff();

	DWORD m_dwTickPrev;
	BOOL m_bDisabledPrev;
	int m_nLinePrev;

	HREGNOTIFY m_hrnNetworkCount;

	int m_nLastTransmitted;		// tap での情報表示用
	int m_nLastReceived;


protected:
	// message handlers
	virtual BOOL GetData(int &nBarTransmitted, int &nBarReceived, BOOL bLine, int &nLine);		// have to return 0..100 for nBarXxxx and nLine
	virtual BOOL OnInitDialog(HWND hWnd);
	virtual BOOL OnDestroy(HWND hWnd);
	virtual BOOL OnLButtonDown(HWND hWnd, POINT &point);


public:
	CGraphTRDialogNDIS(void);
	virtual ~CGraphTRDialogNDIS(void);

	// for callback
	static int m_nNDISCount;
	static void RegistryNotifyCallbackFuncNetworkCount(HREGNOTIFY hNotify, DWORD dwUserData, const PBYTE pData, const UINT cbData);

	// for user settings
	void SetMaxTransmitted(int n) { m_nMaxTransmitted = n; }
	void SetMaxReceived(int n)    { m_nMaxReceived    = n; }
	void NormalizeUserSettings(void);
};
