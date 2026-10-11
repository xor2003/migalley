#ifndef	axcolors_Included
#define	axcolors_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

// ActiveX colours
enum	FixedActiveXColours
{
	AXC_DARKEDGE=RGB(103,132,198),
	AXC_LITEDGE=RGB(103,132,198),
	AXC_DISABLEDDARKEDGE=RGB(179,194,226),
	AXC_DISABLEDLITEDGE=RGB(179,194,226),
	AXC_CIRCULARCOMBOBOXCOLOR=RGB(103,132,198)
};
#pragma pack(pop)
#endif