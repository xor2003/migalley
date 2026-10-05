#include "DLGITEM.H" // RERUN: DlgItem base used below
#include "FILES.H" // RERUN: FileNum used below
#if !defined(AFX_RSCRLBAR_H__9AF635E2_8AE0_11D2_9684_0000476073D8__INCLUDED_)
#define AFX_RSCRLBAR_H__9AF635E2_8AE0_11D2_9684_0000476073D8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CRScrlBar window

class CRScrlBar : public CWnd
{
protected:
	DECLARE_DYNCREATE(CRScrlBar)

// Attributes
public:
	long GetMinValue();
	void SetMinValue(long);
	long GetMaxValue();
	void SetMaxValue(long);
	long GetStepSize();
	void SetStepSize(long);
	BOOL GetHorzAlign();
	void SetHorzAlign(BOOL);
	long GetParentPointer();
	void SetParentPointer(long);
	long GetPageSize();
	void SetPageSize(long);
	long GetScrollPos();
	void SetScrollPos(long);
	BOOL GetUseMessagesInsteadOfEvents();
	void SetUseMessagesInsteadOfEvents(BOOL);
	long GetFileNumOffset();
	void SetFileNumOffset(long);

// Operations
public:
	void Move(long left, long top, long right, long bottom);

protected:
	//{{AFX_MSG(CRScrlBar)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RSCRLBAR_H__9AF635E2_8AE0_11D2_9684_0000476073D8__INCLUDED_)
