//------------------------------------------------------------------------------
//Filename       mathasm.h
//System         
//Author         Jim Taylor
//Date           Fri 12 Jul 1996
//Description 
//
//Math based assembler inline code.	  //JIM 20/05/99				  //JIM 20/05/99
//
//__fastcall defines:
//	param1:		ECX
//	param2:		EDX
//	param3..	on stack
//	return val	EAX
//	To get the optimiser to bother generating code, we must write the 
//  following dummy statements which the optimiser will remove (yippee!):
//		mov	ecx,param1
//		mov	edx,param2
//	You can use eax as scratch as it must contain the return value on exit.
//	At present I don't know wether the optimiser assumes you may damage ecx and edx
//	The routines I wrote today never damage ecx or edx
//
//	Occasionally, the compiler/optimiser can do better than we can 
//	and using __stdcall will allow the compiler to present the values to
//	our assembly code more efficiently, but often we need to do defensive 
//	stacking, losing the benefit.
//
//------------------------------------------------------------------------------
#ifndef	MATHASM_Included
#define	MATHASM_Included

#define	DEFAULT_MATHASM 0

#include "DOSDEFS.H"
#include <cstdint>

#pragma warning(disable:4035)
//������������������������������������������������������������������������������
//Procedure		BITRESET
//Author		Jim Taylor
//Date			Fri 3 Nov 1995
//
//Description	bit test and reset assembler macro. 
//				Tests a whole array of memory... up to 512Mb in principal! 
//
//Inputs		pointer to word memory
//				index to test
//
//Returns		false=0/true
//
//------------------------------------------------------------------------------

#ifdef __WATCOMC__
extern	Bool	BITRESET(void*,ULong);
#pragma	aux		BITRESET		=		\
		"btr	ds:[esi],ecx"			\
		"setc	al"					\
		parm	[esi] [ecx]				\
		modify exact	[al]			\
		value	[al]
#else
#ifdef __MSVC__
inline  bool __fastcall	BITRESET(void* num1,ULong num2)
{
	_asm 
	{
		mov	ecx,num1
		mov	edx,num2
		btr	ds:[ecx],edx
		setc	al
	}
}
#elif defined(__GNUC__)
inline bool BITRESET(void* base, unsigned long bit_index) {
    unsigned char prev;
    unsigned char* addr = static_cast<unsigned char*>(base) + ((bit_index >> 5) * 4);
    unsigned long bit = bit_index & 31UL;

    asm volatile (
        "btrl %2, (%1)\n\t"
        "setc %0"
        : "=q"(prev)
        : "r"(addr), "r"(bit)
        : "memory"
    );
    return prev;
}

#else
extern	Bool	BITRESET(void*,ULong);
#endif
#endif

//-----------------------------------------------------------------------------
// Procedure    BITRESETI
// Author       Paul
// Date         31/10/97
//
// Description
//
// Inputs
//
// Returns
//
//-----------------------------------------------------------------------------
#ifdef __WATCOMC__
extern	ULong	BITRESETI(ULong,ULong);
#pragma	aux		BITRESETI		=		\
		"btr	eax,ecx"			\
		parm	[eax] [ecx]				\
		modify exact	[eax]			\
		value	[eax]
#else
#ifdef __MSVC__
inline ULong __fastcall BITRESETI(ULong num1,ULong num2)
{
	_asm 
	{  
		mov	eax,num1
		mov	edx,num2
		btr	eax,edx
	}
}
#elif defined(__GNUC__)
inline ULong BITRESETI(ULong value, ULong bit_index)
{
    return value & ~(ULong(1) << (bit_index & 31));
}
#endif
#endif


//������������������������������������������������������������������������������
//Procedure		BITSET
//Author		Jim Taylor
//Date			Fri 3 Nov 1995
//
//Description	bit test and reset assembler macro.
//				Tests a whole array of memory... up to 512Mb in principal!
//
//Inputs		pointer to word memory
//				index to test
//
//Returns		false=0/true
//
//------------------------------------------------------------------------------
#ifdef __WATCOMC__
extern	Bool	BITSET(void*,ULong);
#pragma	aux		BITSET		=			\
		"bts	ds:[esi],ecx"			\
		"setc	al"					\
		parm	[esi] [ecx]				\
		modify exact	[al]			\
		value	[al]					\

