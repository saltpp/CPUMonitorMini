/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphDialog.h 133 2008-06-15 06:23:53Z Salt $
 * @brief	CPopupDialog を継承したグラフを描くためのクラス
 *			データは、さらにこれを継承した方で作ってもらう
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#pragma once

#include <windows.h>
#include "PopupDialog.h"


class CGraphDialog : public CPopupDialog
{

public:
	static const int MAX_HISTORY_NUMBER = 32;
	static const int NUM_COLOR = 101;		// 0..100

private:
	int m_nTimerID;

	int m_nYLinePrev;

	void CreateMemDC(HWND hwnd);
	void DeleteMemDC(void);
	COLORREF GetGradationColor(int nValue, int nMax);
	COLORREF *m_pclrBar;

protected:
	int m_nTimerInterval;

	HDC     m_hdcMem;
	HBITMAP m_hbmpMem;
	HBITMAP m_hbmpMemPrev;

	HDC     m_hdcMemBar;
	HBITMAP m_hbmpMemBar;
	HBITMAP m_hbmpMemBarPrev;

	HPEN    m_hpenMem;
	HPEN    m_hpenPrev;
	int     m_nBarWidth;	// 0..m_nWindowWidth
	int     m_nBitBltWidth;

	COLORREF m_clrBG;
	COLORREF m_clrBar0;
	COLORREF m_clrBar50;
	COLORREF m_clrBar100;
	BOOL     m_bGradation;
	BOOL     m_bFillBar;

	BOOL     m_bPlotLineGraph;
	COLORREF m_clrLineGraph;

	void FillSolidRect(HDC hDC, COLORREF clr, RECT *pRect);
	void DrawLineGraph(int nValue, int nLeft, int nRight);

	// message handlers
	virtual BOOL OnInitDialog(HWND hWnd);
	virtual BOOL OnDestroy(HWND hWnd);
	virtual BOOL OnPaint(HWND hWnd);
	virtual BOOL OnEraseBkgnd(HDC hDC);
	virtual BOOL OnTimer(HWND hWnd, int nIDEvent);

	virtual BOOL GetData(int &nBar, int &nReserved, BOOL bLine, int &nLine) = 0;		// have to return 0..100 for nBar and nLine


public:
	CGraphDialog(void);
	virtual ~CGraphDialog(void);

	void SetTimerID(int n)                                           { m_nTimerID              = n;   }

	// for user settings
	void SetTimerInterval(int n)                                     { m_nTimerInterval        = n;   }
	void SetBarWidth(int n)                                          { m_nBarWidth             = n;   }
	void SetColorBG(COLORREF clr)                                    { m_clrBG                 = clr; }
	void SetColorBar(COLORREF clr0, COLORREF clr50, COLORREF clr100) { m_clrBar0 = clr0; m_clrBar50 = clr50; m_clrBar100 = clr100; }
	void SetGradation(BOOL b)                                        { m_bGradation            = b;   }
	void SetFillBar(BOOL b)                                          { m_bFillBar              = b;   }
	void SetPlotLineGraph(BOOL b)                                    { m_bPlotLineGraph        = b;   }
	void SetColorLineGraph(COLORREF clr)                             { m_clrLineGraph          = clr; }
	void NormalizeUserSettings(void);

};
