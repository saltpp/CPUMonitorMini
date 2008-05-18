/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: PopupDialog.cpp 127 2008-05-12 15:06:14Z Shiono $
 * @brief	popup daialog ������āA�\������Ƃ���܂ł��s���N���X
 *			�E�B���h�E�̃T�C�Y�Ƃ��ʒu�Ƃ��͂����ň���
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#include "StdAfx.h"
#include "PopupDialog.h"

#include <windowsx.h>	// for GET_X_LPARAM, GET_Y_LPARAM


CPopupDialog::CPopupDialog(void)
: m_hWndParent(NULL)
, m_bChild(FALSE)
{
}


CPopupDialog::~CPopupDialog(void)
{
}

/**
 * �p���������ŃI�[�o���C�h������A�Ō�ŁA__super::NormalizeUserSettings(); ���ĂԕK�v����B
 * �Ă΂Ȃ��ƁA�e�N���X���̃p�����[�^�����K������Ȃ��B
 */
void CPopupDialog::NormalizeUserSettings(void)
{
	// �T�C�Y�̐���
	int nMaxSize = GetSystemMetrics(SM_CXSCREEN);
	if (nMaxSize > GetSystemMetrics(SM_CYSCREEN))
		nMaxSize = GetSystemMetrics(SM_CYSCREEN);

	// �������̃T�C�Y�̐����`�܂łɐ���
	m_nWindowWidth  = min(max(1, m_nWindowWidth),  nMaxSize);
	m_nWindowHeight = min(max(1, m_nWindowHeight), nMaxSize);

	// ��ʓ��ɓ����
	if (GetSystemMetrics(SM_CXSCREEN) < GetSystemMetrics(SM_CYSCREEN)) {
		// �c
		if (m_nWindowLeft > GetSystemMetrics(SM_CXSCREEN) - m_nWindowWidth)
			m_nWindowLeft = GetSystemMetrics(SM_CXSCREEN) - m_nWindowWidth;

		if (m_nWindowTop > GetSystemMetrics(SM_CYSCREEN) - m_nWindowHeight)
			m_nWindowTop = GetSystemMetrics(SM_CYSCREEN) - m_nWindowHeight;

		if (m_nWindowLeftHorizontal > GetSystemMetrics(SM_CYSCREEN) - m_nWindowWidth)
			m_nWindowLeftHorizontal = GetSystemMetrics(SM_CYSCREEN) - m_nWindowWidth;

		if (m_nWindowTopHorizontal > GetSystemMetrics(SM_CXSCREEN) - m_nWindowHeight)
			m_nWindowTopHorizontal = GetSystemMetrics(SM_CXSCREEN) - m_nWindowHeight;
	}
	else {
		// ��
		if (m_nWindowLeft > GetSystemMetrics(SM_CYSCREEN) - m_nWindowWidth)
			m_nWindowLeft = GetSystemMetrics(SM_CYSCREEN) - m_nWindowWidth;

		if (m_nWindowTop > GetSystemMetrics(SM_CXSCREEN) - m_nWindowHeight)
			m_nWindowTop = GetSystemMetrics(SM_CXSCREEN) - m_nWindowHeight;

		if (m_nWindowLeftHorizontal > GetSystemMetrics(SM_CXSCREEN) - m_nWindowWidth)
			m_nWindowLeftHorizontal = GetSystemMetrics(SM_CXSCREEN) - m_nWindowWidth;

		if (m_nWindowTopHorizontal > GetSystemMetrics(SM_CYSCREEN) - m_nWindowHeight)
			m_nWindowTopHorizontal = GetSystemMetrics(SM_CYSCREEN) - m_nWindowHeight;
	}
}


void CPopupDialog::MoveWindowByScreenDirection(HWND hWnd)
{
	BOOL bPortrait = GetSystemMetrics(SM_CXSCREEN) < GetSystemMetrics(SM_CYSCREEN);	// portrait / landscape
	if (m_bChild) {
		RECT rect;
		GetWindowRect(m_hWndParent, &rect);
		if (bPortrait) {
			// �c
			MoveWindow(hWnd, m_nWindowLeft - rect.left, m_nWindowTop - rect.top, m_nWindowWidth, m_nWindowHeight, 0);
		}
		else {
			// ��
			MoveWindow(hWnd, m_nWindowLeftHorizontal - rect.left, m_nWindowTopHorizontal - rect.top, m_nWindowWidth, m_nWindowHeight, 0);
		}
	}
	else {
		if (bPortrait) {
			// �c
			MoveWindow(hWnd, m_nWindowLeft, m_nWindowTop, m_nWindowWidth, m_nWindowHeight, 0);
		}
		else {
			// ��
			MoveWindow(hWnd, m_nWindowLeftHorizontal, m_nWindowTopHorizontal, m_nWindowWidth, m_nWindowHeight, 0);
		}
	}


}


