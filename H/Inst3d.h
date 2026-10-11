#ifndef INST3D_H
#define INST3D_H

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

class Inst3d
{	//master control for ongoing 3d world instances
	friend	class	wxMIGApp;
	friend	class	View3d;
	friend	class	Mast3d;
	friend	class	Replay;
	friend	class	Persons2;
	friend	class	Persons3;
	friend	class	DPlay;

	enum {
		MAX_ACCEL_FRAME = 100,
#ifndef	NDEBUG
		MAX_NORMAL_FRAME = 7
#else
		MAX_NORMAL_FRAME = 25
#endif
	};
private:
	int	accelframes;
	bool		accel, paused, interactive;
	View3d*		viewedwin;
	Inst3d*		nextinst;
	volatile bool	insidetimer;
	static	KeyMap3d*	commonkeymaps;
	WorldRef	world;
	LiveList*	livelist;
	void	Inst3d::BlockTick(Bool setit);
	bool	mapview;
	int		timeofday;
	volatile int	framecount;

public:
#ifdef DECLARE_HANDLE
	HANDLE			semaphore;
	HANDLE			mutex;
#endif

	Inst3d();
	Inst3d(bool);
	~Inst3d();
	View3d* Interactive(View3d* newwin);
	bool Paused(bool newmode);
	bool Accel(bool newmode);
	View3d* Drawing(View3d* newwin);
	View3d* Interactive();
	bool Paused();
	bool Accel();
	View3d* Drawing();
	static	bool	InThe3D();//	{return !currinst->Paused();}
	static	void	ReleaseDirectX();
	static	void	RestoreDirectX();

	void	DoMoveCycle();
	void	MoveCycle(WorldStuff* worldref);						//AMM 11May99
	
	static void	OnKeyDown(int key);
	static void	OnKeyUp(int key);
	HANDLE	movethread; // thread that runs moveloop
	static unsigned int moveloop(LPVOID x); // RERUN removed the static

private:
	static void OnKeyInput();
};

#pragma pack(pop)
#endif //INST3D_H
