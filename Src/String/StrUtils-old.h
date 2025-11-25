//**************************************************************************************************
//* FILE:		StrUtils.h
//*
//* DESCRIP:
//*
//*



#ifndef _StrUtils_H_
#define _StrUtils_H_


#include <sstream>
#include <string>
#include <functional>
#include <vector>
#include <locale>

#include <ctype.h>
#include <stdarg.h>



#ifdef WINDOWS
//#define string  basic_string
#endif

#include "..\other\VaPass.h"


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


//***************************************************************
//* String Conversion Routines
//***************************************************************

inline void str2wstr(std::wstring &wsTarget, const std::string &sSource)
{
	int nSrcLen = (int) (sSource.size() + 2);

	int nTrgtLen = (nSrcLen * sizeof(wchar_t));

	wchar_t* pTmp = (wchar_t *) calloc(nSrcLen + 1, sizeof(wchar_t));

	if (pTmp == NULL)
	{
		return;
	}

	memset(pTmp, 0, nTrgtLen);

	int nRetValue = 0;

#ifdef WINDOWS
	errno_t err = mbstowcs_s((size_t*) &nRetValue, (wchar_t*) pTmp, nTrgtLen, (const char*) sSource.c_str(), (size_t) nSrcLen);
#else
	nRetValue = (int) mbstowcs((wchar_t*) pTmp, (const char*) sSource.c_str(), (size_t) nSrcLen);
#endif

	wsTarget = pTmp;

	free(pTmp);
}


inline std::wstring str2wstr(const std::string &sStr)
{
	std::wstring wsTmp = L"";

	str2wstr(wsTmp, sStr);

	return wsTmp;
}


inline void wstr2str(std::string &sTarget, const std::wstring &wsSource)
{
	int nSrcLen = (int) (wsSource.size());

	int nTrgtLen = (int) (sTarget.size() + 2);

    char* pTmp = new char[nTrgtLen];

	memset(pTmp, 0, nTrgtLen);

	int nRetValue = 0;

#ifdef WINDOWS
	wcstombs_s((size_t*) &nRetValue, pTmp, (size_t) nTrgtLen, wsSource.c_str(), (size_t) nSrcLen);
#else
	wcstombs(pTmp, wsSource.c_str(), nLen);
#endif

	sTarget = pTmp;
}

inline std::string wstr2str(const std::wstring &wsStr)
{
	std::string sTmp = "";

	wstr2str(sTmp, wsStr);

	return sTmp;
}

#define s2ws(x) str2wstr(x)
#define ws2s(x) wstr2str(x)

inline std::string toLower(const std::string &sIn)
{
    std::string sOut = "";

	std::string::size_type nLen = sIn.length();

    for(std::string::size_type x = 0; x < nLen; ++x)
    {
        sOut += ((isalpha((int) (sIn[x])) != 0) ? tolower(sIn[x]) : sIn[x]);
    }

    return sOut;
}

inline std::string toUpper(const std::string &sIn)
{
    std::string sOut = "";

	std::string::size_type nLen = sIn.length();

    for(std::string::size_type x = 0; x < nLen; ++x)
    {
        sOut += ((isalpha((int) (sIn[x])) != 0) ? toupper(sIn[x]) : sIn[x]);
    }

    return sOut;
}

//#define toString(x)	tos(x)

//***************************************************************
//* tos, tows
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
//* wstring ws = tows(string("hello"));
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


#ifdef WINDOWS
template<typename t>
std::wstring tows(t v)
{
    std::wostringstream o;

    o << v;

    return o.str();
}

#ifndef TCHAR
inline std::wstring tows(const char *pValue) 
{
    if (pValue == NULL) 
	{
		return L"";
	}

    size_t nSrcLen = ::strlen(pValue);

	size_t nTrgtLen = nSrcLen + 2;

    wchar_t *pBuf = (wchar_t *) calloc(nTrgtLen, (sizeof(wchar_t)));

	if (pBuf == NULL)
	{
		return L"";
	}

	memset(pBuf, 0, (nTrgtLen * sizeof(wchar_t)));

	int nRetValue = 0;

#ifdef WINDOWS
	mbstowcs_s((size_t*) &nRetValue, pBuf, (size_t)nTrgtLen, pValue, (size_t) nSrcLen);
#else
	::mbstowcs(pBuf, pValue, nLen);
#endif

    std::wstring sOut(pBuf);

    free(pBuf);

    return sOut;
}
#else
inline std::wstring tows(const TCHAR *pValue) 
{
    if (pValue == NULL) 
	{
		return L"";
	}

    size_t nLen = ::strlen(pValue);

    wchar_t *pBuf = (wchar_t *) calloc((nLen + 2), (sizeof(wchar_t)));

	if (pBuf == NULL)
	{
		return L"";
	}

	memset(pBuf, 0, ((nLen + 1) * sizeof(wchar_t)));

    ::mbstowcs(pBuf, pValue, nLen);

    std::wstring sOut(pBuf);

    free(pBuf);

    return sOut;
}
#endif  //  TCHAR

inline std::wstring tows(const std::string &sValue)
{
    return tows(sValue.c_str());
}

template<typename t>
inline std::string tos(const int nValue)
{
    if (nValue == 0)
	{
        return "0";
	}

    std::string sTmp = "";
    std::string sOut = "";

    int nTmp = nValue;

    while (nTmp > 0)
    {
        sTmp += nTmp % 10 + 48;
        nTmp /= 10;
    }

    for (int i = 0; i < sTmp.length(); i++)
	{
        sOut += sTmp[sTmp.length() - i - 1];
	}

    return sOut;
}

#ifndef TCHAR
inline std::string tos(const char *pValue)
{
    if (pValue == NULL)
	{
        return "";
	}

	int nLen = (int) strlen(pValue);

    std::string sOut = "";

	if (nLen > 0)
	{
		sOut.assign(pValue, nLen);
	}

    return sOut;
}
#else
inline std::string tos(const TCHAR *value)
{
    if (pValue == NULL)
	{
        return "";
	}

	int nLen = strlen(pValue);

    std::string sOut = "";

	if (nLen > 0)
	{
		sOut.assign(pValue, nLen);
	}

    return sOut;
}
#endif  //  TCHAR


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
	::wcstombs(pBuf, pValue, nLen);
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

#endif  //  WINDOWS


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

inline std::string toHexStr(const unsigned char *pBuf, const int nLen)
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


#ifndef WINDOWS

inline std::string stripChar(const std::string &s, char c, bool stripLeading = true, bool stripTrailing = true)
{
	return stripItem(s, bind1st(std::equal_to<char>(), c), stripLeading, stripTrailing); 
}


inline std::wstring stripChar(const std::wstring &s, wchar_t c, bool stripLeading = true, bool stripTrailing = true)
{ 
	return stripItem(s, bind1st(std::equal_to<wchar_t>(), c), stripLeading, stripTrailing); 
}

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
