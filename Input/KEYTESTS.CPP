//------------------------------------------------------------------------------
//Filename       keytests.cpp
//System
//Author         Jim Taylor
//Date           Mon 23 Oct 1995
//Description    Keyboard testing module
//				There are 2 halves...
//				This is the c - main program task side
//				There is also an asm - ISR side - called keytisr.asm
//------------------------------------------------------------------------------

#include "_INPU.H"

//#include	<dos.h>
#include	"DOSDEFS.H"
#include	"FILES.H"
#include	"FILEMAN.H"
#include	"HARDPASM.H"
#include    "SCREENXY.H"

#define	SPECIAL_KEYMAPS	0 /* RERUN is 1 on windows!! */

#include	"KEYTEST.H"

//KeyMapping	*Debug3dMapTable=ThisKeyMapping;


keytests	Key_Tests;

//
//Procedure		keytests
//Author		Jim Taylor
//Date			Mon 30 Oct 1995
//
//Description	CONSTRUCTOR
//	//constructor for keyboard tests
//creates a number of DOS memory buffers & initialises them
//copies DOS ISR to one of the buffers
//locks DEX ISR
//installs both ISRs
//
//Inputs		None!
//
//Returns		None!
//
//------------------------------------------------------------------------------
extern	void	DLWRITE(void*, ULong);
//RERUN #pragma	aux	DLWRITE	="mov ds:[esi],eax"parm	[esi] [eax]

keytests::keytests()
{

	int	i;
	//create a number of DOS memory buffers & initialises them
	//UI queue
	hitqueueui.flat = new UIKeyQueue;
	//3d mapping
	reftable3d.flat = new KeyMap3d;
	//Clear out
	ClearKeyTables();

	//Buffer for DOS mode ISR

	//copy DOS ISR to the buffer

	//lock DEX ISR - borrowed this code...

	//INSTALL BOTH ISRs
	//Note old vectors

	//Set new vectors

	//TEST CODE

	//	RegUIConv((FileNum) 0);
	//	InsertUIKeyThru=InsertUIKey;
	//?eyFake3dThru=KeyFake3d;

}

//
//Procedure		keytests (destructor)
//Author		Jim Taylor
//Date			Mon 30 Oct 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
keytests::~keytests()
{


}

//
//Procedure		ClearKeyTables
//				(private)
//Author		Jim Taylor
//Date			Mon 30 Oct 1995
//
//Description	Clears out data structures for keyboard- discards unused presses
//
//Inputs		none
//
//Returns		none
//
//------------------------------------------------------------------------------
void	keytests::ClearKeyTables()
{
	int	i, j;
	//First, disable both keyboard systems
	UIKeyQueue* keyqueue = (UIKeyQueue*)hitqueueui.flat;
	keyqueue->active = false;
	KeyMap3d* keymap = (KeyMap3d*)reftable3d.flat;
	keymap->active = false;

	//now zero everything in queue

	keyqueue->currshifts = 0;
	keyqueue->addref = 0;	//you may add when they are equal.
	//You can not add if they will become equal
	keyqueue->takeref = 0;	//you may not take when they are equal
	keyqueue->queuesize = MAXqueuesize;
	for (i = 16; i--;)
		keyqueue->keyholds[i] = 0;

	//now zero everything in 3d system

	keymap->currshifts = 0;
	for (i = 16; i--;)
		keymap->keyholds[i] = 0;
	for (i = 128; i--;)
		for (j = 8; j--;)
			keymap->mappings[i][j] = 0;
	for (i = (TOTALKEYBITFLAGS + 7) / 4; i--;)//2 bits per key			//RDH 15Sep97
		keymap->bitflags[i] = 0;
}


//
//Procedure		Reg3dConv
//Author		Jim Taylor
//Date			Mon 30 Oct 1995
//
//Description	Register a new conversion table
//
//Inputs		filenum:	table containing conversion
//
//Returns		none
//
//------------------------------------------------------------------------------
void	keytests::Reg3dConv(FileNum	filenum)
{
	if (filenum)
	{
		fileblock
			tmp(filenum);
		//This file is made up of word pairs:
		//1st word indexes the mappings array by 2 bytes - 0-128,0-7.
		//2nd word indexes the bitflags by bit index. Always even. 2 bits per mapping.
		//
		Reg3dConv((KeyMapping*)getdata(tmp), getsize(tmp) / 4);
	}
	else
	{
		ClearKeyTables();
		((KeyMap3d*)reftable3d.flat)->active = true;
	}
}

//
//Procedure		Reg3dConv
//Author		Jim Taylor
//Date			Mon 30 Oct 1995
//
//Description	maps 1 3d keypress manually
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	keytests::Reg3dConv(KeyMapping* mapreq, int i)
{
	ClearKeyTables();
	KeyMap3d* keymap = (KeyMap3d*)reftable3d.flat;

	//Bound by pointer, not count: the table may have no terminator, so
	//the original peek at the next record after mapreq++ read past the
	//file block. The bound check needs no load, so no entry past the
	//buffer is ever touched. scancode/shiftstate must stay inside
	//mappings[MAXKEYS][8] or the write corrupts keymap.
	KeyMapping* mapend = mapreq + (i > 0 ? i : 0);
	while (mapreq < mapend)
	{
		if (mapreq->scancode == 0)
			break;
		if (mapreq->scancode < KeyMap3d::MAXKEYS && mapreq->shiftstate < 8)
			keymap->mappings[mapreq->scancode][mapreq->shiftstate] = mapreq->bitflag;
		mapreq++;
	}
	((KeyMap3d*)reftable3d.flat)->active = true;
	ModeIs3D = BOOL_TRUE;

}