#else
#ifdef __MSVC__
inline bool __fastcall BITSET(void* num1,ULong num2)
{
	_asm
	{
		mov	ecx,num1
		mov	edx,num2
		bts	ds:[ecx],edx
		setc	al
	}
}
#elif defined(__GNUC__)
inline bool BITSET(void* base, unsigned long bit_index) {
    unsigned char prev;
    // Compute address for dword at (bit_index >> 5)
    unsigned char* addr = static_cast<unsigned char*>(base) + ((bit_index >> 5) * 4);
    unsigned long bit = bit_index & 31UL;

    asm volatile (
        "btsl %2, (%1)\n\t"
        "setc %0"
        : "=q"(prev)              // output: prev in a byte reg
        : "r"(addr), "r"(bit)     // inputs: addr, bit within dword
        : "memory"                // clobbers: memory (RMW)
    );
    return prev;
}

#else
extern	Bool	BITSET(void*,ULong);
#endif
#endif

//-----------------------------------------------------------------------------
// Procedure    BITSETI
// Author       Paul
// Date         31/10/97
//
// Description  
//
// Inputs       
//
// Returns      
//
//-----------------------------------------------------------------------------
#ifdef __WATCOMC__
extern	ULong	BITSETI(ULong,ULong);
#pragma	aux		BITSETI		=		\
		"bts	eax,ecx"			\
		parm	[eax] [ecx]				\
		modify exact	[eax]			\
		value	[eax]					\

#else
#ifdef __MSVC__
inline ULong __fastcall BITSETI(ULong num1, ULong num2)
{
	_asm 
	{
		mov	eax,num1
		mov	edx,num2
		bts	eax,edx
	}
}
#elif defined(__GNUC__)
inline ULong BITSETI(ULong value, ULong bit_index)
{
    return value | (ULong(1) << (bit_index & 31));
}
#endif
#endif


//������������������������������������������������������������������������������
//Procedure		BITTEST
//Author		Jim Taylor
//Date			Fri 3 Nov 1995
//
//Description	bit test and reset assembler macro. 
//				Tests a whole array of memory... up to 512Mb in principal!
//
//Inputs		pointer to word memory
//				index to test
//
//Returns		false=0/true
//
//------------------------------------------------------------------------------


#ifdef __WATCOMC__
extern	Bool	BITTEST(const void*,ULong);
#pragma	aux		BITTEST		=			\
		"bt		ds:[esi],ecx"			\
		"setc	al"					\
		parm	[esi] [ecx]				\
		modify exact	[al]			\
		value	[al]					\

#else
#ifdef __MSVC__
inline bool __fastcall BITTEST(const void* num1,ULong num2)
{
	_asm
	{
		mov	ecx,num1
		mov	edx,num2
		bt	ds:[ecx],edx
		setc	al
	}
}
#elif defined(__GNUC__)
inline bool BITTEST(const void* base, uint64_t bit_index)
{
    const uint8_t* ptr = static_cast<const uint8_t*>(base);
    const uint32_t* dword = reinterpret_cast<const uint32_t*>(
        ptr + ((bit_index >> 5) * 4)
    );
    return (*dword & (1u << (bit_index & 31))) != 0;
}
#else
extern	Bool	BITTEST(const void*,ULong);
#endif
#endif

//-----------------------------------------------------------------------------
// Procedure    BITTESTI
// Author       Paul
// Date         31/10/97
//
// Description  
//
// Inputs
//
// Returns      
//
//-----------------------------------------------------------------------------
#ifdef __WATCOMC__
extern	Bool	BITTESTI(int,ULong);
#pragma	aux		BITTESTI		=			\
		"bt		esi,ecx"			\
		"setc	al"					\
		parm	[esi] [ecx]				\
		modify exact	[al]			\
		value	[al]					\

#else
#ifdef __MSVC__
inline bool __fastcall BITTESTI(int num1,ULong num2)
{
	_asm 
	{
		mov	ecx,num1
		mov	edx,num2
		bt	ecx,edx
		setc	al
	}
}
#elif defined(__GNUC__)
inline bool BITTESTI(int value, ULong bit_index)
{
    return (value & (1 << (bit_index & 31))) != 0;
}
#endif
#endif

//������������������������������������������������������������������������������
//Procedure		BITCOMP
//Author		Jim Taylor
//Date			Fri 3 Nov 1995
//
//Description	bit test and reset assembler macro. 
//				Tests a whole array of memory... up to 512Mb in principal! 
//
//Inputs		pointer to word memory
//				index to test
//
//Returns		false=0/true - old value
//
//------------------------------------------------------------------------------
#ifdef __WATCOMC__
extern	Bool	BITCOMP(void*,ULong);
#pragma	aux		BITCOMP		=			\
		"btc	ds:[esi],ecx"			\
		"setc	al"					\
		parm	[esi] [ecx]				\
		modify exact [al]				\
		value	[al]
