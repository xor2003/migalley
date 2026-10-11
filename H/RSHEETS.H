#ifndef	RSHEETS_Included
#define	RSHEETS_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push)
#pragma pack()
	RDialog*	MakeRDEmpty(CWnd* w=NULL);
	RDialog*	MakeRPEmpty(CWnd* w=NULL);
	RDialog*	MakeRtest1(CWnd* w=NULL);
//	RDialog*	MakeRtestsh1(CWnd* w=NULL);

#pragma pack(pop)
#endif