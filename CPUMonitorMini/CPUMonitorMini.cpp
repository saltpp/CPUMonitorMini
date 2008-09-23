/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: CPUMonitorMini.cpp 137 2008-09-23 11:57:05Z Salt $
 * @brief	CPUMonitorMini の main
 *			WinMain() で、ini ファイル読んで、dialog class のインスタンスを作って、ウィンドウを登録するところまで。
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 *
 * クラス構造
 *
 * CBase <-- CPopupDialog <-- CGraphDialog <-- CGraphDialogCPU
 *                                         <-- CGraphTRDialog  <-- CGraphTRDialogRAS
 *                                                             <-- CGraphTRDialogNDIS
 *
 * CBase              : base クラス。インスタンス ID と、インスタンスのバイナリダンプ用。今回は、特に必要が無いので、消した。
 * CPopupDialog       : Dialog Window を Popup で作る（このクラスは汎用的に使える）
 * CGraphDialog       : 棒グラフと線グラフを描くクラス。MemDC はここで用意する。（これ以降のクラスはグラフ描画用）
 * CGraphDialogCPU    : CPU と Memory 情報を返すクラス、描画は CGraphDialog で行う。
 * CGraphTRDialog     : 送信・受信をまとめて１つの棒グラフで描くクラス、線グラフは CGraph Dialog と同じ。
 * CGraphTRDialogRAS  : RAS 関係（PHS, USB）の情報を返すクラス、描画は CGraphTRDialog で行う。
 * CGraphTRDialogNDIS : NDIS 関係（WLAN）の情報を返すクラス、描画は CGraphTRDialog で行う。
 *
 *----------------------------------------------------------------------------
 *
 * CGraphDialogCPU, CGraphTRDialogRAS, CGraphTRDialogNDIS の３つのインスタンスでウィンドウを管理する。
 * コールバックするのは、CPopupDialog のポインタから行う。ポリモーフィズムで、継承先のメソッドが呼ばれる。
 * 結構綺麗に書けてるよね？　CGraphDialogCPU, CGraphTRDialogRAS, CGraphTRDialogNDIS は、データ取得部分の本質的なところだけになってるし。
 *
 *----------------------------------------------------------------------------
 */



// ToDo
//   establish したら、一旦全部塗りつぶしてクリアするか？
//   レジストリ文字列は ini に出す、電波レベルの６段階というのも出す
//   exit... の ... はおかしいな。Windows 的には dialog 出すのか？と思ってしまう。
//   終了するときは OK/Cancel を選べるようにするか
//   RAS/NDIS ウィンドウを常に出せるようにする


#include "stdafx.h"

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>

#include "CPUMonitorMini.h"
#include "GraphDialogCPU.h"
#include "GraphTRDialogRAS.h"
#include "GraphTRDialogNDIS.h"
#include "IniFile.h"



//============================================================================
// local variables
CGraphDialogCPU    *l_pdlgCPU  = NULL;
CGraphTRDialogRAS  *l_pdlgRAS  = NULL;
CGraphTRDialogNDIS *l_pdlgNDIS = NULL;

CPopupDialog **l_ppdlgHandleMap[4] = { NULL, };			// 親クラスのポインタでインスタンスにアクセスする。　現状、3ウィンドウなので、NULL 分で +1 で 4。　ポインタを入れるだけでもいいけど、安全の為にポインタのポインタを入れて、中をチェックしてから使うことにする。
CPopupDialog ***l_pppdlgHandleMap = l_ppdlgHandleMap;	// これで、↑の配列に入れていく

LONG g_lWndProcORG = NULL;								// 文字種ウィンドウが消える問題対策用。subclass 化して、画面回転を検出する。



//============================================================================
// user settings                        = default value (ini が無い場合の値)
// プリフィックス(l_)は前置しないことにする。

LOCAL BOOL FixATOKProblem               = 0;