#else
#ifdef __MSVC__
inline bool __fastcall BITCOMP(void* num1,ULong num2)
{
	_asm
	{
		mov	ecx,num1
		mov	edx,num2
		btc	ds:[ecx],edx
		setc	al
	}

}
#elif defined(__GNUC__)
inline bool BITCOMP(void* base, ULong bit_index)
{
    uint8_t* p = static_cast<uint8_t*>(base);
    volatile uint32_t* word = reinterpret_cast<volatile uint32_t*>(
        p + ((bit_index >> 5) * 4)
    );
    const uint32_t mask = 1u << (bit_index & 31);

    bool was_set = (*word & mask) != 0;
    *word ^= mask;
    return was_set;
}
#else
extern	Bool	BITCOMP(void*,ULong);
#endif
#endif

//-----------------------------------------------------------------------------
// Procedure    BITCOMPI
// Author       Paul
// Date         31/10/97
//
// Description  
//
// Inputs       
//
// Returns      
//
//-----------------------------------------------------------------------------
#ifdef __WATCOMC__
extern	ULong	BITCOMPI(ULong,ULong);
#pragma	aux		BITCOMPI		=		\
		"btc	eax,ecx"			\
		parm	[eax] [ecx]				\
		modify exact	[eax]			\
		value	[eax]					\

#else
#ifdef __MSVC__
inline ULong __fastcall BITCOMPI(ULong num1,ULong num2)
{
	_asm 
	{
		mov	eax,num1
		mov	edx,num2
		btc	eax,edx
	}
}
#elif defined(__GNUC__)
inline ULong BITCOMPI(ULong value, ULong bit_index)
{
    return value ^ (ULong(1) << (bit_index & 31));
}
#endif
#endif

//������������������������������������������������������������������������������
//Procedure		BSF/BSR (BITSCANLOWEST/HIGHEST)
//Author		Jim Taylor
//Date			Fri 3 Nov 1995
//
//Description	Return the lowest/highest set bit number
//				If the input has no bits set then errcode is returned... 
//				The keyboard uses 0. Other code may use -1 or 32.
//				NOTE:	These routines test a register value - i.e. 1 location
//
//	To test a block of memory use:
//
//	ULong* arrptr=arrstart;
//	ULong	accum=0;
//	for(int arrcount=0;arrcount<ARRSIZE*32;arrcount+=32)
//		if ((accum+=arrptr++[0])!=0))
//		{
//		 	arrcount+=BITSCANLOWEST(accum,0);
//			break;
//		}
//	-Exits with accum = the bit or accum = ARRSIZE*32 if no bit
//
//	ULong* arrptr=&arrstart[ARRSIZE];
//	ULong	accum=0;
//	for(int arrcount=ARRSIZE*32;arrcount>=0;arrcount-=32)
//		if ((accum+=arrptr++[0])!=0))
//		{
//		 	arrcount+=BITSCANHIGHEST(accum,0);
//			break;
//		}
//	-Exits with accum = the bit or accum negative if no bit
//
//	This should compile down to optimal assembler.
//
//
//
//Inputs		pointer to word memory
//				index to test
//
//Returns		bit number or errcode
//
//------------------------------------------------------------------------------
#include <cstring>   // C++ version: std::memcpy

#ifdef __WATCOMC__
extern	ULong	BITSCANLOWEST(ULong bits,ULong errcode);
#pragma	aux		BITSCANLOWEST	=		\
		"bsf	edx,eax"				\
		parm	[eax] [edx]				\
		modify exact	[edx]			\
		value	[edx]
#else
#ifdef __MSVC__
inline ULong __fastcall BITSCANLOWEST(ULong bits,ULong errcode=0)
{
	_asm
	{
		mov	ecx,bits
		mov	eax,errcode
		bsf	ecx,eax
	}
}
#elif defined(__GNUC__)
inline ULong BITSCANLOWEST(ULong bits, ULong errcode = 0)
{
    if (bits == 0)
        return errcode;
    return __builtin_ctz(bits);     // GCC/Clang: count trailing zeros → lowest set bit
}
#endif
#endif

#ifdef __WATCOMC__
extern	ULong	BITSCANHIGHEST(ULong bits,ULong errcode);
#pragma	aux		BITSCANHIGHEST	=		\
		"bsr	edx,eax"				\
		parm	[eax] [edx]				\
		modify exact [edx]				\
		value	[edx]
