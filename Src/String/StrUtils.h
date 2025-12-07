//**************************************************************************************************
//* FILE:		StrUtils.h
//*
//* DESCRIP:	C++ string utility functions
//*
//* AUTHOR:		Written by Russ Barker
//*


#define _CRT_SECURE_NO_WARNINGS


#ifndef _StrUtils_H_
#define _StrUtils_H_

#include <sstream>
#include <string>
#include <functional>
#include <vector>
#include <locale>

#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <cstdarg>

#include <codecvt>


#ifdef WINDOWS
//#define string  basic_string
#endif


namespace StrUtils
{

#ifdef WINDOWS

inline std::string eol() throw()
{
    return "\r\n";
}

#elif MACINTOSH
inline std::string eol() throw()
{
    return "\r";
}

#else

inline std::string eol() throw()
{
    return "\n";
}

#endif


inline std::string strPrintf(const char *pFmt, ...)
{
	char szTmp[2048];

	memset(szTmp, 0, sizeof(szTmp));

	va_list args;
	va_start(args, pFmt); // Initialize va_list

#ifdef WINDOWS
	::sprintf_s(szTmp, sizeof(szTmp), pFmt, args);
#else
	::sprintf(szTmp, pFmt, args);
#endif

	va_end(args); // Clean up va_list

	std::string sOut(szTmp);

	return sOut;
}


inline std::string tab() throw()
{
    return "\t";
}


//std::string get_current_path()
//{
//	char *pTmp = _getcwd(NULL, 0);
//
//	std::string sRet = "";
//
//	if (pTmp != NULL)
//	{
//		sRet = pTmp;
//	}
//
//	return sRet;
//}


inline bool isNumeric(const std::string &sIn)
{
	std::string::size_type nLen = sIn.length();

    for(std::string::size_type x = 0; x < nLen; ++x)
    {
        if (isdigit((int) (sIn[x])) == 0)
		{
			return false;
		}
    }

    return true;
}

inline std::string toLower(const std::string &sIn, const bool bFilterNonPrintable = true)
{
	std::string sOut = "";

	std::string::size_type nLen = sIn.length();

	for (std::string::size_type x = 0; x < nLen; ++x)
	{
		int ch = sIn[x];

		// check for any chars that need to be filtered out

		if (ch < 0)
		{
			continue;
		}

		if ((bFilterNonPrintable == true) && (std::isprint(ch) == 0))
		{
			continue;
		}

		// convert to lower case

		if (std::isalpha(ch) != 0)
		{
			sOut += (char) tolower(ch);
		}
		else
		{
			sOut += (char) ch;
		}
	}

	return sOut;
}


inline std::string toUpper(const std::string& sIn, const bool bFilterNonPrintable = true)
{
	std::string sOut = "";

	std::string::size_type nLen = sIn.length();

	for (std::string::size_type x = 0; x < nLen; ++x)
	{
		int ch = sIn[x];

		// check for any chars that need to be filtered out

		if (ch < 0)
		{
			continue;
		}

		if ((bFilterNonPrintable == true) && (std::isprint(ch) == 0))
		{
			continue;
		}

		// convert to upper case

		if (std::isalpha(ch) != 0)
		{
			sOut += (char) toupper(ch);
		}
		else
		{
			sOut += (char) ch;
		}
	}

	return sOut;
}


inline std::string removeAllSpaces(const std::string& sIn, bool bRemoveTabs = true) 
{
	std::string sOut = "";

	if (sIn == "")
	{
		return sOut;
	}

	int x = 0;

	for (auto c = sIn.begin(); c != sIn.end(); c++)
	{ 
		char ch = (*c);

		if (bRemoveTabs == false)
		{
			if (ch != ' ')
			{
				sOut.push_back(ch);
			}
		}
		else
		{
			// Checks for any whitespace character, including spaceand tab
			
			if (std::isspace((int) ch) == 0)
			{ 
				sOut.push_back(ch);
			}
		}
	}

	return sOut;
}


inline std::string removeLeadingSpaces(const std::string& sIn, bool bRemoveTabs = true)
{
	std::string sOut = "";

	if (sIn == "")
	{
		return sOut;
	}

	int x = 0;

	for (auto c = sIn.begin(); c != sIn.end(); c++)
	{
		char ch = (*c);

		if (bRemoveTabs == false)
		{
			// checks for a space char

			if (ch != ' ')
			{
				break;
			}
		}
		else
		{
			// Checks for any whitespace character, including spaceand tab

			if (std::isspace((int)ch) == 0)
			{
				break;
			}
		}

		x++;
	}

	sOut = sIn.substr(x);

	return sOut;
}


inline std::string removeTrailingSpaces(const std::string& sIn, bool bRemoveTabs = true)
{
	std::string sOut = "";

	if (sIn == "")
	{
		return sOut;
	}

	int x = 0;

	for (auto c = sIn.rbegin(); c != sIn.rend(); ++c)
	{
		char ch = (*c);

		if (bRemoveTabs == false)
		{
			// checks for a space char

			if (ch != ' ')
			{
				break;
			}
		}
		else
		{
			// Checks for any whitespace character, including spaceand tab

			if (std::isspace((int) ch) != 0)
			{
				break;
			}
		}

		x++;
	}

	auto len = (sIn.length() - x);

	sOut = sIn.substr(0, len);

	return sOut;
}


//#define toString(x)	tos(x)

//***************************************************************
//* tos
//*
//* These templates and overloaded functions allow you to convert
//* any streamable value to a string or wide-string. It can be a
//* great convenience to be able to do
//*
//* string("test five ") + tos(5);
//*
//* instead of
//*
//* ostringstream o;
//* o << "test five " << 5;
//* o.str()
//*
//* tows is just like tos, but for wide strings. You can also use these
//* to convert standard strings to wide strings and vice-versa
//*
//* for example:
//*
//* string s = tos(wstring(L"goodbye"));
//*
//*

template<typename t>
std::string tos(t v)
{
    std::ostringstream o;

    o << v;

    return o.str();
}

inline std::string tos(const wchar_t *pValue)
{
    if (pValue == NULL)
	{
        return "";
	}

    size_t nSrcLen = ::wcslen(pValue);

	size_t nTrgtLen = nSrcLen + 2;

    char *pBuf = (char *) calloc(nTrgtLen, sizeof(char));

	if (pBuf == NULL)
	{
        return "";
	}

	memset(pBuf, 0, (nTrgtLen));

	int nRetValue = 0;

#ifdef WINDOWS
	wcstombs_s((size_t*) &nRetValue, pBuf, (size_t) nTrgtLen, pValue, (size_t) nSrcLen);
#else
	::wcstombs(pBuf, pValue, nSrcLen);
#endif  

    std::string sOut(pBuf);

    free(pBuf);

    return sOut;
}


inline std::string tos(const std::wstring &sIn)
{
    return tos(sIn.c_str());
}


inline std::string tos(bool bVal, bool bCaps = false)
{
    std::string sOut = "";

	if (bVal == true)
	{
		if (bCaps == true)
		{
			sOut = "TRUE";
		}
		else
		{
			sOut = "true";
		}
	}
	else
	{
		if (bCaps == true)
		{
			sOut = "FALSE";
		}
		else
		{
			sOut = "false";
		}
	}

    return sOut;
}


inline std::wstring tows(const std::string& sIn)
{
	std::wstring sOut(sIn.begin(), sIn.end());

	return sOut;
}



inline std::string wstos(const std::wstring& sIn)
{
#if 0

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

	return converter.to_bytes(sIn);

#else

	size_t bufSize = std::wcstombs(nullptr, sIn.c_str(), 0) + 1;

	if (bufSize < 1)
	{
		return "";
	}

	// Create a buffer to hold the multibyte string
	std::vector<char> buffer(bufSize);

	// Convert the wstring to a multibyte string
	std::wcstombs(buffer.data(), sIn.c_str(), bufSize);

	// Construct std::string from the char buffer
	return std::string(buffer.data());

#endif
}



inline bool toBool(const std::string &sVal)
{
	//* if string is blank/empty, return false
	if (sVal == "")
	{
		return false;
	}

	std::string sCmpVal = toLower(sVal);

	//* check for assorted string values for "true"
	if 
		(
			(sCmpVal == "true") ||
			(sCmpVal == "yes") ||
			(sCmpVal == "y") ||
			(sVal == "1") 
		)
	{
		return true;
	}

	//* check for assorted string values for "false"
	if 
		(
			(sCmpVal == "false") ||
			(sCmpVal == "no") ||
			(sCmpVal == "n") ||
			(sVal == "0") 
		)
	{
		return false;
	}

	//* bad value... throw an error
	throw std::runtime_error("toBool - ERROR: Invalid input string value");
}


enum eStringCompareResult
{
	eStringCompareResult_lower = -1,
	eStringCompareResult_equal = 0,
	eStringCompareResult_higher = 1
};


inline int strCompare(const std::string sStr1, const std::string sStr2, const unsigned int nLen = 0)
{
	unsigned int nCmpLen = nLen;

	if (nLen > 0)
	{
		if (nCmpLen > (unsigned int) sStr1.length())
		{
			nCmpLen = (unsigned int) sStr1.length();
		}
	}
	else
	{
		nCmpLen = (unsigned int) sStr1.length();
	}

	if (nCmpLen > (unsigned int) sStr2.length())
	{
		nCmpLen = (unsigned int) sStr2.length();
	}

	for (unsigned int x = 0; x < nCmpLen; x++)
	{
		if (sStr1[x] < sStr2[x])
		{
			return eStringCompareResult_lower;
		}

		if (sStr1[x] > sStr2[x])
		{
			return eStringCompareResult_higher;
		}
	}

	return eStringCompareResult_equal;
}

inline int findInString(const std::string sStr1, const std::string sStr2, const bool bCaseSensative = false)
{
	int retValue = -1;

	if (bCaseSensative == false)
	{
		std::string sTestValue = StrUtils::toLower(sStr2);

		std::string sTestString = StrUtils::toLower(sStr1);

		auto foundAt = sTestString.find(sTestValue);

		if (foundAt != std::string::npos)
		{
			retValue = (int) foundAt;
		}
	}
	else
	{
		auto foundAt = sStr1.find(sStr2);

		if (foundAt != std::string::npos)
		{
			retValue = (int) foundAt;
		}
	}

	return retValue;
}

#ifndef _TOHEXSTR_
#define _TOHEXSTR_

const char g_HexChars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

inline std::string toHexStr(const char chValue)
{
	try
	{
		std::string sOut = "";

		unsigned char ucVal = (unsigned char) chValue;

		sOut += g_HexChars[ ( ucVal & 0xF0 ) >> 4 ];
		sOut += g_HexChars[ ( ucVal & 0x0F ) >> 0 ];

		return sOut;
	}
	catch(...)
	{
		
	}

	return "";
}

inline std::string toHexStr(const unsigned char *pBuf, const int nLen, bool bInsertSpaces = false)
{
	try
	{
		if (pBuf == NULL || nLen < 1)
		{
			return "";
		}

		std::string sOut = "";

		unsigned char ucVal;

		for (int x = 0; x < nLen; x++)
		{
			if (bInsertSpaces == true && x > 0)
			{
				sOut += ' ';
			}

			ucVal = (unsigned char) *(pBuf + x);

			sOut += g_HexChars[ ( ucVal & 0xF0 ) >> 4 ];
			sOut += g_HexChars[ ( ucVal & 0x0F ) ];
		}

		return sOut;
	}
	catch(...)
	{
		
	}

	return "";
}


inline bool toByteArray(unsigned char *pTarget, int nLen, const std::string sSrc)
{
	try
	{
		if (pTarget == NULL || nLen < 1)
		{
			//* invalid target param info
			return false;
		}

		int nSrcLen = (int) sSrc.length();

		if ((nSrcLen % 2) > 0 || (nSrcLen / 2) > nLen)
		{
			//* sSrc contains an odd number of hex chars
			//* or target buffer is too small
			return false;
		}

		char cSrc;

		unsigned char cTmp, cOut;

		for (int x = 0; x < nSrcLen; x++)
		{
			//* convert 1st 4 bits (hex char) to binary, then left shift

			cSrc = sSrc[x]; 

			if (cSrc >= '0' && cSrc <= '9')
			{
				cTmp = (unsigned char) (((int) cSrc) - ((int) '0')) << 4; 
			}
			else if (cSrc >= 'A' && cSrc <= 'F')
			{
				cTmp = (unsigned char) (((int) cSrc) - ((int) 'A')) << 4; 
			}
			else if (cSrc >= 'a' && cSrc <= 'f')
			{
				cTmp = (unsigned char) (((int) cSrc) - ((int) 'a')) << 4; 
			}
			else
			{
				return false;
			}

			x++;

			//* convert 2nd 4 bits (hex char) to binary, then OR with 1st 4bit value

			cSrc = sSrc[x]; 

			if (cSrc >= '0' && cSrc <= '9')
			{
				cOut = (unsigned char) (((int) cSrc) - ((int) '0')) | cTmp; 
			}
			else if (cSrc >= 'A' && cSrc <= 'F')
			{
				cOut = (unsigned char) (((int) cSrc) - ((int) 'A')) | cTmp; 
			}
			else if (cSrc >= 'a' && cSrc <= 'f')
			{
				cOut = (unsigned char) (((int) cSrc) - ((int) 'a')) | cTmp; 
			}
			else
			{
				return false;
			}

			*(pTarget + (x / 2)) = cOut;
		}
	}
	catch(...)
	{
		return false;
	}

	return true;
}


#else


inline std::string toHexStr(const char chValue);
inline std::string toHexStr(const unsigned char *pBuf, const int nLen);

inline bool toByteArray(unsigned char *pBuf, int nLen, const std::string sSrc);

#endif  //  _TOHEXSTR_


//******************************************************************
//* String stripping routines
//******************************************************************

//******************************************************************
//* A collection of templates and functions to allow stripping of
//* leading and trailing items from a string
//*
//* --- stripItem
//*
//* Strip leading and trailing items from a container
//* template parameters are the container type, and a function which returns
//* true if we have the value to strip
//*
//*
//* ---- stripChar
//* strips characters off a string by either a function (which returns
//* true if the character is found), or by an actual character mathc.
//* There are also forms for wide strings
//*
//* ---- stripWhitespace
//* removes leading and trailing spaces from a string using isspace()
//*

template <typename F>
inline std::string stripItem(const std::string &s, F func, bool stripLeading, bool stripTrailing)
{
    auto	leftit  = s.begin();
    auto	rightit = s.rbegin();

    if (s.length() < 1 )
	{
        return s;
	}

    if (stripLeading)
    {
        //while((leftit != s.end()) && F func (*leftit))
		while((leftit != s.end()) && func(*leftit))
		{
            leftit++;
		}
    }
    
	if (leftit == s.end()) 
	{
		//return S();
		return s;
	}

    if (stripTrailing)
    {
        while(rightit != s.rend() && func(*rightit))
		{
            rightit++;
		}
    }

    if (rightit == s.rend()) 
	{
		//return S();
		return s;
	}

    auto endpnt = (++rightit).base();

    if (leftit > endpnt)
	{
        return std::string();
	}

    return s.substr( (leftit - s.begin()) , (endpnt-leftit) +1);
}


template <typename S,typename F>
S stripItem(const S &s, F func, bool stripLeading = true, bool stripTrailing = true)
{
    auto leftit  = s.begin();
    auto rightit = s.rbegin();

    if (s.length() < 1 )
	{
        return s;
	}

    if (stripLeading)
    {
        while(leftit != s.end() && func(*leftit))
		{
            leftit++;
		}
    }

    if (leftit == s.end()) 
	{
		//return S();
        return s;
	}

    if (stripTrailing)
    {
        while(rightit != s.rend() && func(*rightit))
		{
            rightit++;
		}
    }

    if (rightit == s.rend()) 
	{
		//return S();
        return s;
	}

    typename S::const_iterator endpnt = (++rightit).base();

    if (leftit > endpnt)
	{
		//return S();
        return s;
	}

    return s.substr((leftit - s.begin()), ((endpnt - leftit) + 1));
}


#ifdef WINDOWS

//inline std::string stripChar(const std::string &s, char c, bool stripLeading = true, bool stripTrailing = true)
//{
//	return stripItem(s, bind1st(std::equal_to<char>(), c), stripLeading, stripTrailing); 
//}
//
//
//inline std::wstring stripChar(const std::wstring &s, wchar_t c, bool stripLeading = true, bool stripTrailing = true)
//{ 
//	return stripItem(s, bind1st(std::equal_to<wchar_t>(), c), stripLeading, stripTrailing); 
//}

#endif  //  WINDOWS


inline std::string stripWhitespace(const std::string &s)
{
    std::string result = "";

	if (s.length() < 1)
	{
        return result;
	}

    for (int i = 0; i < ((int) (s.length())); i++)
	{
		if (s[i] != ' ')
		{
			result += s[i];
		}
	}

    return result;
}


//*****************************************************************
//* tokenizer
//*
//* break up a string into substrings based on a delimiter item.
//*

template<typename S,typename F>
std::vector<S> tokenizer(const S &ins,F isdelimiter) throw()
{
    std::vector<S> result;
    S accum;

    for(typename S::const_iterator i = ins.begin(); i != ins.end(); ++i)
    {
        if (!isdelimiter(*i))
        {
            accum += (*i);
        }
        else
        {
            if (!accum.empty())
            {
                result.push_back(accum);
                accum = S();
            }
        }
    }

    if (!accum.empty())
	{
        result.push_back(accum);
	}

    return result;
}


};

#endif  //  _StrUtils_H_