// CPU/Memory
LOCAL int WindowLeft                    = 396;
LOCAL int WindowTop                     = 30;
LOCAL int WindowLeftHorizontal          = 714;
LOCAL int WindowTopHorizontal           = 30;
LOCAL int WindowWidth                   = 32;
LOCAL int WindowHeight                  = 18;
LOCAL int BarWidth                      = 1;
LOCAL int TimerInterval                 = 2000;
LOCAL COLORREF ColorBG                  = RGB(0, 0, 0);
LOCAL COLORREF ColorBar0                = RGB(0x00, 0xff, 0x00);
LOCAL COLORREF ColorBar50               = RGB(0xff, 0xff, 0x00);
LOCAL COLORREF ColorBar100              = RGB(0xff, 0x00, 0x00);
LOCAL BOOL Gradation                    = TRUE;
LOCAL BOOL FillBar                      = FALSE;
LOCAL BOOL ShowPercentage               = FALSE;

LOCAL BOOL PlotFreeMemory               = TRUE;
LOCAL COLORREF ColorFreeMemory          = RGB(0xff, 0xff, 0xff);
LOCAL int FreeMemoryMax                 = 75;
LOCAL int FreeMemoryMin                 = 25;


// RAS
LOCAL BOOL RAS_Enable                   = TRUE;
LOCAL int RAS_WindowLeft                = 394;
LOCAL int RAS_WindowTop                 = 30;
LOCAL int RAS_WindowLeftHorizontal      = 714;
LOCAL int RAS_WindowTopHorizontal       = 30;
LOCAL int RAS_WindowWidth               = 16;
LOCAL int RAS_WindowHeight              = 18;
LOCAL int RAS_BarWidth                  = 1;
LOCAL int RAS_TimerInterval             = 2000;
LOCAL COLORREF RAS_ColorBG              = RGB(0x00, 0x00, 0x60);
LOCAL COLORREF RAS_ColorBarReceived     = RGB(0x00, 0xff, 0xff);
LOCAL COLORREF RAS_ColorBarTransmitted  = RGB(0xff, 0x00, 0x00);
LOCAL int RAS_ReceivedMax               = 64;
LOCAL int RAS_TransmittedMax            = 32;

LOCAL BOOL RAS_PlotRFStrength           = TRUE;
LOCAL COLORREF RAS_ColorRFStrength      = RGB(0xff, 0xff, 0xff);

LOCAL int RAS_AutoDisconnect            = 0;	// in second. 0 means not disconnect.


// NDIS
LOCAL BOOL NDIS_Enable                  = TRUE;
LOCAL int NDIS_WindowLeft               = 394;
LOCAL int NDIS_WindowTop                = 30;
LOCAL int NDIS_WindowLeftHorizontal     = 714;
LOCAL int NDIS_WindowTopHorizontal      = 30;
LOCAL int NDIS_WindowWidth              = 16;
LOCAL int NDIS_WindowHeight             = 18;
LOCAL int NDIS_BarWidth                 = 1;
LOCAL int NDIS_TimerInterval            = 2000;
LOCAL COLORREF NDIS_ColorBG             = RGB(0x00, 0x00, 0x60);
LOCAL COLORREF NDIS_ColorBarReceived    = RGB(0x00, 0xff, 0xff);
LOCAL COLORREF NDIS_ColorBarTransmitted = RGB(0xff, 0x00, 0x00);
LOCAL int NDIS_ReceivedMax              = 64;
LOCAL int NDIS_TransmittedMax           = 32;

LOCAL BOOL NDIS_PlotRFStrength          = TRUE;
LOCAL COLORREF NDIS_ColorRFStrength     = RGB(0xff, 0xff, 0xff);

LOCAL int NDIS_AutoDisconnect           = 0;	// in second. 0 means not disconnect.


