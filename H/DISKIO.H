//------------------------------------------------------------------------------
//Filename       diskio.h
//System         
//Author         Paul.   
//Date           Thu 6 Jul 1995
//Description    
//------------------------------------------------------------------------------
#ifndef	diskio_Included
#define	diskio_Included

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

class	DiskIO
{
	public:
		char	*DiskIO::LoadNumberedFile(short);
		void	DiskIO::SaveNumberedFile(short, char *, short);

	private:

	protected:

};

#pragma pack(pop)
#endif