#else
#ifdef __MSVC__
inline ULong __fastcall BITSCANHIGHEST(ULong bits,ULong errcode=0)
{
	_asm
	{
		mov	ecx,bits
		mov	eax,errcode
		bsr	ecx,eax
	}
}
#elif defined(__GNUC__)
inline unsigned long BITSCANHIGHEST(unsigned long bits, unsigned long errcode = 0)
{
	if (bits == 0) return errcode;
		return 31 - __builtin_clz(bits);
}
#endif
#endif

//������������������������������������������������������������������������������
//Procedure		MUL[SH/DIV][SIN/UNS]
//Author		Jim Taylor
//Date			Fri 3 Nov 1995
//
//Description	Double long intermediate maths
//				Multiply to a 64 bit result, then divide or shift back to 32 bit
//				Note: if the division doesn't result in 32 bits you are in trouble
//
//Inputs		2 multipliers and a divisor
//				
//
//Returns		
//
//------------------------------------------------------------------------------

#ifdef __WATCOMC__
extern	ULong MULSHUNS (ULong,ULong,UByte);
#pragma	aux	MULSHUNS		=			\
		"mul	edx"	  				\
		"shrd	eax,edx,cl"				\
		parm caller	[eax] [edx] [cl]	\
		modify	exact [edx eax]			\
		value	[eax]
#else
#ifdef __MSVC__
inline ULong __fastcall MULSHUNS (ULong num1, ULong num2, UByte num3)
{
    _asm
    {
		mov		ecx,num1
		mov		eax,num2
		push	edx
		push	ecx
		mul		ecx
		mov		cl,num3
		shrd	eax,edx,cl
		pop		edx
		pop		ecx
    }
}
#elif defined(__GNUC__)
inline ULong MULSHUNS(ULong num1, ULong num2, UByte num3)
{
    const uint64_t product = static_cast<uint64_t>(num1) * num2;
    return static_cast<ULong>(product >> (num3 & 31));
}
#endif
#endif

#ifdef __WATCOMC__
extern	SLong	MULSHSIN (SLong,SLong,UByte);
#pragma	aux	MULSHSIN		=		\
		"imul	edx"	  			\
		"shrd	eax,edx,cl"			\
		parm	[eax] [edx] [cl]	\
		modify	exact [edx eax]		\
		value	[eax]
#else
#ifdef __MSVC__
inline SLong __fastcall MULSHSIN (SLong num1, SLong num2, UByte num3)
{
    _asm
    {
		mov		ecx,num1
		mov		eax,num2
		push	edx
		push	ecx
		Imul	ecx
		mov		cl,num3
		shrd	eax,edx,cl
		pop		edx
		pop		ecx
    }
}
#elif defined(__GNUC__)
inline SLong MULSHSIN(SLong a, SLong b, UByte shift)
{
    const uint64_t product = static_cast<uint64_t>(static_cast<int64_t>(a) * b);
    const unsigned int safe_shift = shift & 31;   // CPU masks to 5 bits
    return static_cast<SLong>(product >> safe_shift);   // unsigned shift → zeros in!
}
#else
extern	SLong	MULSHSIN (SLong,SLong,UByte);
#endif
#endif

#ifdef __WATCOMC__
extern	ULong SHDIVUNS (ULong,UByte,ULong);
#pragma	aux	SHDIVUNS		=			\
		"mov	edx,eax"				\
		"mov	bl,32"					\
		"shl	eax,cl"					\
		"sub	bl,cl"					\
		"mov	cl,bl"					\
		"shr	edx,cl"					\
		"div	esi"					\
		parm caller	[eax] [cl] [esi]	\
		modify	[ebx edx eax]	   		\
		value	[eax]
#else
#ifdef __MSVC__
inline ULong SHDIVUNS (ULong num1, UByte num2, ULong num3)
{
    _asm
    {
		push	ecx
		push	edx
		mov		eax,num1
		mov		cl,num2
		mov		edx,eax
		shl		eax,cl
		sub		ecx,32
		neg		ecx
		shr		edx,cl	;;sign extending...
		div		num3
		pop		edx
		pop		ecx
    }
}
#elif defined(__GNUC__)
inline ULong SHDIVUNS(ULong value, UByte shift, ULong divisor)
{
    const unsigned int safe_shift = shift & 31;
    const uint64_t shifted = (uint64_t)value << safe_shift;
    return divisor ? static_cast<ULong>(shifted / divisor) : 0;
}
#endif
#endif

