/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphDialog.cpp 116 2008-05-11 09:41:37Z Shiono $
 * @brief	CPopupDialog ���p�������O���t��`�����߂̃N���X
 *			�f�[�^�́A����ɂ�����p���������ō���Ă��炤
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


/// @param[in]	nMax		�ő�h�b�g��
/// @param[in]	nValue		�ǂ��̃h�b�g�̐F��Ԃ����@0 = 100%, nMax = 0%
/// @return		COLORREF	�Ή�����F
COLORREF CGraphDialog::GetGradationColor(int nValue, int nMax)
{
	int nCenter = nMax / 2;
	int r, g, b;
	if (nValue <= nCenter) {
		// 100%�`50%
		//r = ((((m_clrBar50 & 0x0000FF) >> 0)  * nValue + ((m_clrBar100 & 0x0000FF) >> 0)  * (nCenter - nValue))) / nCenter;
		//g = ((((m_clrBar50 & 0x00FF00) >> 8)  * nValue + ((m_clrBar100 & 0x00FF00) >> 8)  * (nCenter - nValue))) / nCenter;
		//b = ((((m_clrBar50 & 0xFF0000) >> 16) * nValue + ((m_clrBar100 & 0xFF0000) >> 16) * (nCenter - nValue))) / nCenter;

		// �킴�킴 shift ���Ȃ��Ă��v�Z�͂ł���B���ӂ�邩������Ȃ����ǁB
		r = ((m_clrBar50 & 0x0000FF) * nValue + (m_clrBar100 & 0x0000FF) * (nCenter - nValue)) / nCenter;
		g = ((m_clrBar50 & 0x00FF00) * nValue + (m_clrBar100 & 0x00FF00) * (nCenter - nValue)) / nCenter;
		b = ((m_clrBar50 & 0xFF0000) * nValue + (m_clrBar100 & 0xFF0000) * (nCenter - nValue)) / nCenter;		/// @TODO WindowHeight �� 512 ������ꍇ�́A���ꂪ���ӂ��͂��Ȃ̂Œ��ӁB����A��ʍ�����480�ɗ}���ĂāAnCenter = 240 �������Ȃ��̂ŁA���ӂ�Ȃ��B
	}
	else {
		// 50%�`0%
		int n = nValue - nCenter;
		//r = ((((m_clrBar0 & 0x0000FF) >> 0)  * n + ((m_clrBar50 & 0x0000FF) >> 0)  * (nCenter - n))) / nCenter;
		//g = ((((m_clrBar0 & 0x00FF00) >> 8)  * n + ((m_clrBar50 & 0x00FF00) >> 8)  * (nCenter - n))) / nCenter;
		//b = ((((m_clrBar0 & 0xFF0000) >> 16) * n + ((m_clrBar50 & 0xFF0000) >> 16) * (nCenter - n))) / nCenter;

		// �킴�킴 shift ���Ȃ��Ă��v�Z�͂ł���B���ӂ�邩������Ȃ����ǁB
		r = ((m_clrBar0 & 0x0000FF) * n + (m_clrBar50 & 0x0000FF) * (nCenter - n)) / nCenter;
		g = ((m_clrBar0 & 0x00FF00) * n + (m_clrBar50 & 0x00FF00) * (nCenter - n)) / nCenter;
		b = ((m_clrBar0 & 0xFF0000) * n + (m_clrBar50 & 0xFF0000) * (nCenter - n)) / nCenter;		/// @TODO WindowHeight �� 512 ������ꍇ�́A���ꂪ���ӂ��͂��Ȃ̂Œ��ӁB����A��ʍ�����480�ɗ}���ĂāAnCenter = 240 �������Ȃ��̂ŁA���ӂ�Ȃ��B
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


	// MemDC ��h��Ԃ��ď����Ă���
	RECT rect = {0};
	rect.right  = m_nWindowWidth;		// �E�[�͕`�悳��Ȃ��݂���
	rect.bottom = m_nWindowHeight;		// ���[�͕`�悳��Ȃ��݂���
	FillSolidRect(m_hdcMem, m_clrBG, &rect);

	if (m_bGradation && !m_bFillBar) {
		// Bar �p�� MemDC �ɃO���f�[�V������`���Ă���
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
		// �O���f�[�V�������Ȃ��Ƃ��́A�F��z��ɓ���Ă���
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
	return TRUE;		// ���������ɁA�����������Ƃɂ���B�@OS ���ŏ��������Ƃ�����̂ŁB
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


	// �ꉞ�`�F�b�N
	if (m_hdcMem == NULL)
		return TRUE;

	// ���ɃX�N���[��
	if (m_nBitBltWidth)
		BitBlt(m_hdcMem, 0, 0, m_nBitBltWidth, m_nWindowHeight, m_hdcMem, m_nBarWidth, 0, SRCCOPY);

	// �f�[�^�̎擾
	int nBar, nLine, nDummy;
	BOOL bDraw = GetData(nBar, nDummy, m_bPlotLineGraph, nLine);

	if (bDraw) {
		// �ォ��`��
		RECT rect;
		rect.left   = m_nBitBltWidth;
		rect.right  = m_nBitBltWidth + m_nBarWidth;
		rect.top    = 0;

		if (m_bFillBar) {
			// �h��Ԃ�
			rect.bottom = m_nWindowHeight;
			FillSolidRect(m_hdcMem, m_pclrBar[nBar], &rect);
		}
		else {
			// ���ʂɖ_�O���t��`��
			rect.bottom = m_nWindowHeight - (m_nWindowHeight * nBar) / 100;
			FillSolidRect(m_hdcMem, m_clrBG, &rect);

			rect.top    = rect.bottom;
			rect.bottom = m_nWindowHeight;
			if (m_bGradation) {
				// �O���f�[�V����������]������
				BitBlt(m_hdcMem, m_nBitBltWidth, rect.top, m_nBarWidth + 1, m_nWindowHeight - rect.top, m_hdcMemBar, 0, rect.top, SRCCOPY);
			}
			else {
				// �F�����߂āA�h��Ԃ�
				FillSolidRect(m_hdcMem, m_pclrBar[nBar], &rect);
			}
		}

		// ���O���t
		if (m_bPlotLineGraph) {
			DrawLineGraph(nLine, rect.left, rect.right);
		}

		InvalidateRect(hWnd, NULL, TRUE);
	}
	return TRUE;
}
