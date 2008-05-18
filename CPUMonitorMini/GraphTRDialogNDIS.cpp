/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphTRDialogNDIS.cpp 117 2008-05-11 11:21:40Z Shiono $
 * @brief	CGraphTRDialog を継承した NDIS 情報を取得するクラス
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#include "StdAfx.h"
#include "GraphTRDialogNDIS.h"
#include "CPUMonitorMini.h"

#include <winioctl.h>
#include <ntddndis.h>
#include <nuiouser.h>


#define WLAN_DEVICE_NAME_LENGTH     255
#define QUERY_BUFFER_SIZE           1024

// static 変数を初期化
int CGraphTRDialogNDIS::m_nNDISCount = 0;


CGraphTRDialogNDIS::CGraphTRDialogNDIS(void)
: m_hNDIS(INVALID_HANDLE_VALUE)
, m_bDisabledPrev(TRUE)
{
	*m_szWlanDeviceName = _T('\0');		// 配列はコンストラクタが無くて、コンストラクタ初期化子に書けないので、ここで初期化
}


CGraphTRDialogNDIS::~CGraphTRDialogNDIS(void)
{
}


void CGraphTRDialogNDIS::NormalizeUserSettings(void)
{
	m_nMaxTransmitted = min(max(1, m_nMaxTransmitted), 100000);
	m_nMaxReceived    = min(max(1, m_nMaxReceived),    100000);

	__super::NormalizeUserSettings();
}


void CGraphTRDialogNDIS::RegistryNotifyCallbackFuncNetworkCount(HREGNOTIFY hNotify, DWORD dwUserData, const PBYTE pData, const UINT cbData)
{
	m_nNDISCount = *((DWORD *) pData);
}


BOOL CGraphTRDialogNDIS::OnInitDialog(HWND hWnd)
{

	// Registory アクセスされたら notify されるように登録
	DWORD dwTemp;

	// Network
	RegistryNotifyCallback(HKEY_LOCAL_MACHINE,
							_T("System\\State\\Connections\\Network"),
							_T("Count"),
							RegistryNotifyCallbackFuncNetworkCount,
							0,
							NULL,
							&m_hrnNetworkCount);				// handle
	// 初期値を読み込んでおく
	RegistryGetDWORD(HKEY_CURRENT_USER,
						_T("System\\State\\Connections\\Network"),
						_T("Count"),
						&dwTemp);
	m_nNDISCount = (int) dwTemp;

	return __super::OnInitDialog(hWnd);
}


BOOL CGraphTRDialogNDIS::OnDestroy(HWND hWnd)
{
	// RFStrength
	if (m_hrnNetworkCount) {
		RegistryCloseNotification(m_hrnNetworkCount);
		m_hrnNetworkCount = NULL;
	}

	return __super::OnDestroy(hWnd);
}


// 無線LANデバイスを検索する
BOOL CGraphTRDialogNDIS::SearchWlanDevice()
{

	// *m_szWlanDeviceName = _T('\0');

	BOOL bFound = FALSE;

    // デバイス名列挙
    TCHAR szQueryBuffer[QUERY_BUFFER_SIZE] = {0};										// buffer 確保
    PNDISUIO_QUERY_BINDING pNdisQueryBinding = (PNDISUIO_QUERY_BINDING) szQueryBuffer;	// buffer 割り当て
	pNdisQueryBinding->BindingIndex = 0;
    DWORD dwBytesReturned;
	while (DeviceIoControl(m_hNDIS, IOCTL_NDISUIO_QUERY_BINDING,
							(LPVOID) szQueryBuffer, sizeof(szQueryBuffer),
							(LPVOID) szQueryBuffer, sizeof(szQueryBuffer),
							&dwBytesReturned, NULL)) {

        // デバイス種別を取得
		TCHAR szQueryBufferDevice[QUERY_BUFFER_SIZE] = {0};								// buffer 確保
		PNDISUIO_QUERY_OID pNdisQueryOid = (PNDISUIO_QUERY_OID) szQueryBufferDevice;	// buffer 割り当て

		pNdisQueryOid->Oid           = OID_GEN_PHYSICAL_MEDIUM;
		pNdisQueryOid->ptcDeviceName = szQueryBuffer + pNdisQueryBinding->DeviceNameOffset;

		if (DeviceIoControl(m_hNDIS, IOCTL_NDISUIO_QUERY_OID_VALUE,
							(LPVOID) szQueryBufferDevice, sizeof(szQueryBufferDevice),
							(LPVOID) szQueryBufferDevice, sizeof(szQueryBufferDevice),
							&dwBytesReturned, NULL)) {

			if (*((NDIS_PHYSICAL_MEDIUM *) pNdisQueryOid->Data) == NdisPhysicalMediumWirelessLan ) {	// enum
				bFound = TRUE;

				// デバイス名を保存、以後、デバイス名を使ってアクセスする
                _tcscpy_s(m_szWlanDeviceName, ARRAY_SIZE_OF(m_szWlanDeviceName), pNdisQueryOid->ptcDeviceName);
				break;
            }
        }
        ++(pNdisQueryBinding->BindingIndex);
    }

    return bFound;

}


