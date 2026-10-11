//------------------------------------------------------------------------------
//Filename       prof.h
//System         
//Author         Paul.   
//Date           Mon 20 Apr 1998
//Description    
//------------------------------------------------------------------------------
#ifndef	PROF_Included
#define	PROF_Included

#define	DEFAULT_PROF 0
#include "MONOTXT.H"
//To disable profiling code comment out the next line...
#define PROFILING_ON	"TRUE"

// RERUN: self-assert packing - DOSDEFS-era headers assumed ambient pack(1);
// guard so include order cannot change this layout across TUs (ODR).
#pragma pack(push,1)

struct _timerData{
 	ULong	ht,lt;
};

inline struct _timerData operator-(struct _timerData a,struct _timerData b)
{
	struct _timerData result;
#if defined (__MSVC__)
_asm{
	pushad
	mov	eax,a.lt;
	mov	edx,a.ht;
	sub eax,b.lt;
	sbb edx,b.ht;
	mov	ecx,1000;
	idiv ecx;
	mov result.lt,eax;
	mov result.ht,edx;
	popad
}
#elif defined (__GNUC__)
	__asm__ __volatile__ (
		"pushad\n\t"
		"movl %2, %%eax\n\t"
		"movl %3, %%edx\n\t"
		"subl %4, %%eax\n\t"
		"sbbl %5, %%edx\n\t"
		"movl $1000, %%ecx\n\t"
		"idivl %%ecx\n\t"
		"movl %%eax, %0\n\t"
		"movl %%edx, %1\n\t"
		"popad\n\t"
		: "=r"(result.lt), "=r"(result.ht)
		: "r"(a.lt), "r"(a.ht), "r"(b.lt), "r"(b.ht)
		: "%eax", "%ebx", "%ecx", "%edx"
	);
#endif
return result;
}

inline struct _timerData _makeAverage(struct _timerData a,ULong conut)
{
	struct _timerData result;
#if defined (__MSVC__)
	_asm{
	pushad
	mov eax,a.lt;
	mov	edx,a.ht;
	mov ecx,conut;
	idiv ecx
	mov result.lt,eax;
	mov result.ht,eax;
	popad
	}
#elif defined (__GNUC__)
	__asm__ __volatile__ (
		"pushad\n\t"
		"movl %2, %%eax\n\t"
		"movl %3, %%edx\n\t"
		"movl %4, %%ecx\n\t"
		"idivl %%ecx\n\t"
		"movl %%eax, %0\n\t"
		"movl %%edx, %1\n\t"
		"popad\n\t"
		: "=r"(result.lt), "=r"(result.ht)
		: "r"(a.lt), "r"(a.ht), "r"(conut)
		: "%eax", "%ebx", "%ecx", "%edx"
	);
#endif
	return result;
}

inline struct _timerData operator+(struct _timerData a,struct _timerData b){
struct _timerData result;
#if defined (__MSVC__)
_asm{
	pushad
	mov	eax,a.lt;
	mov	edx,a.ht;
	add eax,b.lt;
	adc edx,b.ht;
	mov result.lt,eax;
	mov result.ht,edx;
	popad
}
#elif defined (__GNUC__)
	__asm__ __volatile__ (
		"pushad\n\t"
		"movl %2, %%eax\n\t"
		"movl %3, %%edx\n\t"
		"addl %4, %%eax\n\t"
		"adcl %5, %%edx\n\t"
		"movl %%eax, %0\n\t"
		"movl %%edx, %1\n\t"
		"popad\n\t"
		: "=r"(result.lt), "=r"(result.ht)
		: "r"(a.lt), "r"(a.ht), "r"(b.lt), "r"(b.ht)
		: "%eax", "%ebx", "%ecx", "%edx"
	);
#endif
return result;
}

inline struct _timerData _readTimer(){
#ifdef PROFILING_ON
	_timerData td;
#if defined (__MSVC__)
	_asm{
		pushad
		_emit	15
		_emit	49
		mov		td.lt,eax
		mov		td.ht,edx
		popad
	}
#elif defined (__GNUC__)
	__asm__ __volatile__ (
		"pushad\n\t"
		".byte 15,49\n\t"
		"movl %%eax, %0\n\t"
		"movl %%edx, %1\n\t"
		"popad\n\t"
		: "=r"(td.lt), "=r"(td.ht)
		:
		: "%eax", "%ebx", "%ecx", "%edx"
	);
#endif
	return td;
#endif
}

#define START_TIMER(p1)\
	struct _timerData td##p1;\
	td##p1=_readTimer();

#define STOP_TIMER(lno,p1)\
{	struct _timerData _tmpTime;\
	UByte _textOut[]=#p1;\
	_tmpTime=_readTimer();\
	_tmpTime=_tmpTime-td##p1;\
	SLong outVal=(SLong)_tmpTime.lt;\
	Mono_Text.PrintAt(0,(lno),outVal,_textOut);\
}

#define ADD_TIMER(p1,p2)\
{	struct _timerData _tmpTime;\
	_tmpTime=_readTimer();\
	_tmpTime=_tmpTime-td##p1;\
	##p2=##p2+_tmpTime;\
}

#define PRINT_TIMER(lno,p1)\
{	UByte _textOut[]=#p1;\
	SLong outVal=(SLong)##p1.lt;\
	Mono_Text.PrintAt(0,(lno),outVal,_textOut);\
}

#define PRINT_TIMER2(lno,p1)\
{	UByte _textOut[]=" ";\
	SLong outVal=(SLong)##p1.lt;\
	Mono_Text.PrintAt(0,(lno),outVal,_textOut);\
}

#pragma pack(pop)
#endif
