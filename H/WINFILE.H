//------------------------------------------------------------------------------
//Filename       winfile.h
//System         
//Author         Paul.   
//Date           Tue 22 Sep 1998
//Description    
//------------------------------------------------------------------------------
#ifndef	WINFILE_Included
#define	WINFILE_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

#define	DEFAULT_WINFILE 0

#if defined (__MSVC__)
extern "C"
{
	__declspec(dllimport) UWord WINAPI InitCDFS();
	__declspec(dllimport) void WINAPI ExitCDFS();
	__declspec(dllimport) BOOL WINAPI SeekToLocation(UWord hFile,ULong fOffset);
};
#else
#endif
// RERUN CDROM related stuff, disable for now
static inline UWord InitCDFS(void) { return 0; }
static inline void ExitCDFS(void) { }
static inline BOOL SeekToLocation(UWord hFile, ULong fOffset) { return 0; }
#pragma pack(pop)
#endif
