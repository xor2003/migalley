//------------------------------------------------------------------------------
//Filename       msn_ctrl.h
//System         
//Author         R. Hyde 
//Date           Wed 17 Jul 1996
//Description    
//------------------------------------------------------------------------------
#ifndef	MSN_CTRL_Included
#define	MSN_CTRL_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_MSN_CTRL 0

	enum CtrlVar {	CTRL_NONE =0,CTRL_LOAD,CTRL_FLY,

					CTRL_QUIT2DOS,
					CTRL_CANCEL,CTRL_ACCEPT,

					GOTOMAP,GOTOACCEL,CTRL_CONFIG_FROM_3D,CTRL_SAVE,


					CTRL_H2HQUIT,CTRL_H2HPLAY

					
					};

enum	{		CLIPBOARDAREAX = PIXEL640 * 85,					//RDH 20Jul96
				INTROCLIPBOARDAREAX = PIXEL640 * 14,
				CLIPBOARDAREAY = PIXEL640 * 136,
				CLIPBOARDAREAW = PIXEL640 * 412, 
				CLIPBOARDAREAH = PIXEL640 * 344,
				CRCTEXTAREAX = PIXEL640 * 148, 					//RDH 08Mar96
				CRCTEXTAREAY = PIXEL640 * 169,
				CRCTEXTAREAW = PIXEL640 * 339, 
				CRCTEXTAREAH = PIXEL640 * 311
				
				};



#pragma pack(pop)
#endif
