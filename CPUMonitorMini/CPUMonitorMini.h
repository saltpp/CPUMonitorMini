/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: CPUMonitorMini.h 134 2008-06-17 11:22:35Z Salt $
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
#define PROGRAM_VERSION _T("Ver.0.40")
//#define PROGRAM_VERSION _T("$Revision: 134 $")
#define PROGRAM_RIGHT   _T("Programmed by Salt")


#define PARENT_WINDOW_CLASS_NAME      _T("HHTaskBar")		// MS_SIPBUTTON ���ƁA���܂ɏ�����
#define PARENT_WINDOW_CLASS_NAME_WM61 _T("Tray")

#define TIMER_ID_CPU  (123)					// �K���Ȓl
#define TIMER_ID_RAS  (TIMER_ID_CPU + 1)
#define TIMER_ID_NDIS (TIMER_ID_CPU + 2)
