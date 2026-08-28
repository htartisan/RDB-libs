//**************************************************************************************************
//* FILE:		RapidJsonUtils.cpp
//*
//* DESCRIP:	
//*
//*


#ifdef WIN32
#include <windows.h>
#include <cstring>
#else
#include "win32_unix.h"
#endif

#include <stdio.h>
#include <sys/types.h>

#include <string>
#include <iostream>       // std::cerr
//#include <typeinfo>       // operator typeid
#include <exception> 

#include "CFileIO.h"

#include "stringUtils.h"
//#include "urlUtils.h"

#include "RapidJsonUtils.h"

//using namespace std;
//using namespace stringUtil;



class JsonParserHandler 
{
public:
	bool Null() 
	{ 
		m_eState = ParsingState_HasNull;
		m_val.SetNull(); 
		return true; 
	}
	
	bool Bool(bool b) 
	{ 
		m_eState = ParsingState_HasBool;
		m_val.SetBool(b);
		return true; 
	}
	
	bool Int(int i) 
	{ 
		m_eState = ParsingState_HasInt;
		m_val.SetInt(i);
		return true;
	}
	
	bool Uint(unsigned int i)
	{
		m_eState = ParsingState_HasUInt;
		m_val.SetUint(i);
		return true;
	}
	
	bool Int64(int64_t l)
	{
		m_eState = ParsingState_HasInt64;
		m_val.SetInt64(l);
		return true;
	}

	bool Uint64(uint64_t l)
	{
		m_eState = ParsingState_HasUInt64;
		m_val.SetUint64(l);
		return true;
	}

	bool Float(float f)
	{
		m_eState = ParsingState_HasFloat;
		m_val.SetFloat(f);
		return true;
	}

	bool Double(double d)
	{
		m_eState = ParsingState_HasDouble;
		m_val.SetDouble(d);
		return true;
	}

	bool RawNumber(const char*, rapidjson::SizeType, bool) 
	{ 
		return false; 
	}
	
	bool String(const char* pStr, rapidjson::SizeType nLen, bool) 
	{ 
		m_eState = ParsingState_HasString;
		m_val.SetString(pStr, nLen);
		return true; 
	}
	
	bool StartObject() 
	{ 
		m_eState = ParsingState_EnteringObject;
		return true;
	}
	
	bool Key(const char* pStr, rapidjson::SizeType nLen, bool)
	{ 
		m_eState = ParsingState_HasKey;
		m_val.SetString(pStr, nLen); 
		return true; 
	}
	
	bool EndObject(rapidjson::SizeType)
	{ 
		m_eState = ParsingState_ExitingObject;
		return true; 
	}
	
	bool StartArray() 
	{ 
		m_eState = ParsingState_EnteringArray;
		return true; 
	}
	
	bool EndArray(rapidjson::SizeType)
	{ 
		m_eState = ParsingState_ExitingArray;
		return true; 
	}

protected:
	
	JsonParserHandler(char* str) : 
		m_val(), 
		m_eState(ParsingState_Init),
		m_reader(), 
		m_ss(str) 
	{
		m_reader.IterativeParseInit();

		ParseNext();
	}
	
	void ParseNext()
	{
		if (m_reader.HasParseError())
		{
			m_eState = ParsingState_Error;

			return;
		}

		m_reader.IterativeParseNext<parseFlags>(m_ss, *this);
	}

protected:
	enum JsonParsingState 
	{
		ParsingState_Unknown,
		ParsingState_Init,
		ParsingState_Parsing,
		ParsingState_Error,
		ParsingState_HasNull,
		ParsingState_HasBool,
		ParsingState_HasInt,
		ParsingState_HasInt64,
		ParsingState_HasUInt,
		ParsingState_HasUInt64,
		ParsingState_HasFloat,
		ParsingState_HasDouble,
		ParsingState_HasRaw,
		ParsingState_HasString,
		ParsingState_HasKey,
		ParsingState_EnteringObject,
		ParsingState_ExitingObject,
		ParsingState_EnteringArray,
		ParsingState_ExitingArray
	};

	JsonParsingState	m_eState;
	
	rapidjson::Value	m_val;

