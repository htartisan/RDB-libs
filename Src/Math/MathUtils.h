//**************************************************************************************************
//* FILE:		MathUtils.h
//*
//* DESCRIP:	C++ math utility functions
//*
//* AUTHOR:		Written by Russ Barker
//*



#ifndef _MathUtils_H_
#define _MathUtils_H_


#include <cmath>



double createFloatWithIntegerAndFraction
    (
        const int32_t integerPart, 
        const int32_t fractionalPart, 
        const int32_t fractionalDigits 
    );
	
	
#endif  //  _MathUtils_H_
