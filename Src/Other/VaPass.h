//**************************************************************************************************
//* FILE:		VaPass.h
//*
//* DESCRIP:
//*
//*


#ifndef _VA_PASS_
#define _VA_PASS_


#include <stdarg.h>


#pragma warning( disable : 4290 )

#ifndef byte
typedef unsigned char		byte;
#endif

#ifndef DWORD
typedef unsigned long       DWORD;
#endif


template<byte count>
struct SVaPassNext
{
    SVaPassNext<count-1> big;
    DWORD dw;
};

template<> struct SVaPassNext<0>{};
//SVaPassNext - is generator of structure of any size at compile time.

class CVaPassNext
{
public:
    SVaPassNext<50> svapassnext;
    
	CVaPassNext(va_list & args)
	{
		try	//to avoid access violation
		{
			memcpy(&svapassnext, args, sizeof(svapassnext));

		} catch (...) {}
    }
};

#define va_pass(valist) CVaPassNext(valist).svapassnext


#endif // _VA_PASS_

