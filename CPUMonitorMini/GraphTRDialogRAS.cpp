/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphTRDialogRAS.cpp 131 2008-05-18 10:56:20Z Shiono $
 * @brief	CGraphTRDialog を継承した RAS 情報を取得するクラス
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#include "StdAfx.h"
#include "GraphTRDialogRAS.h"
#include "CPUMonitorMini.h"
#include <ras.h>
#include <regext.h>



// static 変数を初期化
int CGraphTRDialogRAS::m_nModemCount    = 0;
int CGraphTRDialogRAS::m_nCellularCount = 0;
DWORD CGraphTRDialogRAS::m_dwRFStrength = 0;


CGraphTRDialogRAS::CGraphTRDialogRAS(void)
: m_bDisabledPrev(TRUE)
, m_hrasconn(0)
, m_hrnModemCount(NULL)
, m_hrnCellularCount(NULL)
, m_hrnRFStrength(NULL)
{
}


CGraphTRDialogRAS::~CGraphTRDialogRAS(void)
{
}


void CGraphTRDialogRAS::RegistryNotifyCallbackFuncModemCount(HREGNOTIFY hNotify, DWORD dwUserData, const PBYTE pData, const UINT cbData)
{
	m_nModemCount = *((DWORD *) pData);
}

void CGraphTRDialogRAS::RegistryNotifyCallbackFuncCellularCount(HREGNOTIFY hNotify, DWORD dwUserData, const PBYTE pData, const UINT cbData)
{
	m_nCellularCount = *((DWORD *) pData);
}


void CGraphTRDialogRAS::RegistryNotifyCallbackFuncRFStrength(HREGNOTIFY hNotify, DWORD dwUserData, const PBYTE pData, const UINT cbData)
{
	m_dwRFStrength = *((DWORD *) pData) * 100 / 6;	// ades は 6 段階
}


BOOL CGraphTRDialogRAS::OnInitDialog(HWND hWnd)
{

	// Registory に変化があったら notify されるように登録
	DWORD dwTemp;

	// Modem\Count
	RegistryNotifyCallback(HKEY_LOCAL_MACHINE,
							_T("System\\State\\Connections\\Modem"),	// hTc-Z (X01HT もか？）は Cellular, ZERO3, EM-1, X01T は Modemo の Count が 1 になる
							_T("Count"),
							RegistryNotifyCallbackFuncModemCount,
							0,
							NULL,
							&m_hrnModemCount);				// handle
	// 初期値を読み込んでおく
	RegistryGetDWORD(HKEY_LOCAL_MACHINE,
						_T("System\\State\\Connections\\Modem"),
						_T("Count"),
						&dwTemp);
	m_nModemCount = (int) dwTemp;

	// Modem\Cellular
	RegistryNotifyCallback(HKEY_LOCAL_MACHINE,
							_T("System\\State\\Connections\\Cellular"),
							_T("Count"),
							RegistryNotifyCallbackFuncCellularCount,
							0,
							NULL,
							&m_hrnModemCount);				// handle
	// 初期値を読み込んでおく
	RegistryGetDWORD(HKEY_LOCAL_MACHINE,
						_T("System\\State\\Connections\\Cellular"),
						_T("Count"),
						&dwTemp);
	m_nCellularCount = (int) dwTemp;


	// RFStrength
	if (m_bPlotLineGraph) {
		HRESULT hResult = RegistryNotifyCallback(HKEY_CURRENT_USER,
													_T("Software\\Sharp\\ShPhoneLib"),
													_T("RFStrength"),
													RegistryNotifyCallbackFuncRFStrength,
													0,
													NULL,
													&m_hrnRFStrength);	// handle
		if (hResult == S_OK) {
			// 初期値を読み込んでおく
			RegistryGetDWORD(HKEY_CURRENT_USER,
								_T("Software\\Sharp\\ShPhoneLib"),
								_T("RFStrength"),
								&dwTemp);
			RegistryNotifyCallbackFuncRFStrength(NULL, 0L, (const PBYTE) &dwTemp, 0L);	// callback 関数を自分で呼んで設定
		}
		else {
			// たぶん ZERO3 じゃない
			m_bPlotLineGraph = FALSE;
		}
	}

	return __super::OnInitDialog(hWnd);
}


BOOL CGraphTRDialogRAS::OnDestroy(HWND hWnd)
{
	// RFStrength
	if (m_bPlotLineGraph) {
		if (m_hrnRFStrength) {
			RegistryCloseNotification(m_hrnRFStrength);
			m_hrnRFStrength = NULL;
		}
	}

	if (m_hrnCellularCount) {
		RegistryCloseNotification(m_hrnCellularCount);
		m_hrnCellularCount = NULL;
	}

	if (m_hrnModemCount) {
		RegistryCloseNotification(m_hrnModemCount);
		m_hrnModemCount = NULL;
	}

	return __super::OnDestroy(hWnd);
}


