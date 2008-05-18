/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: PopupDialog.cpp 127 2008-05-12 15:06:14Z Shiono $
 * @brief	popup daialog を作って、表示するところまでを行うクラス
 *			ウィンドウのサイズとか位置とかはここで扱う
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
 * 継承した側でオーバライドしたら、最後で、__super::NormalizeUserSettings(); を呼ぶ必要あり。
 * 呼ばないと、親クラス側のパラメータが正規化されない。
 */
void CPopupDialog::NormalizeUserSettings(void)
{
	// サイズの制限
	int nMaxSize = GetSystemMetrics(SM_CXSCREEN);
	if (nMaxSize > GetSystemMetrics(SM_CYSCREEN))
		nMaxSize = GetSystemMetrics(SM_CYSCREEN);

	// 狭い方のサイズの正方形までに制限
	m_nWindowWidth  = min(max(1, m_nWindowWidth),  nMaxSize);
	m_nWindowHeight = min(max(1, m_nWindowHeight), nMaxSize);

	// 画面内に入れる
	if (GetSystemMetrics(SM_CXSCREEN) < GetSystemMetrics(SM_CYSCREEN)) {
		// 縦
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
		// 横
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
			// 縦
			MoveWindow(hWnd, m_nWindowLeft - rect.left, m_nWindowTop - rect.top, m_nWindowWidth, m_nWindowHeight, 0);
		}
		else {
			// 横
			MoveWindow(hWnd, m_nWindowLeftHorizontal - rect.left, m_nWindowTopHorizontal - rect.top, m_nWindowWidth, m_nWindowHeight, 0);
		}
	}
	else {
		if (bPortrait) {
			// 縦
			MoveWindow(hWnd, m_nWindowLeft, m_nWindowTop, m_nWindowWidth, m_nWindowHeight, 0);
		}
		else {
			// 横
			MoveWindow(hWnd, m_nWindowLeftHorizontal, m_nWindowTopHorizontal, m_nWindowWidth, m_nWindowHeight, 0);
		}
	}


}


//----------------------------------------------------------------------------
/**
 * @retval	TRUE	処理した。終わった後 DefWindowProc() を呼ばない。
 * @retval	FALSE	処理してない。終わった後 DefWindowProc() を呼ぶ。
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
		m_hWnd = hWnd;		// emulator だと、ここで入れておかないと、WM_TIMER が先に来てしまう。本来は、CreateDialogIndirect() の返値を m_hWnd に入れればいいはずだが。
		if (OnInitDialog(hWnd))
			return 0;		// key focus を当てたければ 1 を返す
		break;


	case WM_DESTROY:
		if (OnDestroy(hWnd))
			return 1;		// 処理したら、0 以外を返す
		break;


	case WM_PAINT:
		if (OnPaint(hWnd))
			return 1;		// 処理したら、0 以外を返す
		break;


	case WM_ERASEBKGND:
		if (OnEraseBkgnd((HDC) wParam))
			return 1;		// 処理したら、0 以外を返す。0 を返すと OS 側で背景が消される、こっちで消した後なら 0 以外を返す。
		break;


	case WM_TIMER:
		if (OnTimer(hWnd, (int) wParam))
			return 1;		// 処理したら、0 以外を返す
		break;


	case WM_SETTINGCHANGE:
		if (wParam == SETTINGCHANGE_RESET) {
			MoveWindowByScreenDirection(hWnd);
		}
		break;


	case WM_LBUTTONDOWN:
		// MAKEPOINTS 使うと POINTS になる。POINT ではないので使えない。
		POINT point;
		point.x = GET_X_LPARAM(lParam);
		point.y = GET_Y_LPARAM(lParam);

		if (OnLButtonDown(hWnd, point))
			return 1;		// 処理したら、0 以外を返す
		break;

	//case WM_CTLCOLORDLG:

	}

    return DefWindowProc(hWnd, uiMsg, wParam, lParam) ;
}

// ホーミン氏の http://geocities.yahoo.co.jp/gl/hou_ming_2/comment/20060523/1148398202 を参考にさせてもらった
HWND CPopupDialog::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR *pszCaption, INT_PTR (CALLBACK *pDialogProc)(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam), BOOL bChild)
{
	m_hWndParent = hWndParent;
	m_bChild = bChild;

	typedef struct{
		DLGTEMPLATE dt;
		DLGITEMTEMPLATE di;		// ここは DWORD 境界に置く必要あり
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


	m_hWnd = (HWND) HWND_UNIQUE_ID_BEFORE_CREATING;		// DialogProc() 側で、このインスタンスを見つけられるようにする。次の CreateDialogIndirect() の中で、DialogPoroc() が呼ばれるので。
	m_hWnd = CreateDialogIndirect(hInstance,
									(LPCDLGTEMPLATE) &dlg,
									hWndParent,
									pDialogProc);
	SetWindowTextW(m_hWnd, pszCaption);

	return m_hWnd;
}

