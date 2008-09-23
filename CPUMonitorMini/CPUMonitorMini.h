/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: CPUMonitorMini.h 137 2008-09-23 11:57:05Z Salt $
 * @brief	CPUMonitorMini �� main
 *			WinMain() �ŁAini �t�@�C���ǂ�ŁAdialog class �̃C���X�^���X������āA�E�B���h�E��o�^����Ƃ���܂ŁB
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#pragma once
#include "resourceppc.h"

#define PROGRAM_NAME    _T("CPUMonitorMini")
#define PROGRAM_VERSION _T("Ver.0.50")
//#define PROGRAM_VERSION _T("$Revision: 137 $")
#define PROGRAM_RIGHT   _T("Programmed by Salt")


#define PARENT_WINDOW_CLASS_NAME      _T("HHTaskBar")		// MS_SIPBUTTON ���ƁA���܂ɏ�����
#define PARENT_WINDOW_CLASS_NAME_WM61 _T("Tray")

#define TIMER_ID_CPU  (123)					// �K���Ȓl�i�A�v�����ŏd�Ȃ�Ȃ����OK�j
#define TIMER_ID_RAS  (TIMER_ID_CPU + 1)
#define TIMER_ID_NDIS (TIMER_ID_CPU + 2)
