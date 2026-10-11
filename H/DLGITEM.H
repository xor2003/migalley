#pragma once

// Force natural layout - included both before and after DOSDEFS.H's
// leaked pack(1) across TUs; DlgItem offsets must agree everywhere (ODR).
#pragma pack(push)
#pragma pack()

class COleFont;
class CPicture;
class CRButton;
class CRComboExtra;
class CREdit;
class CREditBt;
class CREdtBt;
class CRSpltBar;
class CRTabs;
class CRCombo;
class CRListBox;
class CRRadio;
class CRJoyCfg;
class CRStatic;
class CRSpinBut;

class DlgItem:public CWnd // RERUN moved here from RCOMBOX.H
{
	DECLARE_DYNAMIC(DlgItem)
public:
	operator COleFont*()	{return	(COleFont*)this;}
	operator CPicture*()	{return	(CPicture*)this;}
	operator CRButton*()	{return	(CRButton*)this;}
	operator CRComboExtra*(){return	(CRComboExtra*)this;}
	operator CREdit*()		{return	(CREdit*)this;}
	operator CREditBt*()	{return	(CREditBt*)this;}
	operator CREdtBt*()		{return	(CREdtBt*)this;}
	operator CRSpltBar*()	{return	(CRSpltBar*)this;}
	operator CRTabs*()		{return	(CRTabs*)this;}
	operator CRCombo*()		{return	(CRCombo*)this;}
	operator CRListBox*()	{return	(CRListBox*)this;}
	operator CRRadio*()		{return	(CRRadio*)this;}
	operator CRJoyCfg*()	{return	(CRJoyCfg*)this;}
	operator CRStatic*()	{return	(CRStatic*)this;}
	operator CRSpinBut*()	{return	(CRSpinBut*)this;}
//	operator *()		{return	(*)this;}
//	operator *()		{return	(*)this;}
};

#pragma pack(pop)
