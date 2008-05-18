/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphDialog.cpp 116 2008-05-11 09:41:37Z Shiono $
 * @brief	CPopupDialog を継承したグラフを描くためのクラス
 *			データは、さらにこれを継承した方で作ってもらう
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#include "StdAfx.h"
#include "GraphDialog.h"



CGraphDialog::CGraphDialog(void)
: m_nYLinePrev(-1)
, m_hdcMem(NULL)
, m_hdcMemBar(NULL)
, m_pclrBar(NULL)
{
}


CGraphDialog::~CGraphDialog(void)
{
}


void CGraphDialog::NormalizeUserSettings(void)
{
	m_nBarWidth = min(max(1, m_nBarWidth), m_nWindowWidth);
	m_nBitBltWidth = m_nWindowWidth - m_nBarWidth;

	__super::NormalizeUserSettings();
}


void CGraphDialog::FillSolidRect(HDC hDC, COLORREF clr, RECT *pRect)
{
	SetBkColor(hDC, clr);
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, pRect, NULL, 0, NULL);
}


/// @param[in]	nMax		最大ドット数
/// @param[in]	nValue		どこのドットの色を返すか　0 = 100%, nMax = 0%
/// @return		COLORREF	対応する色
COLORREF CGraphDialog::GetGradationColor(int nValue, int nMax)
{
	int nCenter = nMax / 2;
	int r, g, b;
	if (nValue <= nCenter) {
		// 100%～50%
		//r = ((((m_clrBar50 & 0x0000FF) >> 0)  * nValue + ((m_clrBar100 & 0x0000FF) >> 0)  * (nCenter - nValue))) / nCenter;
		//g = ((((m_clrBar50 & 0x00FF00) >> 8)  * nValue + ((m_clrBar100 & 0x00FF00) >> 8)  * (nCenter - nValue))) / nCenter;
		//b = ((((m_clrBar50 & 0xFF0000) >> 16) * nValue + ((m_clrBar100 & 0xFF0000) >> 16) * (nCenter - nValue))) / nCenter;

		// わざわざ shift しなくても計算はできる。あふれるかもしれないけど。
		r = ((m_clrBar50 & 0x0000FF) * nValue + (m_clrBar100 & 0x0000FF) * (nCenter - nValue)) / nCenter;
		g = ((m_clrBar50 & 0x00FF00) * nValue + (m_clrBar100 & 0x00FF00) * (nCenter - nValue)) / nCenter;
		b = ((m_clrBar50 & 0xFF0000) * nValue + (m_clrBar100 & 0xFF0000) * (nCenter - nValue)) / nCenter;		/// @TODO WindowHeight が 512 超える場合は、これがあふれるはずなので注意。現状、画面高さは480に抑えてて、nCenter = 240 しか来ないので、あふれない。
	}
	else {
		// 50%～0%
		int n = nValue - nCenter;
		//r = ((((m_clrBar0 & 0x0000FF) >> 0)  * n + ((m_clrBar50 & 0x0000FF) >> 0)  * (nCenter - n))) / nCenter;
		//g = ((((m_clrBar0 & 0x00FF00) >> 8)  * n + ((m_clrBar50 & 0x00FF00) >> 8)  * (nCenter - n))) / nCenter;
		//b = ((((m_clrBar0 & 0xFF0000) >> 16) * n + ((m_clrBar50 & 0xFF0000) >> 16) * (nCenter - n))) / nCenter;

		// わざわざ shift しなくても計算はできる。あふれるかもしれないけど。
		r = ((m_clrBar0 & 0x0000FF) * n + (m_clrBar50 & 0x0000FF) * (nCenter - n)) / nCenter;
		g = ((m_clrBar0 & 0x00FF00) * n + (m_clrBar50 & 0x00FF00) * (nCenter - n)) / nCenter;
		b = ((m_clrBar0 & 0xFF0000) * n + (m_clrBar50 & 0xFF0000) * (nCenter - n)) / nCenter;		/// @TODO WindowHeight が 512 超える場合は、これがあふれるはずなので注意。現状、画面高さは480に抑えてて、nCenter = 240 しか来ないので、あふれない。
	}

	// return (COLORREF) ((b << 16) | (g << 8) | r); 
	return (COLORREF) ((b & 0xFF0000) | (g  & 0x00FF00) | (r & 0x0000FF)); 
}

void CGraphDialog::CreateMemDC(HWND hwnd)
{
	HDC hdc = GetDC(hwnd);

	m_hdcMem      = CreateCompatibleDC(hdc);
	m_hbmpMem     = CreateCompatibleBitmap(hdc, m_nWindowWidth, m_nWindowHeight);
	m_hbmpMemPrev = (HBITMAP) SelectObject(m_hdcMem, m_hbmpMem);

	if (m_bGradation && !m_bFillBar) {
		m_hdcMemBar      = CreateCompatibleDC(hdc);
		m_hbmpMemBar     = CreateCompatibleBitmap(hdc, m_nBarWidth, m_nWindowHeight);
		m_hbmpMemBarPrev = (HBITMAP) SelectObject(m_hdcMemBar, m_hbmpMemBar);
	}
	else {
		m_pclrBar = new COLORREF[NUM_COLOR];
	}

	ReleaseDC(hwnd, hdc);


	// MemDC を塗りつぶして消しておく
	RECT rect = {0};
	rect.right  = m_nWindowWidth;		// 右端は描画されないみたい
	rect.bottom = m_nWindowHeight;		// 下端は描画されないみたい
	FillSolidRect(m_hdcMem, m_clrBG, &rect);

	if (m_bGradation && !m_bFillBar) {
		// Bar 用の MemDC にグラデーションを描いておく
		for (int i = 0; i < m_nWindowHeight; ++i) {
			COLORREF clr = GetGradationColor(i, m_nWindowHeight);
			HANDLE hPen = CreatePen(PS_SOLID, 1, clr);
			HANDLE hPenPrev = SelectObject(m_hdcMemBar, hPen);
			MoveToEx(m_hdcMemBar, 0, i, NULL);
			LineTo(m_hdcMemBar, m_nBarWidth, i);
			SelectObject(m_hdcMemBar, hPenPrev);
			DeleteObject(hPen);
		}
	}
	else {
		// グラデーションしないときは、色を配列に入れておく
		for (int i = 0; i < NUM_COLOR; ++i) {
			m_pclrBar[i] = GetGradationColor(100 - i, 100);
		}
		
	}

	// for line graph
	if (m_bPlotLineGraph) {
		m_hpenMem = CreatePen(PS_SOLID, 1, m_clrLineGraph);
		m_hpenPrev = (HPEN) SelectObject(m_hdcMem, m_hpenMem);
		// SetROP2(m_hdcMem, R2_XORPEN);
	}
}