#ifdef __WATCOMC__
extern	ULong SHDIVSIN (SLong,UByte,ULong);
#pragma	aux	SHDIVSIN		=			\
		"mov	edx,eax"				\
		"mov	ebx,32"					\
		"shl	eax,cl"					\
		"sub	ebx,ecx"				\
		"mov	ecx,ebx"				\
		"sar	edx,cl"					\
		"idiv	esi"					\
		parm caller	[eax] [cl] [esi]	\
		modify	[ebx edx eax]	   		\
		value	[eax]
#else
#ifdef __MSVC__
inline ULong SHDIVSIN (SLong num1,UByte num2,ULong num3)
{
    _asm
    {
		push	ecx
		push	edx
		mov		eax,num1
		mov		cl,num2
		mov		edx,eax
		shl		eax,cl
		sub		ecx,32
		neg		ecx
		sar		edx,cl	;;sign extending...
		idiv	num3
		pop		edx
		pop		ecx
    }
}
#elif defined(__GNUC__)
inline SLong SHDIVSIN(SLong value, UByte shift, ULong divisor)
{
    // RERUN: shipped asm truncates value<<shift to 32 bits before
    // sign-extending into edx — the 64-bit shift kept high bits that
    // the shipped code lost (|value|>=0x8000 at shift 16 diverged).
    const int32_t lo = (int32_t)((uint32_t)value << (shift & 31));
    return divisor ? static_cast<SLong>((int64_t)lo / (int32_t)divisor) : 0;
}

#else
extern	ULong SHDIVSIN (SLong,UByte,ULong);
#endif
#endif

#ifdef __WATCOMC__
extern	ULong	MULDIVUNS (ULong,ULong,ULong);
#pragma	aux	MULDIVUNS		=		\
		"mul	edx"	  			\
		"div	ecx"				\
		parm	[eax] [edx] [ecx]	\
		modify	[edx eax]			\
		value	[eax]
#else
#ifdef __MSVC__
inline ULong MULDIVUNS (ULong num1, ULong num2, ULong num3)
{
    _asm
    {
		push	edx
		push	ecx
		mov		eax,num1
		mov		edx,num2
		mov		ecx,num3
		mul		edx
		div		ecx
		pop		ecx
		pop		edx
    }
}
#elif defined(__GNUC__)
inline ULong MULDIVUNS(ULong num1, ULong num2, ULong num3)
{
	uint64_t product = (uint64_t)num1 * num2;
    return (ULong)(product / num3);
}
#endif
#endif

#ifdef __WATCOMC__
extern	SLong	MULDIVSIN (SLong,SLong,SLong);
#pragma	aux	MULDIVSIN		=		\
		"imul	edx"	  			\
		"idiv	ecx"				\
		parm	[eax] [edx] [ecx]	\
		modify	[edx eax]			\
		value	[eax]
#else
#ifdef __MSVC__
inline SLong MULDIVSIN (SLong num1,SLong num2,SLong num3)
{
    _asm
    {
		push	edx
		push	ecx
		mov		eax,num1
		mov		edx,num2
		mov		ecx,num3
		imul	edx
		idiv	ecx
		pop		ecx
		pop		edx
    }
}
#elif defined(__GNUC__)
inline SLong MULDIVSIN(SLong num1, SLong num2, SLong num3)
{
	int64_t product = (int64_t)num1 * num2;
    return (SLong)(product / num3);
}
#endif
#endif
////////////////////////////////////////////////////////////

#ifdef __WATCOMC__
extern	SWord	mathlib_w_getsign(SWord);
#pragma	aux	mathlib_w_getsign=	\
		"cwd"			\
		parm	[ax]	\
		modify	exact	[dx]	\
		value	[dx]
#else
#ifdef __MSVC__
inline SWord mathlib_w_getsign(SWord num)
{																	  //RDH 20/05/99
    _asm
    {
		mov		ax,num
		sar		ax,15
    }
}
#elif defined(__GNUC__)
inline SWord mathlib_w_getsign(SWord num)
{
    return num < 0 ? static_cast<SWord>(-1) : static_cast<SWord>(0);
}
#endif
#endif

#ifdef __WATCOMC__
extern	SWord	mathlib_w_applysign(SWord,SWord);
#pragma aux	mathlib_w_applysign=	\
		"xor	ax,dx"				\
		"sub	ax,dx"				\
		parm	[ax]	[dx]		\
		modify exact	[ax]		\
		value	[ax]
