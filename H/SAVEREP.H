#if !defined(AFX_SAVEREP_H__85EC0320_969B_11D2_9684_0000476073D8__INCLUDED_)
#define AFX_SAVEREP_H__85EC0320_969B_11D2_9684_0000476073D8__INCLUDED_

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SaveRep.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSaveReplay dialog

class CSaveReplay : public CDialog
{
// Construction
public:
	CSaveReplay(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSaveReplay)
	enum { IDD = IDD_SAVEREPLAY };
	CEdit	m_IDCX_SAVENAME;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaveReplay)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSaveReplay)
	afx_msg void OnUpdateSavename();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#pragma pack(pop)
#endif // !defined(AFX_SAVEREP_H__85EC0320_969B_11D2_9684_0000476073D8__INCLUDED_)
