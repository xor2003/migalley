#if !defined(AFX_DISNOTES_H__7FB62881_0C5A_11D2_A5A3_0040052179B6__INCLUDED_)
#define AFX_DISNOTES_H__7FB62881_0C5A_11D2_A5A3_0040052179B6__INCLUDED_

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// disnotes.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Cdisnotes dialog

class Cdisnotes : public RowanDialog
{
// Construction
public:
	Cdisnotes(int text, CWnd* pParent = NULL);   // standard constructor
	int idtext;
// Dialog Data
	//{{AFX_DATA(Cdisnotes)
	enum { IDD = IDD_DISNOTES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cdisnotes)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Cdisnotes)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#pragma pack(pop)
#endif // !defined(AFX_DISNOTES_H__7FB62881_0C5A_11D2_A5A3_0040052179B6__INCLUDED_)