// ini のキー文字列と変数名をテーブルにしておく
LOCAL CIniFile::INI_KEY l_iniKeyTable[] = {

	// Global settings
	{_T("FixATOKProblem"),            &FixATOKProblem,                   10, },


	// CPU Window
	{_T("WindowLeft"),                &WindowLeft,                       10, },
	{_T("WindowTop"),                 &WindowTop,                        10, },
	{_T("WindowLeftHorizontal"),      &WindowLeftHorizontal,             10, },
	{_T("WindowTopHorizontal"),       &WindowTopHorizontal,              10, },
	{_T("WindowWidth"),               &WindowWidth,                      10, },
	{_T("WindowHeight"),              &WindowHeight,                     10, },
	{_T("BarWidth"),                  &BarWidth,                         10, },
	{_T("TimerInterval"),             &TimerInterval,                    10, },
	{_T("ColorBG"),                   (int *) &ColorBG,                  16, },
	{_T("ColorBar0"),                 (int *) &ColorBar0,                16, },
	{_T("ColorBar50"),                (int *) &ColorBar50,               16, },
	{_T("ColorBar100"),               (int *) &ColorBar100,              16, },
	{_T("Gradation"),                 &Gradation,                        10, },
	{_T("FillBar"),                   &FillBar,                          10, },
	{_T("ShowPercentage"),            &ShowPercentage,                   10, },

	{_T("PlotFreeMemory"),            &PlotFreeMemory,                   10, },
	{_T("ColorFreeMemory"),           (int *) &ColorFreeMemory,          16, },
	{_T("FreeMemoryMin"),             &FreeMemoryMin,                    10, },
	{_T("FreeMemoryMax"),             &FreeMemoryMax,                    10, },


	// RAS Window
	{_T("RAS_Enable"),                &RAS_Enable,                       10, },
	{_T("RAS_WindowLeft"),            &RAS_WindowLeft,                   10, },
	{_T("RAS_WindowTop"),             &RAS_WindowTop,                    10, },
	{_T("RAS_WindowLeftHorizontal"),  &RAS_WindowLeftHorizontal,         10, },
	{_T("RAS_WindowTopHorizontal"),   &RAS_WindowTopHorizontal,          10, },
	{_T("RAS_WindowWidth"),           &RAS_WindowWidth,                  10, },
	{_T("RAS_WindowHeight"),          &RAS_WindowHeight,                 10, },
	{_T("RAS_BarWidth"),              &RAS_BarWidth,                     10, },
	{_T("RAS_TimerInterval"),         &RAS_TimerInterval,                10, },
	{_T("RAS_ColorBG"),               (int *) &RAS_ColorBG,              16, },
	{_T("RAS_ColorBarReceived"),      (int *) &RAS_ColorBarReceived,     16, },
	{_T("RAS_ColorBarTransmitted"),   (int *) &RAS_ColorBarTransmitted,  16, },
	{_T("RAS_ReceivedMax"),           &RAS_ReceivedMax,                  10, },
	{_T("RAS_TransmittedMax"),        &RAS_TransmittedMax,               10, },

	{_T("RAS_PlotRFStrength"),        &RAS_PlotRFStrength,               10, },
	{_T("RAS_ColorRFStrength"),       (int *) &RAS_ColorRFStrength,      16, },

	{_T("RAS_AutoDisconnect"),        &RAS_AutoDisconnect,               10, },

	// NDIS Window
	{_T("NDIS_Enable"),               &NDIS_Enable,                      10, },
	{_T("NDIS_WindowLeft"),           &NDIS_WindowLeft,                  10, },
	{_T("NDIS_WindowTop"),            &NDIS_WindowTop,                   10, },
	{_T("NDIS_WindowLeftHorizontal"), &NDIS_WindowLeftHorizontal,        10, },
	{_T("NDIS_WindowTopHorizontal"),  &NDIS_WindowTopHorizontal,         10, },
	{_T("NDIS_WindowWidth"),          &NDIS_WindowWidth,                 10, },
	{_T("NDIS_WindowHeight"),         &NDIS_WindowHeight,                10, },
	{_T("NDIS_BarWidth"),             &NDIS_BarWidth,                    10, },
	{_T("NDIS_TimerInterval"),        &NDIS_TimerInterval,               10, },
	{_T("NDIS_ColorBG"),              (int *) &NDIS_ColorBG,             16, },
	{_T("NDIS_ColorBarReceived"),     (int *) &NDIS_ColorBarReceived,    16, },
	{_T("NDIS_ColorBarTransmitted"),  (int *) &NDIS_ColorBarTransmitted, 16, },
	{_T("NDIS_ReceivedMax"),          &NDIS_ReceivedMax,                 10, },
	{_T("NDIS_TransmittedMax"),       &NDIS_TransmittedMax,              10, },

	{_T("NDIS_PlotRFStrength"),       &NDIS_PlotRFStrength,              10, },
	{_T("NDIS_ColorRFStrength"),      (int *) &NDIS_ColorRFStrength,     16, },

	{_T("NDIS_AutoDisconnect"),       &NDIS_AutoDisconnect,              10, },



	{NULL,                            NULL,                              0,  },
};