	rapidjson::Reader	m_reader;
	
	rapidjson::InsituStringStream m_ss;

	static const int parseFlags = (rapidjson::kParseDefaultFlags | rapidjson::kParseInsituFlag);
};


void CRapidJsonParserUtil::clsInit()
{
	m_pJsonBuffer = NULL;

	m_lJsonBufLen = 0;

	m_pBaseNode = NULL;
	m_pLastNode = NULL;

	m_pInFile =	0;

	m_pFileReader = 0;

	pFileReadBuf = 0;

	m_bJsonLoadedFromFile = false;
	m_bJsonLoadedFromBuf = false;

	//m_sJsonDumpFile = DEFAULT_JSON_DUMP_FILE;

	//m_bDumpJsonOnError = false;

	try
	{
		m_pJsonParser = new JSON_PARSER;

		//m_iIT = m_pJsonParser->MemberEnd();
	}
	catch(...)
	{
		m_pJsonParser = NULL;
	}
}

void CRapidJsonParserUtil::clsDeInit()
{
	FreeJsonBuffer();

	try
	{
		if (m_pJsonParser != NULL)
		{
			delete m_pJsonParser;

			m_pJsonParser = NULL;
		}
	}
	catch(...)
	{
	
	};
}


#if 0
void CRapidJsonParserUtil::SetJsonBuffer(char *pBuffer)
{
	if (m_pJsonBuffer != NULL)
	{
		free(m_pJsonBuffer);

		m_pJsonBuffer = NULL;
	}

	m_pJsonBuffer = pBuffer;

	m_bJsonLoadedFromFile = false;
}
#endif

bool CRapidJsonParserUtil::Parse()
{
	if (m_pJsonParser == NULL)
	{
		return false;
	}

	try
	{
		if (m_pInFile != 0 && m_pFileReader != 0)
		{
			m_pBaseNode = &(m_pJsonParser->ParseStream(*m_pFileReader));

			if (m_pJsonParser->HasParseError() == true)
			{
				return false;
			}

		}
		else
		{
			if (m_pJsonBuffer == 0)
			{
				return false;
			}

			m_pBaseNode = &(m_pJsonParser->Parse(m_pJsonBuffer));

			if (m_pJsonParser->HasParseError() == true)
			{
				return false;
			}
		}

//#if 1
//		m_pBaseNode = m_pJsonParser;
//#else
//		//if (m_pJsonParser->HasMember() == true)
//		{
//			auto iCurr = m_pJsonParser->MemberBegin();
//
//			if (iCurr != m_pJsonParser->MemberEnd())
//			{
//				m_pBaseNode = (JSON_OBJECT_PTR) &(*iCurr);
//			}
//		}
//#endif
		return true;
	}
	catch (...)
	{

	}

	return false;
}


bool CRapidJsonParserUtil::AllocJsonBuffer(int nLen)
{
	if (nLen < 3)
	{
		return false;
	}

	try
	{
		if (m_pJsonBuffer != NULL)
		{
			CRapidJsonParserUtil::FreeJsonBuffer();
		}

		m_pJsonBuffer = (char *) calloc((nLen + 2), 1);
		if (m_pJsonBuffer == NULL)
		{
			return false;
		}

		return true;
	}
	catch(...)
	{

	}

	return false;
}



#if 0
bool CRapidJsonParserUtil::WriteJsonFile(char *pFolder, char *pFileName) throw(std::runtime_error)
{
	if (m_pJsonParser == NULL)
	{
		return false;
	}

	if (m_pJsonBuffer == NULL)
	{
		// if the buffer is currently NULL, then we don't have anything to write to the file

		//throw std::runtime_error("Invalid param");
		return false;
	}

	long lJsonLen = (long) strlen(m_pJsonBuffer);
	if (lJsonLen < 1)
	{
		//throw std::runtime_error("JSON buffer length < 1");
		return false;
	}

	if (pFileName == NULL)
	{
		//throw std::runtime_error("Invalid param");
		return false;
	}

	bool bStatus = false;




	}
	catch(...)
	{
		bStatus = false;
	}

	cFileManager.Close();

	return ((bStatus == TRUE) ? true : false);
}
#endif