#else
#ifdef __MSVC__
inline SWord _fastcall mathlib_w_applysign(SWord num1,SWord num2)
{
    _asm
    {
		mov		ax,num1
		mov		dx,num2
		xor		ax,dx
		sub		ax,dx
    }
}
#elif defined(__GNUC__)
inline SWord mathlib_w_applysign(SWord num1, SWord num2)
{
    return num2 < 0 ? -num1 : num1;
}

#else
extern	SWord	mathlib_w_applysign(SWord,SWord);
#endif
#endif

#ifdef __WATCOMC__
extern	SLong	mathlib_l_getsign(SLong);
#pragma	aux	mathlib_l_getsign=	\
		"cdq"			\
		parm	[eax]	\
		modify	exact	[edx]	\
		value	[edx]
#else
#ifdef __MSVC__
inline SLong mathlib_l_getsign(SLong num)
{
	SLong	retval;
    _asm
    {
		mov		eax,num
		sar		eax,31
    }
}
#elif defined(__GNUC__)
inline SLong mathlib_l_getsign(SLong num)
{
    return num < 0 ? -1 : 0;
}
#endif
#endif

#ifdef __WATCOMC__
extern	SLong	mathlib_l_applysign(SLong,SLong);
#pragma aux	mathlib_l_applysign=	\
		"xor	eax,edx"			\
		"sub	eax,edx"			\
		parm	[eax]	[edx]		\
		modify exact	[eax]		\
		value	[eax]
#else
#ifdef __MSVC__
inline SLong __fastcall mathlib_l_applysign(SLong num1,SLong num2)
{
    _asm
    {
		mov		Eax,num1
		mov		Edx,num2
		xor		Eax,Edx
		sub		Eax,Edx
    }
}
#elif defined(__GNUC__)
inline SLong mathlib_l_applysign(SLong num1,SLong num2)
{
    return (num1 ^ num2) - num2;
}
#else
extern	SLong	mathlib_l_applysign(SLong,SLong);
#endif

#endif

#ifdef __WATCOMC__
#else
#ifdef __MSVC__
inline void SETPREC(int p)
{
	UWord v;
	_asm {
		fstcw v;
	};
		v &=0xf0ff;
		v|=p<<8;
	_asm {
		fldcw v;
	};
}
inline int GETPREC()
{
	UWord v;
	_asm {
		fstcw v;
	};
	return ((v&0x0f00)>>8);
}

inline UWord GETFPCW()
{
	UWord v;
	_asm {
		fstcw v;
	}
	return v;
}

inline void SETFPCW(UWord w)
{
	UWord v=w;
	_asm {
		fldcw v;
	}
}
#elif defined(__GNUC__)
#if defined(__i386__) || defined(__x86_64__)

// x87 implementation
static inline int GETPREC(void) {
    UWord v;
    __asm__ __volatile__("fstcw %0" : "=m"(v));
    return (v >> 8) & 3;
}

static inline void SETPREC(int p) {
    UWord v;
    __asm__ __volatile__("fstcw %0" : "=m"(v));
    v = (v & 0xF0FFu) | ((p & 3) << 8);
    __asm__ __volatile__("fldcw %0" : : "m"(v));
}

static inline UWord GETFPCW(void) {
    UWord v;
    __asm__ __volatile__("fstcw %0" : "=m"(v));
    return v;
}

static inline void SETFPCW(UWord w) {
    __asm__ __volatile__("fldcw %0" : : "m"(w));
}

#else

// Non-x86 fallback
static inline int GETPREC(void) { return 0; }
static inline void SETPREC(int p) { (void)p; }
static inline UWord GETFPCW(void) { return 0; }
static inline void SETFPCW(UWord w) { (void)w; }

#endif

#endif
#endif

#ifdef __WATCOMC__
extern	void repmovsd(void* s,void* d,int len);
#pragma aux repmovsd="rep movsd" parm [esi][edi][ecx]
#else
#ifdef __MSVC__
inline void repmovsd(void *s,void *d,int len)
{
    __asm
    {
		mov esi,s;
		mov edi,d;
		mov ecx,len;
		rep movsd;
    }
}
#elif defined(__GNUC__)
inline void repmovsd(const void* src, void* dst, std::size_t dwords)
{
    asm volatile(
        "rep movsd"
        : "+S"(src), "+D"(dst), "+c"(dwords)
        :
        : "memory"
    );
}
#endif
#endif
#pragma warning(default:4035)

//if included:
#endif
