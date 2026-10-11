//------------------------------------------------------------------------------
//Filename       tilemake.h
//System         
//Author         Paul.   
//Date           Thu 16 Jul 1998
//Description    
//------------------------------------------------------------------------------
#ifndef	TILEMAKE_Included
#define	TILEMAKE_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_TILEMAKE 0

struct DataRecord;

struct Rect
{
	SLong t,b,l,r;
};

const SLong _maxClips=5;

struct ClipInfo
{
	UByte level;
	UByte clipRecIndex;
};

struct SquareClips
{
	UWord count;
	ClipInfo clipInfo[_maxClips];
};

struct ROWANSURFACEDESC;

class TMake
{
public:
	TMake();
	~TMake();

	void Init(const SLong,const SLong,MigWindow*) const;
	void Reinit(const SLong,const SLong,MigWindow*) const;
	void Exit() const;
	void ChkUpdate(const SLong,const SLong) const;
	DataRecord GetBlock(const SLong x,const SLong z) const;
	void GetSquares(Rect*) const;
	void GetClipRegion(const UByte,Rect&) const;
	void GetTileClipData(SquareClips*&);
	void BuildShadowList(WorldStuff*,itemptr);
	void RenderTile2Surface(UByte*,ROWANSURFACEDESC*);
	void Render2Surface(const HTEXT&,UByte,UWord);
	void GetMinMaxAlt(const SLong x,const SLong z,const SWord dir,SLong& min,SLong& max);
	void GetMinMaxAlt(const SLong x,const SLong z,SLong& min,SLong& max);
	void DoGetTextureScales(SLong&,SLong&,SLong&,SLong&,SLong&,SLong&);
	ROWANSURFACEDESC* GetRSD(const HTEXT&) const;
	void DoFreeTextures();
	void DoFreeTexture(const HTEXT& ht) const;
	HTEXT DoAllocateTexture(UByte);
	bool CheckCache();
};

extern TMake* tMake;

#pragma pack(pop)
#endif
