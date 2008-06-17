/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphTRDialog.cpp 133 2008-06-15 06:23:53Z Salt $
 * @brief	CGraphDialog ���p�����āA�_�O���t�̕`�������AReceived, Transmitted �ŏd�˂ĕ\����悤�ɕύX�����N���X
 *			�f�[�^�́A����ɂ�����p���������ō���Ă��炤
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
	// �ꉞ�`�F�b�N
	if (m_hdcMem == NULL)
		return TRUE;

	// �f�[�^�̎擾
	int nBarTransmitted, nBarReceived, nLine;
	BOOL bDraw = GetData(nBarTransmitted, nBarReceived, m_bPlotLineGraph, nLine);

	if (bDraw) {
		// �f�[�^����ꂽ��E�B���h�E��\������
		if (m_bHidPrev) {	// �O�� hide ��������
			ShowWindow(SW_SHOW);
			m_bHidPrev = FALSE;
		}

		// ���ɃX�N���[��
		if (m_nBitBltWidth)
			BitBlt(m_hdcMem, 0, 0, m_nBitBltWidth, m_nWindowHeight, m_hdcMem, m_nBarWidth, 0, SRCCOPY);


		// �ォ��`��
		RECT rect;
		rect.left   = m_nBitBltWidth;
		rect.right  = m_nBitBltWidth + m_nBarWidth;
		rect.top    = 0;

		int nYTransmitted = m_nWindowHeight - (m_nWindowHeight * nBarTransmitted) / 100;
		int nYReceived    = m_nWindowHeight - (m_nWindowHeight * nBarReceived) / 100;


		if (nYTransmitted >= nYReceived) {		// Y ���W���傫��������
			rect.bottom = nYReceived;
			FillSolidRect(m_hdcMem, m_clrBG, &rect);

			rect.top    = rect.bottom;
			rect.bottom = nYTransmitted;
			FillSolidRect(m_hdcMem, m_clrBar50, &rect);

			rect.top    = rect.bottom;
			rect.bottom = m_nWindowHeight;		// ���[�͕`�悳��Ȃ��݂���
			FillSolidRect(m_hdcMem, m_clrBar0, &rect);
		}
		else {
			rect.bottom = nYTransmitted;
			FillSolidRect(m_hdcMem, m_clrBG, &rect);

			rect.top    = rect.bottom;
			rect.bottom = nYReceived;
			FillSolidRect(m_hdcMem, m_clrBar0, &rect);

			rect.top    = rect.bottom;
			rect.bottom = m_nWindowHeight;		// ���[�͕`�悳��Ȃ��݂���
			FillSolidRect(m_hdcMem, m_clrBar50, &rect);
		}


		// ���O���t
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
