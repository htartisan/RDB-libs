//****************************************************************************
// FILE:    UrlUtils.cpp
//
// DESC:    C++ URL definitions
//
// AUTHOR:  Russ Barker
//

#include <set>
#include <ctype.h>


#include "urlUtils.h"
#include <cstring>

#ifdef WIN32

//#include <windows.h>

#include <crtdbg.h>

#else

#include <assert.h>
#define ASSERT assert
#define _ASSERTE assert

#endif

#include <time.h>


using namespace std;

static std::string gExcluded = " <>#%\"{}|\\^[]`";
static std::string gReserved = ";/?:@&=+$,";
static std::string gMark = "-_.!~*'()";
static std::string gDigit = "0123456789";
static std::string gLowAlpha="abcdefghijklmnopqrstuvwxyz";
static std::string gHiAlpha ="ABCDEFGHIJKLMNOPQRSTUVWXYZ";

template<class T>
static set<T> setunion(const set<T> &s1, const set<T> &s2)
{
    set<T> result(s1);
    result.insert(s2.begin(), s2.end());
    return result;
}

static set<char> setFromString(const string &st)
{
    return set<char>(st.begin(), st.end());
}

static set<char> gSetExcluded(gExcluded.begin(), gExcluded.end());
static set<char> gSetReserved(gReserved.begin(), gReserved.end());
static set<char> gSetExcludedAndReserved(setunion(gSetExcluded, gSetReserved));
static set<char> gSetMark(gMark.begin(), gMark.end());
static set<char> gSetLowAlpha(gLowAlpha.begin(), gLowAlpha.end());
static set<char> gSetHiAlpha(gHiAlpha.begin(), gHiAlpha.end());
static set<char> gSetDigit(gDigit.begin(), gDigit.end());
static set<char> gSetAlpha(setunion(gSetLowAlpha, gSetHiAlpha));
static set<char> gSetAlphanum(setunion(gSetAlpha, gSetDigit));
static set<char> gSetUnreserved(setunion(gSetAlphanum, gSetMark));

////////////////////////////////////////////////
////////// Escaping Funcs //////////////////////
///////////////////////////////////////////////

// convert a character to its hex representation
static string escapeChar(char c) throw()
{
    char buf[10];
    sprintf(buf,"%%%02X",(int)c);
    _ASSERTE(strlen(buf) == 3);
    return buf;
}

// convert the char if it's in the set
static string escapeIfInSet(char c, const set<char> &st) throw()
{
    return (st.find(c) == st.end() ? string(1,c) : escapeChar(c));
}

static string escapeIfNotInSet(char c, const set<char> &st) throw()
{
    return (st.find(c) == st.end() ? escapeChar(c) : string(1, c));
}

template<class ESC>
static string escapeString(const std::string &s, const set<char> &st, ESC func) throw()
{
    std::string result;

    std::string::size_type siz = s.size();

    for (std::string::size_type x = 0; x < siz; ++x)
    {
        result += func(s[x], st);
    }

    return result;
}

string urlUtils::escape(const std::string &s) throw()
{
    return escapeString(s, gSetExcludedAndReserved, escapeIfInSet);
}


/////////////////////////////////////////////////////
//////////// Unescaping funcs //////////////////////
////////////////////////////////////////////////////

// convert %xx to a character
static char unescapeSequence(const std::string &s) throw()
{
    _ASSERTE(s.size() == 3);
    _ASSERTE(s[0] == '%' && isxdigit(s[1]) && isxdigit(s[2]));

    int v;
    
	sscanf(s.c_str(), "%%%02x", &v);
    
	return (char)v;
}

string urlUtils::unescapeString(const std::string &s) throw()
{
    std::string result;
    std::string escTok;
    
	int ccnt(0);

    for(string::const_iterator i = s.begin(); i != s.end(); ++i)
    {
        bool escChar(false);
        switch(ccnt)
        {
        case 0:
            escChar = ((*i) == '%');
            break;
        case 1:
            escChar = (isxdigit(*i) ? true : false);
            break;
        case 2:
            escChar = (isxdigit(*i) ? true : false);
            break;
        }

        if (escChar)
        {
            escTok += (*i);
            ccnt += 1;
        }
        else
        {
            result += escTok;
            ccnt = 0;
            result += (*i);
        }
        if (ccnt == 3)
        {
            result += unescapeSequence(escTok);
            escTok = "";
            ccnt = 0;
        }
    }

    result += escTok;
    
	return result;
}



