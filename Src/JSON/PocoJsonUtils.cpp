//**************************************************************************************************
//* FILE:		PocoJsonUtils.cpp
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
#include "PocoJsonUtils.h"

//using namespace std;
//using namespace stringUtil;




void CPocoJsonParserUtil::InitCls()
{
	m_pJsonBuffer = NULL;

	m_parseResult.empty();

	m_lJsonBufLen = 0;

	m_pBaseNode = NULL;
	m_pItEntry = NULL;

	m_lastNode.m_eType = JSON_ObjectType_def::ObjectType_Root;
	m_lastNode.m_sName = "";
	m_lastNode.m_pParent = 0;
	m_lastNode.m_pObject = 0;
	m_lastNode.m_pArray = 0;

	m_pFileBuf = 0;
	m_pIsFile = 0;

	m_bJsonLoadedFromFile = false;
	m_bJsonLoadedFromBuf = false;

	//m_sJsonDumpFile = DEFAULT_JSON_DUMP_FILE;

	//m_bDumpJsonOnError = false;

	try
	{
		m_pJsonParser = new JSON_PARSER;
	}
	catch(...)
	{
		m_pJsonParser = NULL;
	}
}

void CPocoJsonParserUtil::DeInitCls()
{
	FreeJsonBuffer();

	if (m_pItEntry != NULL)
	{
		delete m_pItEntry;
		m_pItEntry = NULL;
	}

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
void CPocoJsonParserUtil::SetJsonBuffer(char *pBuffer)
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


bool CPocoJsonParserUtil::Parse()
{
	if (m_pJsonParser == NULL)
	{
		return false;
	}

	try
	{
		if (m_pFileBuf != 0 && m_pIsFile != 0)
		{
			m_parseResult = m_pJsonParser->parse(*m_pIsFile);
		}
		else
		{
			if (m_pJsonBuffer == 0)
			{
				return false;
			}

			m_parseResult = m_pJsonParser->parse(m_pJsonBuffer);
		}

		m_pBaseNode = m_parseResult.extract<JSON_OBJECT_PTR>();

		if (m_parseResult.isEmpty() == false)
		{


			m_lastNode.m_eType = JSON_ObjectType_def::ObjectType_Root;
			m_lastNode.m_sName = "";

			try
			{
				m_lastNode.m_pObject = m_parseResult.extract<JSON_OBJECT_PTR>();
			}
			catch (...)
			{
			}

			try
			{
				m_lastNode.m_pArray = m_parseResult.extract<JSON_ARRAY_PTR>();
			}
			catch (...)
			{
			}

			return true;
		}
	}
	catch (std::exception e)
	{
		std::string sErr = e.what();

	}
	catch (...)
	{

	}

	return false;
}


bool CPocoJsonParserUtil::AllocJsonBuffer(int nLen)
{
	if (nLen < 3)
	{
		return false;
	}

	try
	{
		if (m_pJsonBuffer != NULL)
		{
			CPocoJsonParserUtil::FreeJsonBuffer();
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
bool CPocoJsonParserUtil::WriteJsonFile(char *pFolder, char *pFileName) throw(std::runtime_error)
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
bool CPocoJsonParserUtil::DumpJsonBuffer(char *pFolder, char *pFileName, bool bSpaceFill) throw(std::runtime_error)
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

//long CPocoJsonParserUtil::GetJsonLength()
//{
//	if (m_pJsonParser == NULL)
//	{
//		return -1;
//	}
//
//	m_pJsonParser->document.last_node()
//}

