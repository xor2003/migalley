//------------------------------------------------------------------------------
//Filename       screenxy.h
//System         New engine
//Author         Dave Whiteside
//Date           Mon 25 Sep 1995
//Description    There are two structures that are used a lot so here they are
//------------------------------------------------------------------------------
#ifndef	SCREENXY_Included
#define	SCREENXY_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_SCREENXY 0

//------------------------------------------------------------------------------
//Author		Dave Whiteside
//Date			Mon 25 Sep 1995
//Modified	
//
//Description	
//
//Inputs		
//------------------------------------------------------------------------------
class	ScreenXY
	{
	public:
		ScreenXY(SWord xx, SWord yy)
		{
			x=xx;
			y=yy;
		}

		ScreenXY()
		{
			x=0;
			y=0;
		}

		ScreenXY	operator+(ScreenXY xy2)
		{
			ScreenXY rv(x+xy2.x,y+xy2.y);
			return(rv);
		}

		SWord	x;
		SWord	y;
	protected:
	private:
	};

//------------------------------------------------------------------------------
//Author		Dave Whiteside
//Date			Mon 25 Sep 1995
//Modified	
//
//Description	
//
//Inputs		
//------------------------------------------------------------------------------
class	ScreenWH
	{
	public:
		ScreenWH()
		{
			w=0;
			h=0;
		}

		ScreenWH(SWord ww, SWord hh)
		{
			w=ww;
			h=hh;
		}

		SWord	w;
		SWord	h;
	protected:
	private:
	};


#pragma pack(pop)
#endif
