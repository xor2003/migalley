#if !defined(AFX_LSTMSNLG_H__6CE1FC62_CE2F_11D2_8ADB_004005247C35__INCLUDED_)
#define AFX_LSTMSNLG_H__6CE1FC62_CE2F_11D2_8ADB_004005247C35__INCLUDED_

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// LstMsnLg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLastMissionLog dialog

class CLastMissionLog : public RowanDialog
{
// Construction
public:
	 
	CLastMissionLog(CWnd* pParent = NULL);   // standard constructor
	int	rownum[100];
// Dialog Data
	//{{AFX_DATA(CLastMissionLog)
	enum { IDD = IDD_LASTMISSIONLOG };
		// NOTE: the ClassWizard will add data members here
		CRListBox	m_IDC_RLISTBOXCTRL1;
		CRStatic	m_IDC_DETAILS;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLastMissionLog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLastMissionLog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectRlistboxctrl1(long row, long column);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#pragma pack(pop)
#endif // !defined(AFX_LSTMSNLG_H__6CE1FC62_CE2F_11D2_8ADB_004005247C35__INCLUDED_)