BOOL ReadIniFile(HINSTANCE hInstance)
{
	// ini ファイルの名前を作る
	TCHAR szBuf[MAX_PATH];
	GetModuleFileName(hInstance, szBuf, ARRAY_SIZE_OF(szBuf));

	TCHAR *p = _tcsrchr(szBuf, _T('.'));	// _wsplitpath_s() 使えよ。
	if (p) {
		*p= _T('\0');
		_tcscat_s(szBuf, ARRAY_SIZE_OF(szBuf), _T(".ini"));

		// ini を開く
		CIniFile ini;
		ini.SetFileName(szBuf);
		ini.SetKeyTable(l_iniKeyTable);
		return ini.Read();
	}

	return FALSE;
}


// 全てのウィンドウ生成時にこれを登録して、これがコールバックされたら、hwnd で振り分けてそれぞれのクラスのインスタンスを呼び分ける
// MFC も同じようにやってるよね？
INT_PTR CALLBACK DialogProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{

	// WM_INITDIALOG だけは特別処理。CreateDialogIndirect() を抜ける前に、これがコールバックされてしまうので。
	// MFC も特別処理してるっぽい。
	if (uiMsg == WM_INITDIALOG) {
		CPopupDialog ***p = l_ppdlgHandleMap;
		while (*p && **p) {
			if ((**p)->GetHWND() == (HWND) CPopupDialog::HWND_UNIQUE_ID_BEFORE_CREATING) {		// 先に見つかったものが、正しいインスタンスのはず。毎回必ずインスタンスを探して必ず見つかるはずなので。
				return (**p)->DialogProc(hWnd, uiMsg, wParam, lParam);
			}
			++p;
		}
	}
	else {
		// hwnd で振り分けて、それぞれのインスタンスの DialogPorc() を呼ぶ。CPopupDialog のポインタ経由で呼ぶが、DialogProc() の中で呼ばれるメソッドが virtual なので最終的には継承先のメソッドが呼ばれる。
		CPopupDialog ***p = l_ppdlgHandleMap;
		while (*p && **p) {
			if ((**p)->GetHWND() == hWnd) {
				return (**p)->DialogProc(hWnd, uiMsg, wParam, lParam);
			}
			++p;
		}
	}

	return NULL;
}


LRESULT CALLBACK WndProcSubclass(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	if (uiMsg == WM_SETTINGCHANGE) {
		if (wParam == SETTINGCHANGE_RESET) {
			CPopupDialog ***p = l_ppdlgHandleMap;
			while (*p && **p) {
				(**p)->DialogProc((**p)->GetHWND(), uiMsg, wParam, lParam);
				++p;
			}
		}
	}

	return CallWindowProc((WNDPROC) g_lWndProcORG, hWnd, uiMsg, wParam, lParam);
}