////////////////////////////////////////////
///////// HTTP functions ///////////////////
////////////////////////////////////////////

bool urlUtils::urlManager::get(std::string &reply) throw()
{
	if (m_scheme != "http")
	{
		return false;
	}

	//* HTTP GET 

	m_retryCntr = 0;
	m_respHeaders.clear();
	reply.clear();

	do
	{
		try
		{
			bool bGotContentLen = false;

			std::string sHost = m_authority.getHost();
			std::string sPort = stringUtil::tos(m_authority.getPort());

			m_client.SetHost(sHost, sPort);
			m_client.SetPath(m_path);
			m_client.SetAdditionalDataToSend(NULL, 0);

			if (m_authority.useSec() == true)
			{
				m_client.SetServerLogin(m_authority.getUser(), m_authority.getPW());
			}

			// Set request headers.

			std::string sHeaders = "";

			sHeaders = m_reqHeaders.get();

			m_client.SetAdditionalRequestHeaders(sHeaders);
 
			if (m_authority.useSec() == true)
			{
				m_client.SetServerLogin(m_authority.getUser(), m_authority.getPW());
			}
 
			m_client.SetTimeouts(m_timeout * 100);

			if (m_client.Open() == false)
			{
				m_client.Close();
				m_retryCntr++;

				continue;
			}

			// Send HTTP get request.
			if (m_client.SendHttpRequest("GET", true) == false)
			{
				m_client.Close();
				m_retryCntr++;

				continue;
			}

			m_respHeaders = 
				m_client.GetResponseHeader();
			
			reply = 
				m_client.GetResponseContent();

			m_client.Close();

			return true;
		}
		catch (...)
		{
			m_client.Close();
			m_retryCntr++;
		}
	
	} while (m_retryCntr < m_maxRetries);

	return false;
}


bool urlUtils::urlManager::put(std::string &query) throw()
{
	if (m_scheme != "http")
	{
		return false;
	}

	//* HTTP PUT

	m_retryCntr = 0;
	m_respHeaders.clear();

	do
	{
		try
		{
			int len = query.length();

			std::string sHost = m_authority.getHost();
			std::string sPort = stringUtil::tos(m_authority.getPort());

			m_client.SetHost(sHost, sPort);
			m_client.SetPath(m_path);
  
			// Set request headers.

			std::string sHeaders = "";

			m_client.SetAdditionalDataToSend(
				len > 0 ? (BYTE *) query.data() : NULL,
				len);
			m_reqHeaders.add("Content-Length", stringUtil::tos(len));

			sHeaders = m_reqHeaders.get();

			m_client.SetAdditionalRequestHeaders(sHeaders);
 
			if (m_authority.useSec() == true)
			{
				m_client.SetServerLogin(m_authority.getUser(), m_authority.getPW());
			}
 
			m_client.SetTimeouts(m_timeout * 100);

			if (m_client.Open() == false)
			{
				m_client.Close();
				m_retryCntr++;

				continue;
			}

			// Send HTTP put request.
			if (m_client.SendHttpRequest("PUT", true) == false)
			{
				m_client.Close();
				m_retryCntr++;

				continue;
			}

			m_client.Close();

			m_respHeaders = 
				m_client.GetResponseHeader();

			return true;
		}
		catch (...)
		{
			m_client.Close();
			m_retryCntr++;
		}
	
	} while (m_retryCntr < m_maxRetries);

	return false;
}


bool urlUtils::urlManager::post(std::string &query, std::string &reply) throw()
{
	if (m_scheme != "http")
	{
		return false;
	}

	//* POST using HappyHTTP high-level request interface

	m_retryCntr = 0;
	m_respHeaders.clear();
	reply.clear();

	do
	{
		try
		{
			int len = query.length();

			std::string sHost = m_authority.getHost();
			std::string sPort = stringUtil::tos(m_authority.getPort());

			m_client.SetHost(sHost, sPort);
			m_client.SetPath(m_path);
  
			// Set request headers.

			std::string sHeaders = "";

			m_client.SetAdditionalDataToSend(
				len > 0 ? (BYTE *) query.data() : NULL,
				len);
			m_reqHeaders.add("Content-Length", stringUtil::tos(len));

			sHeaders = m_reqHeaders.get();

			m_client.SetAdditionalRequestHeaders(sHeaders);
 
			if (m_authority.useSec() == true)
			{
				m_client.SetServerLogin(m_authority.getUser(), m_authority.getPW());
			}
 
			m_client.SetTimeouts(m_timeout * 100);

			if (m_client.Open() == false)
			{
				m_client.Close();
				m_retryCntr++;

				continue;
			}

			// Send HTTP post request.
			if (m_client.SendHttpRequest("POST", true) == false)
			{
				m_client.Close();
				m_retryCntr++;

				continue;
			}

			m_client.Close();

			m_respHeaders = 
				m_client.GetResponseHeader();
			
			reply = 
				m_client.GetResponseContent();

			return true;
		}
		catch (...)
		{
			m_client.Close();
			m_retryCntr++;
		}
	
	} while (m_retryCntr < m_maxRetries);

	return false;
}