#if 0
bool CRapidJsonParserUtil::DumpJsonBuffer(char *pFolder, char *pFileName, bool bSpaceFill) throw(std::runtime_error)
{
	if (m_pJsonBuffer == NULL)
	{
		// if the buffer is currently NULL, then we don't have anything to write to the file

		return false;
	}

	bool bStatus = false;

	//* open the JSON file and load the JSON into a buffer

	CFileIO		cFileManager;

	try
	{
		if (m_lJsonBufLen < 1)
		{
			long lBufLen = (long) strlen(m_pJsonBuffer);
			if (lBufLen < 1)
			{
				return false;
			}

			m_lJsonBufLen = lBufLen;

			bSpaceFill = false;
		}

		std::string sDumpFile = "";

		if (pFileName != NULL)
		{
			sDumpFile.append(pFileName);
		}
		else
		{
			sDumpFile.append(m_sJsonDumpFile);
		}

		if (pFolder != NULL)
		{
#ifdef UNICODE
			std::wstring sTmp = stringUtil::str2wstr(pFolder);
			bStatus = cFileManager.SetFileDir((WCHAR *) sTmp.c_str());
#else
			bStatus = cFileManager.SetFileDir((char *) pFolder);
#endif
			if (bStatus == false)
			{
				return false;
			}
		}

#ifdef UNICODE
		std::wstring sTmp = stringUtil::str2wstr(sDumpFile);
		bStatus = cFileManager.SetFileName((WCHAR *) sTmp.c_str());
#else
		bStatus = cFileManager.SetFileName((char *) sDumpFile.c_str());
#endif
		if (bStatus == false)
		{
			return false;
		}

		//* open file for "w" (write only) mode
#ifdef UNICODE
		cFileManager.SetFileMode(L"w");
#else
		cFileManager.SetFileMode("w");
#endif

		bStatus = cFileManager.Open();
		if (bStatus == false)
		{
			return false;
		}

		if (bSpaceFill == true)
		{
			char *pTemp = (char *) calloc((m_lJsonBufLen + 2), 1);
			if (pTemp != NULL)
			{
				char *pC1 = m_pJsonBuffer;
				char *pC2 = pTemp;

				for (long x = 0; x < m_lJsonBufLen; x++)
				{
					if (*pC1 == '\n' || *pC1 == '\r' || *pC1 == '\t' || *pC1 == '\v' || *pC1 == '\a' || *pC1 == '\f' || *pC1 == 0)
					{
						*pC2 = ' ';
					}
					else
					{
						*pC2 = *pC1;
					}

					pC1++;
					pC2++;
				}

#ifdef UNICODE
				{
					std::wstring sTmp = stringUtil::str2wstr(pTemp);
					bStatus = cFileManager.WriteBuffer((WCHAR *) sTmp.c_str(), (LONG) sTmp.length());
				}
#else
				bStatus = cFileManager.WriteBuffer(pTemp, (LONG) m_lJsonBufLen);
#endif
			}
			else
			{
#ifdef UNICODE
				std::wstring sTmp = stringUtil::str2wstr(m_pJsonBuffer, m_lJsonBufLen);
				bStatus = cFileManager.WriteBuffer((WCHAR *) sTmp.c_str(), (LONG) sTmp.length());
#else
				bStatus = cFileManager.WriteBuffer((char *) m_pJsonBuffer, (LONG) m_lJsonBufLen);
#endif
			}
		}
		else
		{
#ifdef UNICODE
			std::wstring sTmp = stringUtil::str2wstr(m_pJsonBuffer, m_lJsonBufLen);
			bStatus = cFileManager.WriteBuffer((WCHAR *)sTmp.c_str(), (LONG) sTmp.length());
#else
			bStatus = cFileManager.WriteBuffer((char *) m_pJsonBuffer, (LONG) m_lJsonBufLen);
#endif
		}
	}
	catch(...)
	{

	}

	cFileManager.Close();

	return bStatus;
}
#endif

//long CRapidJsonParserUtil::GetJsonLength()
//{
//	if (m_pJsonParser == NULL)
//	{
//		return -1;
//	}
//
//	m_pJsonParser->document.last_node()
//}

