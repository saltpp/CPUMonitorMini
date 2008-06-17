/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphTRDialog.h 133 2008-06-15 06:23:53Z Salt $
 * @brief	CGraphDialog を継承して、棒グラフの描き方を、Received, Transmitted で重ねて表せるように変更したクラス
 *			データは、さらにこれを継承した方で作ってもらう
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#pragma once

#include "GraphDialog.h"

class CGraphTRDialog : public CGraphDialog {

private:
	int m_nYLinePrev;
	BOOL m_bHidPrev;


protected:
	// message handlers
	virtual BOOL OnTimer(HWND hWnd, int nIDEvent);

	int m_nElapseAutoDisconnect;
	int m_nCountAutoDisconnect;
	
public:
	CGraphTRDialog(void);
	virtual ~CGraphTRDialog(void);

	virtual BOOL GetData(int &nBarTransmitted, int &nBarReceived, BOOL bLine, int &nLine) = 0;		// have to return 0..100 for nBarXxxx and nLine

	void SetAutoDisconnectElapse(int nElapse) { m_nElapseAutoDisconnect = m_nTimerInterval ? (nElapse * 1000 / m_nTimerInterval) : 0; }
};
