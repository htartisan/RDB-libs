//**************************************************************************************************
//* FILE:		MathUtils.cpp
//*
//* DESCRIP:	C++ math utility functions
//*
//* AUTHOR:		Written by Russ Barker
//*


#include "MathUtils.h"


double createFloatWithIntegerAndFraction
    (
        const int32_t integerPart, 
        const int32_t fractionalPart, 
        const int32_t fractionalDigits 
    ) 
{
    // Convert the integer part to a double
    double result = static_cast<double>(integerPart);

    // Calculate the scaling factor for the fractional part
    // e.g., if fractionalDigits is 2, factor is 100.0
    double scalingFactor = std::pow(10.0, fractionalDigits);

    // Convert the fractional part to a double and scale it
    double scaledFractionalPart = static_cast<double>(fractionalPart) / scalingFactor;

    // Add the scaled fractional part to the result
    if (integerPart >= 0) 
    {
        result += scaledFractionalPart;
    }
    else {
        // Handle negative numbers correctly: subtract the absolute value of the fractional part
        result -= scaledFractionalPart;
    }

    return result;
}