void CGraphTRDialogRAS::NormalizeUserSettings(void)
{
	m_nMaxTransmitted = min(max(1, m_nMaxTransmitted), 10000);
	m_nMaxReceived    = min(max(1, m_nMaxReceived),    10000);

	__super::NormalizeUserSettings();
}


BOOL CGraphTRDialogRAS::GetData(int &nBarTransmitted, int &nBarReceived, BOOL bLine, int &nLine)
{
	if (m_nModemCount + m_nCellularCount > 0) {

		BOOL bFound = FALSE;

		DWORD dwTick = GetTickCount();

		if (dwTick != m_dwTickPrev) {
			int nDT = dwTick - m_dwTickPrev;	// in msec
			m_dwTickPrev = dwTick;

			// RAS
			RASCONN rasconn[2];
			rasconn[0].dwSize = sizeof(RASCONN);	// 52
			DWORD dwCB = sizeof(rasconn);			// ここは全体分を指定
			DWORD dwConnections = 0;
			RasEnumConnections(rasconn, &dwCB, &dwConnections);

			for (int i = 0; i < (int) dwConnections; ++i) {
				if (_tcsncmp(rasconn[i].szEntryName, _T("`USB"), 4)) {	// registry から取るべきだな。 `USB は ZERO3 だけ？　HKCU/ControlPanel/Comm/Cnct="`USB"
					// "'USB" でなければ、WAN だろう
					m_hrasconn = rasconn[i].hrasconn;
					RAS_STATS rasstats;
					rasstats.dwSize = sizeof(RAS_STATS);
					RasGetLinkStatistics(m_hrasconn, 0, &rasstats);


					int n;
					// Byte 単位で返ってくるので ×8 で bit 単位
					// nDT は ms 単位なので、それで割るために、先に * 1000 しておく（nDT を 1000 で割って、double とかに入れてもいいが。ARM の double は遅そうなのでやめておく。）
					// Kbps 単位にしたいので、1000 で割る。（誤差を減らすために最後で割る。k=1000 じゃなくて K=1024 が一般的だと思ったが、bps 系は k, K 両方とも 1000 だよね？）
					// m_nMax で指定された値を 100% にしたいので 100 / m_nMax を掛ける

					// n = ((rasstats.dwBytesXmited - m_dwXmitedPrev) * 8 * 1000 / (nDT * m_nMaxTransmitted)) * 100 / 1000;	// 1024;		// zero divide 注意、Normalize で 0 以外になるはず   // Byte で返ってきて
					m_nLastTransmitted = (rasstats.dwBytesXmited - m_dwXmitedPrev) * 8 * 1000 / nDT;	// tap したときの情報表示用に、一旦保存
					n = m_nLastTransmitted * 100 / m_nMaxTransmitted / 1000;	// 1024;
					n = min(max(0, n), 100);
					nBarTransmitted = n;

					// n = ((rasstats.dwBytesRcved - m_dwRcvedPrev)   * 8 * 1000 / (nDT * m_nMaxReceived))    * 100 / 1000;	// 1024;		// 1000～3000 ぐらいしか出ないな
					m_nLastReceived = (rasstats.dwBytesRcved - m_dwRcvedPrev) * 8 * 1000 / nDT;			// tap したときの情報表示用に、一旦保存
					n = m_nLastReceived * 100 / m_nMaxReceived / 1000;	// 1024;
					n = min(max(0, n), 100);
					nBarReceived = n;

					m_dwRcvedPrev  = rasstats.dwBytesRcved;
					m_dwXmitedPrev = rasstats.dwBytesXmited;

					// １回目は、差分が正しく取れてないので消す
					if (m_bDisabledPrev) {
						m_bDisabledPrev = FALSE;
						nBarTransmitted = 0;
						nBarReceived    = 0;
					}

					// Line Graph
					// if (bLine)	// 要らないや。比較するだけ無駄だ。
						nLine = m_dwRFStrength;

					bFound = TRUE;
				}
			}

		}
		if (bFound) {
			return TRUE;
		}
		else {
			m_bDisabledPrev = TRUE;
			m_hrasconn = NULL;
			return FALSE;
		}

	}
	else
		return FALSE;
}


BOOL CGraphTRDialogRAS::OnLButtonDown(HWND hWnd, POINT &point)
{
	TCHAR szBuf[0x100];		/// @TODO バッファサイズはどこかで #define しよう
	_stprintf(szBuf, _T("Transmitted : %.1lfKbps\n")
					 _T("Received    : %.1lfKbps\n")
					 _T("\n\n")
					 _T("Do you want to disconnect RAS connection?"),
					 (double) m_nLastTransmitted / 1000,	// 1024,
					 (double) m_nLastReceived    / 1000);	// 1024);

	int nResult = MessageBox(hWnd, szBuf, PROGRAM_NAME _T(" ") PROGRAM_VERSION, MB_OKCANCEL);
	
	if (nResult == IDOK) {
		if (m_hrasconn) {
			RasHangUp(m_hrasconn);
		}
	}
	return TRUE;
}