// Close
void CGraphTRDialogNDIS::CloseNdisUio()
{
	if (m_hNDIS != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hNDIS);
		m_hNDIS = INVALID_HANDLE_VALUE;
	}
}


// Open
BOOL CGraphTRDialogNDIS::OpenNdisUio()
{
    m_hNDIS = CreateFile(NDISUIO_DEVICE_NAME,
							GENERIC_ALL,
							0, NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
							(HANDLE)INVALID_HANDLE_VALUE);
    
	return (m_hNDIS != INVALID_HANDLE_VALUE);
}


BOOL CGraphTRDialogNDIS::GetData(int &nBarTransmitted, int &nBarReceived, BOOL bLine, int &nLine)
{
	if (m_nNDISCount) {		// レジストリの Count が１になったらチェックを始める

		BOOL bFound = FALSE;

		DWORD dwTick = GetTickCount();

		if (dwTick != m_dwTickPrev) {
			int nDT = dwTick - m_dwTickPrev;	// in msec
			m_dwTickPrev = dwTick;

			BOOL bResult = OpenNdisUio();			
			if (bResult) {

				if (*m_szWlanDeviceName == _T('\0'))	// デバイス名を取得してなければ探す
					bResult = SearchWlanDevice();

				if (bResult) {
					NIC_STATISTICS statistics = {0};
					DWORD dwBytesReturned = 0;

					statistics.Size = sizeof(statistics);
					statistics.ptcDeviceName = m_szWlanDeviceName;	// device name を入れる

					if (DeviceIoControl(m_hNDIS, 
										IOCTL_NDISUIO_NIC_STATISTICS, 
										0, 0, 
										&statistics, sizeof(statistics), 
										&dwBytesReturned, 
										NULL)) {

						if (statistics.MediaState == MEDIA_STATE_CONNECTED) {
							static ULONGLONG ullPacketsReceivedPrev, ullPacketsSentPrev;

							int n;
							// n = (int) (statistics.PacketsSent - ullPacketsSentPrev) * 1000 * 100 / (nDT * m_nMaxTransmitted);
							m_nLastTransmitted = (int) (statistics.PacketsSent - ullPacketsSentPrev) * 1000 / nDT;	// tap したときの情報表示用に、一旦保存
							n = m_nLastTransmitted * 100 / m_nMaxTransmitted;
							n = min(max(0, n), 100);
							nBarTransmitted = n;

							// n = (int) (statistics.PacketsReceived - ullPacketsReceivedPrev) * 10000 * 100 / (nDT * m_nMaxReceived);
							m_nLastReceived = (int) (statistics.PacketsReceived - ullPacketsReceivedPrev) * 1000 / nDT;	// tap したときの情報表示用に、一旦保存
							n = m_nLastReceived * 100 / m_nMaxReceived;
							n = min(max(0, n), 100);
							nBarReceived = n;

							ullPacketsReceivedPrev = statistics.PacketsReceived;
							ullPacketsSentPrev     = statistics.PacketsSent;

							// Line Graph
							nLine = statistics.LinkSpeed / 1100;	// LinkSpped is in 100bits/s. 10Mb/s = 100000, 11MB/s = 110000
							nLine = min(max(0, nLine), 100);

							// １回目は、差分が正しく取れてないので消す
							if (m_bDisabledPrev) {
								m_bDisabledPrev = FALSE;
								nBarTransmitted = 0;
								nBarReceived    = 0;
							}


							bFound = TRUE;

						}
					}

					CloseNdisUio();
				}

			}
		}

		if (bFound) {
			return TRUE;
		}
		else {
			m_bDisabledPrev = TRUE;
			*m_szWlanDeviceName = _T('\0');
			return FALSE;
		}
	}
	else
		return FALSE;

}


BOOL CGraphTRDialogNDIS::OnLButtonDown(HWND hWnd, POINT &point)
{
	TCHAR szBuf[0x100];		/// @TODO バッファサイズはどこかで #define しよう
	_stprintf(szBuf, _T("Transmitted : %d packets\n")
					 _T("Received    : %d packets\n"),
					 //_T("\n\n")
					 //_T("Do you want to disconnect NDIS connection?"),
					 m_nLastTransmitted,
					 m_nLastReceived);

	MessageBox(hWnd, szBuf, PROGRAM_NAME _T(" ") PROGRAM_VERSION, MB_OK);

	return TRUE;
}