//
//Procedure		RegUIConv
//Author		Jim Taylor
//Date			Mon 30 Oct 1995
//
//Description	Register a userinterface keyboard conversion
//				This will be a stack of converters, starting with the
//				main user-interface keys.
//
//Inputs		filenum: translation list
//
//Returns
//
//------------------------------------------------------------------------------
void	keytests::RegUIConv(FileNum)	//filenum)
{
	if (((UIKeyQueue*)hitqueueui.flat)->active == false)
		ClearKeyTables();
	//This file registers a foreground translation buffer rather than an interrupt feature
	//So I will ignore for now, and just activate the interrupt service!
	((UIKeyQueue*)hitqueueui.flat)->active = true;
	ModeIs3D = BOOL_FALSE;
}

//
//Procedure		UnRegUIConv
//Author		Jim Taylor
//Date			Mon 30 Oct 1995
//
//Description	Deregisters keyboard mapping for user interface
//				The user interface mappings are a stack
//
//Inputs		none
//
//Returns		none
//
//------------------------------------------------------------------------------
void	keytests::UnRegUIConv()
{}
//
//Procedure		GetUIKey
//Author		Jim Taylor
//Date			Mon 30 Oct 1995
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
bool	keytests::GetUIKey(UIKeyEvent& returnedevent)
{
	UIKeyQueue* keyqueue = (UIKeyQueue*)hitqueueui.flat;
	int	i;
	{
		//        _disable();
		i = keyqueue->addref - keyqueue->takeref;
	}
	//    _enable();
	if (i == 0)
		return(false);
	returnedevent = keyqueue->keyqueue[keyqueue->takeref];
	{
		//       _disable();
		keyqueue->takeref++;
		if (keyqueue->takeref == keyqueue->queuesize)
			keyqueue->takeref = 0;
	}
	//    _enable();
	return (true);
}

//
//Procedure		mousepos
//Author		Jim Taylor
//Date			Tue 30 Jan 1996
//
//Description
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
/*ScreenXY&	keytests::mousepos()
{
UIKeyQueue*
keyqueue=(UIKeyQueue*)	hitqueueui.flat;
return(keyqueue->currmouse);
}*/

//
//Procedure		InsertUIKey
//Author		Jim Taylor
//Date			Tue 30 Jan 1996
//
//Description	insert a key press/release event into the key queue
//
//Inputs
//
//Returns
//
//------------------------------------------------------------------------------
void	keytests::InsertUIKey(UIKeyNoMouse& keyfake)
{
	UIKeyQueue* keyqueue = (UIKeyQueue*)hitqueueui.flat;
	{
		//    _disable();
		int		oldadd = keyqueue->addref;
		int		addref = oldadd + 1;

		if (addref == keyqueue->queuesize)
		{
			addref = 0;
		}
		if (addref == keyqueue->takeref)
		{
			//            _enable();											//JIM 21Oct96
			return;
		}
		keyqueue->keyqueue[oldadd].keycode = keyfake.keycode;
		keyqueue->keyqueue[oldadd].keyhit = keyfake.keyhit;
		if (keyfake.keyshifts)
			keyqueue->keyqueue[oldadd].keyshifts = keyfake.keyshifts;
		else
			keyqueue->keyqueue[oldadd].keyshifts = keyqueue->currshifts;
		//        keyqueue->keyqueue[oldadd].mousecoords=keyqueue->currmouse;
		keyqueue->addref = addref;
	}
	//_enable();
}


//
//Procedure		KeyHeld3d
//Author		Jim Taylor
//Date			Fri 3 Nov 1995
//
//Description	Check if a key is currently held down
//
//Inputs		Alias to test for
//
//Returns		true if not been released
//
//------------------------------------------------------------------------------

bool	keytests::KeyHeld3d(KeyVal3D keyval)
{
	KeyMap3d* keymap = (KeyMap3d*)reftable3d.flat;

	if (keymap == NULL)
		return false; //RERUN added

	if (BITRESET((keymap->bitflags), keyval + 1))
		return(true);
	return	(BITTEST((keymap->bitflags), keyval));
	return false;
}

//
//Procedure		KeyPress3d
//Author		Jim Taylor
//Date			Fri 3 Nov 1995
//
//Description	Check if a key has been pressed since last test
//
//Inputs		Alias to test for
//
//Returns		true	if just pressed
//
//------------------------------------------------------------------------------
bool	keytests::KeyPress3d(KeyVal3D keyval)
{
	KeyMap3d*    keymap = (KeyMap3d*)reftable3d.flat;
	
	if (keymap == NULL)
		return false; //RERUN added

	return(BITRESET((keymap->bitflags), keyval + 1));
}

//
//Procedure		KeyFake3d
//Author		Jim Taylor
//Date			Fri 3 Nov 1995
//
//Description	Fakes a key press to the 3d
//
//Inputs		Alias to fake
//				set or clear bits
//
//Returns		true	if just pressed
//
//------------------------------------------------------------------------------
void	keytests::KeyFake3d(KeyVal3D keyval,	//combo to fake
	bool	held,		//set/clear held flag
	bool	hit)		//set/clear oneshot flag
{
	KeyMap3d*   keymap = (KeyMap3d*)reftable3d.flat;

	if (keymap == NULL)
		return; //RERUN added

	if (held)
		BITSET((keymap->bitflags), keyval);
	else
		BITRESET((keymap->bitflags), keyval);
	if (hit)
		BITSET((keymap->bitflags), keyval + 1);
	else
		BITRESET((keymap->bitflags), keyval + 1);

}
