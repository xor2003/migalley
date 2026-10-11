#if !defined(AFX_RDEMPTYP_H__505728C1_2797_11D2_83A5_00004760763D__INCLUDED_)
#define AFX_RDEMPTYP_H__505728C1_2797_11D2_83A5_00004760763D__INCLUDED_

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push)
#pragma pack()

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// RDEmptyP.h : header file
//
#include "RTABS.H" //RERUN

/////////////////////////////////////////////////////////////////////////////
// RDEmptyP dialog

class RDEmptyP : public RowanDialog
{
// Construction
public:
	 
	
	RDEmptyP(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(RDEmptyP)
	enum { IDD = IDD_EMPTYPAGE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RDEmptyP)
	protected:
	// Add these:
	CRTabs m_tabCtrl;
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(RDEmptyP)
	afx_msg void OnSelectTabTabctrl(long tab);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#pragma pack(pop)
#endif // !defined(AFX_RDEMPTYP_H__505728C1_2797_11D2_83A5_00004760763D__INCLUDED_)