HWND FindWindowEx(HWND hwndTop, TCHAR *szClassName, TCHAR *szCaptionName)
{
	HWND hwndResult = NULL;

	HWND hwnd = hwndTop;
	while (hwnd) {
		TCHAR szBuf[0x100];		/// @TODO マジックナンバーを消せ
		GetClassName(hwnd, szBuf, ARRAY_SIZE_OF(szBuf));
		if (_tcsncmp(szBuf, szClassName, ARRAY_SIZE_OF(szBuf)) == 0) {
			GetWindowText(hwnd, szBuf, ARRAY_SIZE_OF(szBuf));
			if (_tcsncmp(szBuf, szCaptionName, ARRAY_SIZE_OF(szBuf)) == 0) {
				hwndResult = hwnd;
				break;
			}
		}

		HWND hwndChild = GetWindow(hwnd, GW_CHILD);
		if (hwndChild) {
			hwndResult = FindWindowEx(hwndChild, szClassName, szCaptionName);
			if (hwndResult)
				break;
		}
		
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	
	return hwndResult;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) 
{

	// TODO: Place code here.

    HANDLE hMutex = CreateMutex(NULL /*MUTEX_ALL_ACCESS*/, FALSE, PROGRAM_NAME _T("_Mutex"));
	if (::GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, _T("Exit...\n\n") PROGRAM_NAME _T(" ") PROGRAM_VERSION _T("\n") PROGRAM_RIGHT, PROGRAM_NAME _T(" ") PROGRAM_VERSION, MB_OK);
		HWND hWndTop = GetWindow(NULL, GW_CHILD);		// GetDesktopWindow(); だと別のを取ってくる。
		HWND hDlg = FindWindowEx(hWndTop, _T("Dialog"), PROGRAM_NAME);
		if (hDlg)
			PostMessage(hDlg, WM_CLOSE, 0, 0);			// 親ウィンドウに送れば、子ウィンドウも WM_DESTROY される		
    }
	else {
		HWND hWndParent = FindWindow(PARENT_WINDOW_CLASS_NAME, NULL);			// "HHTaskBar"
		if (hWndParent == NULL) {
			hWndParent = FindWindow(PARENT_WINDOW_CLASS_NAME_WM61, NULL);		// "Tray" for WM6.1
		}
		if (hWndParent == NULL) {
			MessageBox(NULL, _T("Couldn't find ") PARENT_WINDOW_CLASS_NAME _T(" or ") PARENT_WINDOW_CLASS_NAME_WM61 _T(", so exit."), PROGRAM_NAME _T(" ") PROGRAM_VERSION, MB_OK);
			return 0;
		}

		// ini ファイルの読み込み
		ReadIniFile(hInstance);


		// 画面回転を検出するために subclass 化する
		HWND hWndParentSubclass = hWndParent;
		if (FixATOKProblem) {
			g_lWndProcORG = GetWindowLong(hWndParentSubclass, GWL_WNDPROC);
			if (SetWindowLong(hWndParentSubclass, GWL_WNDPROC, (LONG) WndProcSubclass) == 0) {
				g_lWndProcORG = NULL;
			}
		}


		// CPU/Memory ウィンドウのインスタンスの生成
		l_pdlgCPU  = new CGraphDialogCPU();
		*l_pppdlgHandleMap++ = (CPopupDialog **) &l_pdlgCPU;
		// パラメータ設定
		l_pdlgCPU->SetWindowPos(WindowLeft, WindowTop, WindowWidth, WindowHeight, WindowLeftHorizontal, WindowTopHorizontal);
		l_pdlgCPU->SetTimerID(TIMER_ID_CPU);
		l_pdlgCPU->SetTimerInterval(TimerInterval);
		l_pdlgCPU->SetBarWidth(BarWidth);
		l_pdlgCPU->SetColorBG(ColorBG);
		l_pdlgCPU->SetColorBar(ColorBar0, ColorBar50, ColorBar100);
		l_pdlgCPU->SetGradation(Gradation);
		l_pdlgCPU->SetFillBar(FillBar);
		l_pdlgCPU->SetShowPercentage(ShowPercentage);
		l_pdlgCPU->SetPlotLineGraph(PlotFreeMemory);
		l_pdlgCPU->SetColorLineGraph(ColorFreeMemory);
		l_pdlgCPU->SetMinFreeMemory(FreeMemoryMin);
		l_pdlgCPU->SetMaxFreeMemory(FreeMemoryMax);
		// 値のチェック＆修正
		l_pdlgCPU->NormalizeUserSettings();
		// Window の作成
		hWndParent = l_pdlgCPU->Create(hInstance, hWndParent, PROGRAM_NAME, DialogProc, FixATOKProblem);
		l_pdlgCPU->ShowWindow(TRUE);

		
		if (RAS_Enable) {
			// RAS ウィンドウのインスタンスの生成
			l_pdlgRAS  = new CGraphTRDialogRAS();
			*l_pppdlgHandleMap++ = (CPopupDialog **) &l_pdlgRAS;
			// パラメータ設定
			l_pdlgRAS->SetWindowPos(RAS_WindowLeft, RAS_WindowTop, RAS_WindowWidth, RAS_WindowHeight, RAS_WindowLeftHorizontal, RAS_WindowTopHorizontal);
			l_pdlgRAS->SetTimerID(TIMER_ID_RAS);
			l_pdlgRAS->SetTimerInterval(RAS_TimerInterval);
			l_pdlgRAS->SetBarWidth(RAS_BarWidth);
			l_pdlgRAS->SetColorBG(RAS_ColorBG);
			l_pdlgRAS->SetColorBar(RAS_ColorBarTransmitted, RAS_ColorBarReceived, RGB(0x00, 0x00, 0x00));
			l_pdlgRAS->SetPlotLineGraph(RAS_PlotRFStrength);
			l_pdlgRAS->SetColorLineGraph(RAS_ColorRFStrength);
			l_pdlgRAS->SetMaxTransmitted(RAS_TransmittedMax);
			l_pdlgRAS->SetMaxReceived(RAS_ReceivedMax);
			l_pdlgRAS->SetAutoDisconnectElapse(RAS_AutoDisconnect);
			// 値のチェック＆修正
			l_pdlgRAS->NormalizeUserSettings();
			// Window の作成
			hWndParent = l_pdlgRAS->Create(hInstance, hWndParent, PROGRAM_NAME _T("_RAS"), DialogProc, FALSE);		// FixATOKProblem);		// 子にすると CPU Window から出れなくなる
		}

		
		if (NDIS_Enable) {
			// NDIS ウィンドウのインスタンスの生成
			l_pdlgNDIS = new CGraphTRDialogNDIS();
			*l_pppdlgHandleMap++ = (CPopupDialog **) &l_pdlgNDIS;
			// パラメータ設定
			l_pdlgNDIS->SetWindowPos(NDIS_WindowLeft, NDIS_WindowTop, NDIS_WindowWidth, NDIS_WindowHeight, NDIS_WindowLeftHorizontal, NDIS_WindowTopHorizontal);
			l_pdlgNDIS->SetTimerID(TIMER_ID_NDIS);
			l_pdlgNDIS->SetTimerInterval(NDIS_TimerInterval);
			l_pdlgNDIS->SetBarWidth(NDIS_BarWidth);
			l_pdlgNDIS->SetColorBG(NDIS_ColorBG);
			l_pdlgNDIS->SetColorBar(NDIS_ColorBarTransmitted, NDIS_ColorBarReceived, RGB(0x00, 0x00, 0x00));
			l_pdlgNDIS->SetPlotLineGraph(NDIS_PlotRFStrength);
			l_pdlgNDIS->SetColorLineGraph(NDIS_ColorRFStrength);
			l_pdlgNDIS->SetMaxTransmitted(NDIS_TransmittedMax);
			l_pdlgNDIS->SetMaxReceived(NDIS_ReceivedMax);
			l_pdlgNDIS->SetAutoDisconnectElapse(NDIS_AutoDisconnect);
			// 値のチェック＆修正
			l_pdlgNDIS->NormalizeUserSettings();
			// Window の作成
			hWndParent = l_pdlgNDIS->Create(hInstance, hWndParent, PROGRAM_NAME _T("_NDIS"), DialogProc, FALSE);		// FixATOKProblem);		// 子にすると CPU Window から出れなくなる
		}



		// あとは、message loop の DispatchMessage() の中で、OS 側から、DialogProc() が callback される
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			// TranslateMessage(&msg);	// WM_CHAR, WM_COMMAND とかハンドリングしてないので、特に必要無し。
			DispatchMessage(&msg);
		}


		if (g_lWndProcORG) {	// if (l_bFixATOKProblem) {
			// subclass 化した後なら、解除しておく
			SetWindowLong(hWndParentSubclass, GWL_WNDPROC, g_lWndProcORG);
			g_lWndProcORG = NULL;
		}

		// Mutex の開放
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);

		// ウィンドウインスタンスの削除
		if (l_pdlgCPU) {
			delete l_pdlgCPU;
			l_pdlgCPU = NULL;
		}

		if (l_pdlgRAS) {
			delete l_pdlgRAS;
			l_pdlgRAS = NULL;
		}

		if (l_pdlgNDIS) {
			delete l_pdlgNDIS;
			l_pdlgNDIS = NULL;
		}
	}
	return 0; 
} 
