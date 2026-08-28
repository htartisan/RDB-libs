//**************************************************************************************************
//* FILE:		VaPass.h
//*
//* DESCRIP:
//*
//*


#define _CRT_SECURE_NO_WARNINGS


#ifndef _VA_PASS_
#define _VA_PASS_


#include <stdarg.h>
#include <cstdio>
#include <cstring>

#pragma warning( disable : 4290 )

#ifndef byte
typedef unsigned char		byte;
#endif

#if !defined(_WIN32) && !defined(WIN32) && !defined(DWORD) && !defined(DWORD_DEFINED)
#define DWORD_DEFINED
typedef unsigned int       DWORD;
#endif

#if !defined(_WIN32) && !defined(WIN32) && !defined(sprintf_s)
#define sprintf_s(buffer, ...) sprintf(buffer, __VA_ARGS__)
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
