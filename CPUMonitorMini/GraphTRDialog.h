/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphTRDialog.h 118 2008-05-11 11:26:18Z Shiono $
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


public:
	CGraphTRDialog(void);
	virtual ~CGraphTRDialog(void);

	virtual BOOL GetData(int &nBarTransmitted, int &nBarReceived, BOOL bLine, int &nLine) = 0;		// have to return 0..100 for nBarXxxx and nLine
};