void CGraphDialog::DeleteMemDC(void)
{
	if (m_bPlotLineGraph) {
		// SetROP2(m_hdcMem, R2_COPYPEN);
		SelectObject(m_hdcMem, m_hpenPrev);
		DeleteObject(m_hpenMem);
	}

	if (m_hdcMemBar) {
		SelectObject(m_hdcMemBar, m_hbmpMemBarPrev);
		DeleteObject(m_hbmpMemBar);
		DeleteObject(m_hdcMemBar);
		m_hdcMemBar = NULL;
	}

	if (m_hdcMem) {
		SelectObject(m_hdcMem, m_hbmpMemPrev);
		DeleteObject(m_hbmpMem);
		DeleteObject(m_hdcMem);
		m_hdcMem = NULL;
	}

	if (m_pclrBar) {
		delete [] m_pclrBar;
	}
}


BOOL CGraphDialog::OnInitDialog(HWND hWnd)
{
	CreateMemDC(hWnd);
	SetTimer(hWnd, m_nTimerID, m_nTimerInterval, NULL);

	return __super::OnInitDialog(hWnd);
}


BOOL CGraphDialog::OnDestroy(HWND hWnd)
{
	KillTimer(hWnd, m_nTimerID);
	DeleteMemDC();
	PostQuitMessage(0);

	return TRUE;
}


BOOL CGraphDialog::OnPaint(HWND hWnd)
{
    PAINTSTRUCT     ps;
    HDC hDC = BeginPaint(hWnd, &ps);

	BitBlt(hDC, 0, 0, m_nWindowWidth, m_nWindowHeight, m_hdcMem, 0, 0, SRCCOPY);


	EndPaint(hWnd, &ps);
	
	return TRUE;
}


BOOL CGraphDialog::OnEraseBkgnd(HDC hDC)
{
	return TRUE;		// 何もせずに、処理したことにする。　OS 側で処理されるとちらつくので。
}

void CGraphDialog::DrawLineGraph(int nValue, int nLeft, int nRight)
{
	int nY = m_nWindowHeight - (m_nWindowHeight * nValue) / 100;
	if (m_nYLinePrev == -1) {
		m_nYLinePrev = nY;
	}
	else {
		MoveToEx(m_hdcMem, nLeft - 1, m_nYLinePrev, NULL);

		LineTo(m_hdcMem, nRight - 1, nY);
		LineTo(m_hdcMem, nRight, nY);
		
		m_nYLinePrev = nY;
	}
}

BOOL CGraphDialog::OnTimer(HWND hWnd, int nIDEvent)
{
	//BringWindowToTop(hWnd);
	//::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW);	// salttest


	// 一応チェック
	if (m_hdcMem == NULL)
		return TRUE;

	// 左にスクロール
	if (m_nBitBltWidth)
		BitBlt(m_hdcMem, 0, 0, m_nBitBltWidth, m_nWindowHeight, m_hdcMem, m_nBarWidth, 0, SRCCOPY);

	// データの取得
	int nBar, nLine, nDummy;
	BOOL bDraw = GetData(nBar, nDummy, m_bPlotLineGraph, nLine);

	if (bDraw) {
		// 上から描画
		RECT rect;
		rect.left   = m_nBitBltWidth;
		rect.right  = m_nBitBltWidth + m_nBarWidth;
		rect.top    = 0;

		if (m_bFillBar) {
			// 塗りつぶす
			rect.bottom = m_nWindowHeight;
			FillSolidRect(m_hdcMem, m_pclrBar[nBar], &rect);
		}
		else {
			// 普通に棒グラフを描画
			rect.bottom = m_nWindowHeight - (m_nWindowHeight * nBar) / 100;
			FillSolidRect(m_hdcMem, m_clrBG, &rect);

			rect.top    = rect.bottom;
			rect.bottom = m_nWindowHeight;
			if (m_bGradation) {
				// グラデーション部分を転送する
				BitBlt(m_hdcMem, m_nBitBltWidth, rect.top, m_nBarWidth + 1, m_nWindowHeight - rect.top, m_hdcMemBar, 0, rect.top, SRCCOPY);
			}
			else {
				// 色を求めて、塗りつぶす
				FillSolidRect(m_hdcMem, m_pclrBar[nBar], &rect);
			}
		}

		// 線グラフ
		if (m_bPlotLineGraph) {
			DrawLineGraph(nLine, rect.left, rect.right);
		}

		InvalidateRect(hWnd, NULL, TRUE);
	}
	return TRUE;
}
