//------------------------------------------------------------------------------
//Filename       world.h
//System         
//Author         Paul.   
//Date           Tue 19 Sep 1995
//Description    
//------------------------------------------------------------------------------
#ifndef	WORLD_Included
#define	WORLD_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#include	"WORLDINC.H"
class ViewPoint;
//#include    "display.h"

enum	{		//unsigned												//JIM 09Aug96
	WORLD_2_SECTOR_SHIFT=	20,
	SECTOR_NUMBER_MASK 	=	0x001F,
	REAL_SECTOR_MASK	=	0xFE000000,
	SECTOR_SIZE			=	0x00100000,
	REAL_SECTOR_STEP	=	0x01000000,	//sector_size*s_num_mask/2
		};
enum	{		//signed
	SECTOR_WH			=	32,
	SECTOR_MAXNUM		=	(int)SECTOR_WH*(int)SECTOR_WH
		};
#define	DEFAULT_WORLD 0

//------------------------------------------------------------------------------
struct	RouteHdr
{
	RouteHdr	*next;
	UniqueID	waypoint;
	int			numusers;
};

class	WorldStuff
		{
public:
enum	{PITEMTABLESIZE = (int)IllegalBAND+1};
static	ItemBasePtr		pItem[PITEMTABLESIZE],
						pMapItemHead;
		RouteHdr	*routelist;
		Coords3D 	sunpos;										//RJS 23Mar98
		ANGLES	sunhdg,sunpitch,sunrot;							//RJS 23Mar98
		ViewPoint* vp;

private:

		friend	void mobileitem::MoveList(MobileItemPtr,WorldStuff&);
//TempCode ARM 26Jul96 		friend	mobileitem::MoveList(MobileItemPtr);

		friend  void TransientItem::MoveTransList(TransientItemPtr,WorldStuff&);//MGA 20Mar96
//TempCode ARM 26Jul96 		friend  TransientItem::MoveTransList(TransientItemPtr);//MGA 20Mar96

		itemptr	sectorlist[SECTOR_MAXNUM];

		public:

ItemBasePtr		ConvertPtrUID(UniqueID tmpUID);

			WorldStuff(void );

			~WorldStuff();

			void AddToWorld(itemptr );

			void RemoveFromWorld(itemptr );
			void RemoveFromSector(itemptr,UWord	sector_number);

			void getsectornos(itemptr, UWord &x, UWord &y);

			UWord makeindex(UWord x, UWord y);

			itemptr getfirstitem(UWord );
			UWord GetSector(itemptr );				//PD 25Jun96

			void	ClearWorld();
			
			Bool	SameSector(itemptr,itemptr);

			void	SetLaunchers();

			Bool	SectorEmpty(COORDS3D&);

			void	MakeSunPos(ViewPoint*);						//RJS 23Mar98

			void	SetViewpoint(ViewPoint*);
		private:

		protected:

		};

struct	WorldSearch
{
	int			sectornum;
	ItemPtr	currelt;
	WorldStuff*	world;
	CON	WorldSearch(WorldStuff*	w)
		{world=w;sectornum=-1;currelt=NULL;}
	
	ItemPtr	Next()
		{
			if (currelt)
				currelt=currelt->Next;
			if (!currelt)
			{
				while (sectornum<(int)SECTOR_MAXNUM-1)
				{
					sectornum++;
					currelt=world->getfirstitem((UWord)sectornum);
					if (currelt)
						return(currelt);
				}
		 		
			}
			return(currelt);
		}
};
#pragma pack(pop)
#endif
