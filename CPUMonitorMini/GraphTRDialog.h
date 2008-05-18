/**
 *----------------------------------------------------------------------------
 *
 * @file	$Id: GraphTRDialog.h 118 2008-05-11 11:26:18Z Shiono $
 * @brief	CGraphDialog ���p�����āA�_�O���t�̕`�������AReceived, Transmitted �ŏd�˂ĕ\����悤�ɕύX�����N���X
 *			�f�[�^�́A����ɂ�����p���������ō���Ă��炤
 *
 * @author  Salt
 *
 *----------------------------------------------------------------------------
 */

#pragma once

#include "GraphDialog.h"

class CGraphTRDialog : public CGraphDialog {

private:
	int m_nYLinePrev;
	BOOL m_bHidPrev;


protected:
	// message handlers
	virtual BOOL OnTimer(HWND hWnd, int nIDEvent);


public:
	CGraphTRDialog(void);
	virtual ~CGraphTRDialog(void);

	virtual BOOL GetData(int &nBarTransmitted, int &nBarReceived, BOOL bLine, int &nLine) = 0;		// have to return 0..100 for nBarXxxx and nLine
};