////////////////////////////////////////////
/////////// Classes ////////////////////////

string urlUtils::urlAuthority::escape() const throw()
{
    std::string result;

    //if (m_userInfo != "")
    //{
    //    set<char> validUserInfoChars(setunion(gSetUnreserved, setFromString(";:&=+$,")));
    //    result += escapeString(m_userInfo, validUserInfoChars, escapeIfNotInSet);
    //    result += "@";
    //}

    // host name and port cannot have escape sequences
    result += m_host;

    if (m_nPort != "")
    {
        result += ":";
        result += m_nPort;
    }

    return result;
}


string urlUtils::urlQueryEntry::escape() const throw()
{
    std::string result;

    result += escapeString(m_entry.first, gSetExcludedAndReserved, escapeIfInSet);
    if (m_entry.second != "")
    {
        result += "=";

        result += escapeString(m_entry.second, gSetExcludedAndReserved, escapeIfInSet);
    }

    return result;
}


urlUtils::urlQueryEntry urlUtils::urlQueryEntry::parse(const std::string &s) throw()
{
    urlUtils::urlQueryEntry result;

    std::string::size_type pos = s.find("=");
	if (pos != std::string::npos)
	{
		result.m_entry.first = unescapeString(s.substr(0, pos));

		result.m_entry.second = (pos == string::npos ? "" : unescapeString(s.substr(pos + 1)));
	}

    return result;
}


string urlUtils::urlQuery::escape() const throw()
{
    std::string result;

    for(vector<urlQueryEntry>::const_iterator i = m_query.begin(); i != m_query.end(); ++i)
    {
        result += (*i).escape();

        if (i+1 != m_query.end())
		{
            result += "&";
		}
    }

    return result;
}


urlUtils::urlQuery urlUtils::urlQuery::parse(const std::string &sin) throw()
{
    urlUtils::urlQuery result;

    std::string s(sin);

    while(s != "")
    {
        std::string::size_type pos = s.find("&");
        
		result.m_query.push_back(urlUtils::urlQueryEntry::parse(s.substr(0, pos)));
        
		if (pos == string::npos)
		{
            break;
		}

        s = s.substr(pos + 1);
    }

    return result;
}


string urlUtils::urlManager::escape() const throw()
{
    std::string result;

    result += m_scheme; // no escapes in scheme
    result += "://";
    result += m_authority.escape();

    // escape the path
    set<char> validPathChars(setunion(gSetUnreserved, setFromString("/;:@&=+$,")));
    result += escapeString(m_path, validPathChars, escapeIfNotInSet);

    if (!m_query.empty())
    {
        result += "?";
        result += m_query.escape();
    }

    return result;
}



std::string urlUtils::mimeEncode(const std::string sSrc) 
{
	std::string base64Chars = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	unsigned char *pEncode = (unsigned char *) sSrc.data();

	unsigned int nLen = sSrc.length();

	std::string ret;

	int i = 0;
	int x = 0;

	unsigned char array3[3];
	unsigned char array4[4];

	while (nLen--) 
	{
		array3[i++] = *(pEncode++);

		if (i == 3) 
		{
			array4[0] = (array3[0] & 0xfc) >> 2;
			array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
			array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
			array4[3] = array3[2] & 0x3f;

			for(i = 0; (i <4) ; i++)
			{
				ret += base64Chars[array4[i]];
			}

			i = 0;
		}
	}

	if (i)
	{
		for(x = i; x < 3; x++)
		{
			array3[x] = '\0';
		}

		array4[0] = (array3[0] & 0xfc) >> 2;
		array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
		array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
		array4[3] = array3[2] & 0x3f;

		for (x = 0; (x < i + 1); x++)
		{
			ret += base64Chars[array4[x]];
		}

		while((i++ < 3))
		{
			ret += '=';
		}

	}

	return ret;
}
