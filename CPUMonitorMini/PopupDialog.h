/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: PopupDialog.h 127 2008-05-12 15:06:14Z Shiono $
 * @brief	popup daialog を作って、表示するところまでを行うクラス
 *			ウィンドウのサイズとか位置とかはここで扱う
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#pragma once

#include <windows.h>

class CPopupDialog {

public:
	static const DWORD HWND_UNIQUE_ID_BEFORE_CREATING = 0xFFFFFFFF;		// HWND 型では初期化できないや


private:
	void MoveWindowByScreenDirection(HWND hwnd);

	HWND m_hWndParent;
	BOOL m_bChild;

protected:
	HWND m_hWnd; 

	// Window location and size
	int m_nWindowWidth;
	int m_nWindowHeight;
	int m_nWindowTop;
	int m_nWindowLeft;
	int m_nWindowTopHorizontal;
	int m_nWindowLeftHorizontal;

	void NormalizeUserSettings(void);


	// message handlers
	virtual BOOL OnInitDialog(HWND hWnd);
	virtual BOOL OnDestroy(HWND hWnd)                   { return FALSE; }
	virtual BOOL OnPaint(HWND hWnd)                     { return FALSE; }
	virtual BOOL OnEraseBkgnd(HDC hDC)                  { return FALSE; }
	virtual BOOL OnTimer(HWND hWnd, int nIDEvent)       { return FALSE; }
	virtual BOOL OnLButtonDown(HWND hWnd, POINT &point) { return FALSE; }


public:
	CPopupDialog(void);
	virtual ~CPopupDialog(void);

	HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR *pszCaption, INT_PTR (CALLBACK *pDialogProc)(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam), BOOL bChild);
	HWND GetHWND(void) { return m_hWnd; }
	INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void ShowWindow(BOOL b) { ::ShowWindow(m_hWnd, b ? SW_SHOW : SW_HIDE); }

	// for user settings
	void SetWindowPos(int nLeft, int nTop, int nWidth, int nHeight, int nLeftHorizontal, int nTopHorizontal) {
		m_nWindowLeft           = nLeft;
		m_nWindowTop            = nTop;
		m_nWindowLeftHorizontal = nLeftHorizontal;
		m_nWindowTopHorizontal  = nTopHorizontal;
		m_nWindowWidth          = nWidth;
		m_nWindowHeight         = nHeight;
	}

};
