// RCstmBtn.h: interface for the CRCustomButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RCSTMBTN_H__7144E481_2D11_11D2_83A5_00004760763D__INCLUDED_)
#define AFX_RCSTMBTN_H__7144E481_2D11_11D2_83A5_00004760763D__INCLUDED_

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push)
#pragma pack()

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//#include "..\H\Rbutton.h"

class CRCustomButton : public CRButton  
{
public:
	CRCustomButton();
	virtual ~CRCustomButton();
	short m_ID;
};

#pragma pack(pop)
#endif // !defined(AFX_RCSTMBTN_H__7144E481_2D11_11D2_83A5_00004760763D__INCLUDED_)
