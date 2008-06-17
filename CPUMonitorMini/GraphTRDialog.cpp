/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphTRDialog.cpp 133 2008-06-15 06:23:53Z Salt $
 * @brief	CGraphDialog を継承して、棒グラフの描き方を、Received, Transmitted で重ねて表せるように変更したクラス
 *			データは、さらにこれを継承した方で作ってもらう
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#include "StdAfx.h"
#include "GraphTRDialog.h"



CGraphTRDialog::CGraphTRDialog(void)
: m_nYLinePrev(-1)
, m_bHidPrev(TRUE)
, m_nElapseAutoDisconnect(0)
, m_nCountAutoDisconnect(0)
{
}


CGraphTRDialog::~CGraphTRDialog(void)
{
}


BOOL CGraphTRDialog::OnTimer(HWND hWnd, int nIDEvent)
{
	// 一応チェック
	if (m_hdcMem == NULL)
		return TRUE;

	// データの取得
	int nBarTransmitted, nBarReceived, nLine;
	BOOL bDraw = GetData(nBarTransmitted, nBarReceived, m_bPlotLineGraph, nLine);

	if (bDraw) {
		// データが取れたらウィンドウを表示する
		if (m_bHidPrev) {	// 前回 hide だったら
			ShowWindow(SW_SHOW);
			m_bHidPrev = FALSE;
		}

		// 左にスクロール
		if (m_nBitBltWidth)
			BitBlt(m_hdcMem, 0, 0, m_nBitBltWidth, m_nWindowHeight, m_hdcMem, m_nBarWidth, 0, SRCCOPY);


		// 上から描画
		RECT rect;
		rect.left   = m_nBitBltWidth;
		rect.right  = m_nBitBltWidth + m_nBarWidth;
		rect.top    = 0;

		int nYTransmitted = m_nWindowHeight - (m_nWindowHeight * nBarTransmitted) / 100;
		int nYReceived    = m_nWindowHeight - (m_nWindowHeight * nBarReceived) / 100;


		if (nYTransmitted >= nYReceived) {		// Y 座標が大きい方から
			rect.bottom = nYReceived;
			FillSolidRect(m_hdcMem, m_clrBG, &rect);

			rect.top    = rect.bottom;
			rect.bottom = nYTransmitted;
			FillSolidRect(m_hdcMem, m_clrBar50, &rect);

			rect.top    = rect.bottom;
			rect.bottom = m_nWindowHeight;		// 下端は描画されないみたい
			FillSolidRect(m_hdcMem, m_clrBar0, &rect);
		}
		else {
			rect.bottom = nYTransmitted;
			FillSolidRect(m_hdcMem, m_clrBG, &rect);

			rect.top    = rect.bottom;
			rect.bottom = nYReceived;
			FillSolidRect(m_hdcMem, m_clrBar0, &rect);

			rect.top    = rect.bottom;
			rect.bottom = m_nWindowHeight;		// 下端は描画されないみたい
			FillSolidRect(m_hdcMem, m_clrBar50, &rect);
		}


		// 線グラフ
		if (m_bPlotLineGraph) {
			DrawLineGraph(nLine, rect.left, rect.right);
		}

		InvalidateRect(hWnd, NULL, TRUE);

	}
	else {

		ShowWindow(SW_HIDE);
		m_bHidPrev = TRUE;

	}

	return TRUE;
}