//----------------------------------------------------------------------------
/**
 * @retval	TRUE	���������B�I������� DefWindowProc() ���Ă΂Ȃ��B
 * @retval	FALSE	�������ĂȂ��B�I������� DefWindowProc() ���ĂԁB
 */
BOOL CPopupDialog::OnInitDialog(HWND hWnd)
{
	MoveWindowByScreenDirection(hWnd);

	return TRUE;
}


INT_PTR CALLBACK CPopupDialog::DialogProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg) {

	case WM_INITDIALOG:
		m_hWnd = hWnd;		// emulator ���ƁA�����œ���Ă����Ȃ��ƁAWM_TIMER ����ɗ��Ă��܂��B�{���́ACreateDialogIndirect() �̕Ԓl�� m_hWnd �ɓ����΂����͂������B
		if (OnInitDialog(hWnd))
			return 0;		// key focus �𓖂Ă������ 1 ��Ԃ�
		break;


	case WM_DESTROY:
		if (OnDestroy(hWnd))
			return 1;		// ����������A0 �ȊO��Ԃ�
		break;


	case WM_PAINT:
		if (OnPaint(hWnd))
			return 1;		// ����������A0 �ȊO��Ԃ�
		break;


	case WM_ERASEBKGND:
		if (OnEraseBkgnd((HDC) wParam))
			return 1;		// ����������A0 �ȊO��Ԃ��B0 ��Ԃ��� OS ���Ŕw�i���������A�������ŏ�������Ȃ� 0 �ȊO��Ԃ��B
		break;


	case WM_TIMER:
		if (OnTimer(hWnd, (int) wParam))
			return 1;		// ����������A0 �ȊO��Ԃ�
		break;


	case WM_SETTINGCHANGE:
		if (wParam == SETTINGCHANGE_RESET) {
			MoveWindowByScreenDirection(hWnd);
		}
		break;


	case WM_LBUTTONDOWN:
		// MAKEPOINTS �g���� POINTS �ɂȂ�BPOINT �ł͂Ȃ��̂Ŏg���Ȃ��B
		POINT point;
		point.x = GET_X_LPARAM(lParam);
		point.y = GET_Y_LPARAM(lParam);

		if (OnLButtonDown(hWnd, point))
			return 1;		// ����������A0 �ȊO��Ԃ�
		break;

	//case WM_CTLCOLORDLG:

	}

    return DefWindowProc(hWnd, uiMsg, wParam, lParam) ;
}

// �z�[�~������ http://geocities.yahoo.co.jp/gl/hou_ming_2/comment/20060523/1148398202 ���Q�l�ɂ����Ă������
HWND CPopupDialog::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR *pszCaption, INT_PTR (CALLBACK *pDialogProc)(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam), BOOL bChild)
{
	m_hWndParent = hWndParent;
	m_bChild = bChild;

	typedef struct{
		DLGTEMPLATE dt;
		DLGITEMTEMPLATE di;		// ������ DWORD ���E�ɒu���K�v����
	} DLG_TEMPLATE;

	DLG_TEMPLATE dlg = {
		bChild ? WS_CHILD : WS_POPUP,
		WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW,
		0,						// number of DLGITEMTEMPLATE
		0, 0, 0, 0,				// x, y, cx, cy

		0,						// style
		0,						// dwExtendedStyle
		0, 0, 0, 0,				// x, y, cx, cy
		0						// id
	};


	m_hWnd = (HWND) HWND_UNIQUE_ID_BEFORE_CREATING;		// DialogProc() ���ŁA���̃C���X�^���X����������悤�ɂ���B���� CreateDialogIndirect() �̒��ŁADialogPoroc() ���Ă΂��̂ŁB
	m_hWnd = CreateDialogIndirect(hInstance,
									(LPCDLGTEMPLATE) &dlg,
									hWndParent,
									pDialogProc);
	SetWindowTextW(m_hWnd, pszCaption);

	return m_hWnd;
}

