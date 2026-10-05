#ifndef RTABS_H
#define RTABS_H

#include "DLGITEM.H" // RERUN: DlgItem base used below
#include "MFC_stub.h"

class CRTabs : public CWnd
{
	DECLARE_DYNCREATE(CRTabs)
public:
	CRTabs();
	virtual ~CRTabs();

	void SetHorzAlign(BOOL bHorz);
	BOOL GetHorzAlign();
	void AddTab(LPCTSTR text, long data);
	long GetFirstTab();
	void SetFirstTab(long data);
	long CalculateHeight(int width);
	long CalcWidestWord();
	long SelectTab(int index);
	void SetForeColor(OLE_COLOR color);
	void SetFont(CFont* pFont);

protected:
	//{{AFX_MSG(CRTabs)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif