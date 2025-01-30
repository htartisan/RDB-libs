//**************************************************************************************************
//* FILE:		win32_unix.cpp
//*
//* DESCRIP:	
//*
//*



#include "win32_unix.h"


#ifndef WINDOWS

#include <unistd.h>
#include <sys/time.h>
#include <string>
#include <stdexcept>
#include <map>
#include <fstream>

#include "stringUtils.h"


using namespace std;


void Sleep(int s) throw()
{
    ::sleep(s / 1000);
}

int timeGetTime() throw()
{
    struct timeval tp;
    ::gettimeofday(&tp,NULL);
    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}


typedef map<string,string> keyMap_t;
typedef map<string, keyMap_t> iniFile_t;
static string gCurrentINIFileName;
static iniFile_t	gCurrentINIFile;


static void loadNewINIFile(const char *fname) throw()
{
    gCurrentINIFileName = fname;
    gCurrentINIFile.clear();
    ifstream f(fname);
    string currentSection;

    while (f)
    {
        string s;
        getline(f,s);
        s = stringUtil::stripWhitespace(s);
        if (!s.empty())
        {
            if (s[0] == '[' && s.size() >= 3)
            {
                currentSection = s.substr(1,s.size() - 2);
            }
            else if (s[0] == ';' || s[0] == '#')
            {}
            else
            {
                string::size_type pos = s.find('=');
                if (pos != string::npos)
                {
                    string key = stringUtil::stripWhitespace(s.substr(0,pos));
                    string val = stringUtil::stripWhitespace(s.substr(pos+1));
                    gCurrentINIFile[currentSection][key] = val;
                }
            }
        }
    }
}

static DWORD privateProfileStringCopy(char *retVal,DWORD siz,const char *val)
{
    strncpy(retVal,val,siz);
    return min((int)siz,(int)strlen(val));
}

DWORD GetPrivateProfileString
    (
        LPCTSTR lpAppName,
        LPCTSTR lpKeyName,
        LPCTSTR lpDefault,
        LPTSTR lpReturnedString,
        DWORD nSize,
        LPCTSTR lpFileName
    ) throw()
{
    if (lpFileName != gCurrentINIFileName)
        loadNewINIFile(lpFileName);

    iniFile_t::const_iterator ini_i = gCurrentINIFile.find(lpAppName);
    if (ini_i == gCurrentINIFile.end())
        return privateProfileStringCopy(lpReturnedString,nSize,lpDefault);

    keyMap_t::const_iterator keym_i = (*ini_i).second.find(lpKeyName);
    if (keym_i == (*ini_i).second.end())
        return privateProfileStringCopy(lpReturnedString,nSize,lpDefault);

    return privateProfileStringCopy(lpReturnedString,nSize,(*keym_i).second.c_str());
}


UINT GetPrivateProfileInt
    (
        LPCTSTR lpAppName,
        LPCTSTR lpKeyName,
        INT nDefault,
        LPCTSTR lpFileName
    ) throw()
{
    if (lpFileName != gCurrentINIFileName)
        loadNewINIFile(lpFileName);

    iniFile_t::const_iterator ini_i = gCurrentINIFile.find(lpAppName);
    if (ini_i == gCurrentINIFile.end())
        return nDefault;

    keyMap_t::const_iterator keym_i = (*ini_i).second.find(lpKeyName);
    if (keym_i == (*ini_i).second.end())
        return nDefault;

    return atoi((*keym_i).second.c_str());
}


#endif
