//**************************************************************************************************
//* FILE:		RapidJsonUtils.h
//*
//* DESCRIP:	
//*
//*


#ifndef RapidJsonUtils_H_
#define RapidJsonUtils_H_

#include <string>
#include <ostream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstring>
//#include <vector>
//#include <map>
//#include <filebuf>
#include <cstdio>

#include "RapidJson/reader.h"
#include "RapidJson/document.h"
#include "RapidJson/writer.h"
#include "rapidjson/filereadstream.h"

#include "stringUtils.h"

using namespace std;
//using namespace rapidjson;


//#define JSON_PARSER			rapidjson::Reader
#define JSON_PARSER			rapidjson::Document

#define JSON_MEMBER			rapidjson::Document::Member
#define JSON_MEMBER_PTR		rapidjson::Document::Member *

#define JSON_MEMBER_ITR		rapidjson::Document::MemberIterator
#define JSON_MEMBER_CITR	rapidjson::Document::ConstMemberIterator


#define JSON_OBJECT			rapidjson::Value
//#define JSON_OBJECT		rapidjson::Document

#define JSON_OBJECT_PTR		JSON_OBJECT *
//#define JSON_OBJECT_PTR	JSON_PARSER *

#define JSON_OBJECT_ALLOC	rapidjson::Document::GetAllocator()

//#define JSON_OBJECT_ITR	rapidjson::Value::ValueIterator
#define JSON_OBJECT_ITR		rapidjson::Document::ValueIterator

//#define JSON_OBJECT_CITR	rapidjson::Value::ConstValueIterator
#define JSON_OBJECT_CITR	rapidjson::Document::ConstValueIterator

typedef enum
{
	ObjectType_Unknown,
	ObjectType_Empty,
	ObjectType_String,
	ObjectType_Bool,
	ObjectType_Int,
	ObjectType_Int64,
	ObjectType_UInt,
	ObjectType_UInt64,
	ObjectType_Float,
	ObjectType_Double,
	ObjectType_Numeric,
	ObjectType_SubObject,
	ObjectType_Array,

} JSON_ObjectType_def;


#ifdef UNICODE
#define FILEBUF_TYPE		char_w
#define FILEBUF_DEF			std::filebuf<char_w>
#else
#define FILEBUF_TYPE		char
//#define FILEBUF_DEF			std::filebuf<char>
#define FILEBUF_DEF			std::filebuf
#endif

#define FILE_PTR			std::FILE *

#define FILEREADER			rapidjson::FileReadStream
#define FILEREADER_PTR		rapidjson::FileReadStream *

#define READBUF_SIZE		65536


#ifndef _PATH_SEPERATOR_
#ifdef WIN32
#define _PATH_SEPERATOR_		'\\'
#define _PATH_SEPERATOR_STR_	"\\"
#else
#define _PATH_SEPERATOR_		'/'
#define _PATH_SEPERATOR_STR_	"/"
#endif
#endif

#define DEFAULT_JSON_DUMP_FILE	"dump.json"


class CRapidJsonParserUtil
{
private:
	JSON_PARSER				*m_pJsonParser;

	JSON_OBJECT_PTR			m_pBaseNode;
	JSON_OBJECT_PTR			m_pLastNode;
	JSON_OBJECT_PTR			m_pParentNode;		

	JSON_ObjectType_def		m_eLastNodeType;

	//JSON_OBJECT_ITR			m_iIT;
	JSON_MEMBER_ITR			m_iIT;

	char					*m_pJsonBuffer;

	long					m_lJsonBufLen;

	bool					m_bJsonLoadedFromFile;
	bool					m_bJsonLoadedFromBuf;
	
	FILE_PTR				m_pInFile;

	FILEREADER_PTR			m_pFileReader;

	char					*pFileReadBuf;

	//int					m_nParseFlags;

	//bool					m_bDumpJsonOnError;

	//std::string			m_sJsonDumpFile;

protected:

	JSON_OBJECT_PTR FindJsonParam(JSON_OBJECT_PTR pNode, const std::string sParam, bool bFirst = false)
	{
		try
		{
			if (sParam == "")
			{
				return 0;
			}

			if (pNode == 0)
			{
				long lNumMbembers = pNode->MemberCount();
				if (lNumMbembers < 1)
				{
					return 0;
				}

				if (m_pJsonParser->HasMember(sParam.c_str()) == false)
				{
					return 0;
				}

				auto i = m_pJsonParser->FindMember(sParam.c_str());

				if (i == m_pJsonParser->MemberEnd())
				{
					return 0;
				}

				JSON_OBJECT_PTR pRet = (JSON_OBJECT_PTR) &(*i);

				return pRet;
			}
			else
			{
				long lNumMbembers = pNode->MemberCount();
				if (lNumMbembers < 1)
				{
					return 0;
				}

				if (pNode->HasMember(sParam.c_str()) == false)
				{
					return 0;
				}

				auto i = pNode->FindMember(sParam.c_str());

				if (i == pNode->MemberEnd())
				{
					return 0;
				}

				JSON_OBJECT_PTR pRet = (JSON_OBJECT_PTR) &(*i);

				return pRet;
			}
		}
		catch (...)
		{

		}

		return 0;
	}

	void clsInit();
	void clsDeInit();

public:

	CRapidJsonParserUtil() :
		m_pJsonParser(NULL),
		m_pJsonBuffer(NULL),
		m_lJsonBufLen(0),
		m_bJsonLoadedFromFile(false),
		m_bJsonLoadedFromBuf(false)
	{
		clsInit();
	}

	CRapidJsonParserUtil(bool bDumpOnError) :
		m_pJsonParser(NULL),
		m_pJsonBuffer(NULL),
		m_lJsonBufLen(0),
		m_bJsonLoadedFromFile(false),
		m_bJsonLoadedFromBuf(false)
	{
		clsInit();

		//SetDumpOnError(bDumpOnError);
	}

	//CRapidJsonParserUtil(bool bDumpOnError, const char *pDumpFile) :
	//	m_pJsonParser(NULL),
	//	m_pJsonBuffer(NULL),
	//	m_lJsonBufLen(0),
	//	m_bJsonLoadedFromFile(false),
	//	m_bJsonLoadedFromBuf(false)
	//{
	//	clsInit();

	//	SetDumpOnError(bDumpOnError, pDumpFile);
	//}

	//CRapidJsonParserUtil(bool bDumpOnError, const std::string &sDumpFile) :
	//	m_pJsonParser(NULL),
	//	m_pJsonBuffer(NULL),
	//	m_lJsonBufLen(0),
	//	m_bJsonLoadedFromFile(false),
	//	m_bJsonLoadedFromBuf(false)
	//{
	//	clsInit();

	//	SetDumpOnError(bDumpOnError, sDumpFile);
	//}

	~CRapidJsonParserUtil()
	{
		clsDeInit();
	}

	//void SetDumpOnError(bool bVal)
	//{
	//	m_bDumpJsonOnError = bVal;
	//}
	//void SetDumpOnError(bool bDumpOnError, const char *pDumpFile)
	//{
	//	SetDumpOnError(bDumpOnError);
	//	SetDumpFile(pDumpFile);
	//}
	//void SetDumpOnError(bool bDumpOnError, const std::string &sDumpFile)
	//{
	//	SetDumpOnError(bDumpOnError, (const char *) sDumpFile.c_str());
	//}

	//void SetDumpFile(const char *pDumpFile)
	//{
	//	if (pDumpFile == NULL)
	//	{
	//		m_sJsonDumpFile = DEFAULT_JSON_DUMP_FILE;
	//	}
	//	else
	//	{
	//		m_sJsonDumpFile = pDumpFile;
	//	}
	//}
	//void SetDumpFile(std::string &sDumpFile)
	//{
	//	SetDumpFile((const char *) sDumpFile.c_str());
	//}

	void Clear()
	{
		try 
		{
			m_pJsonParser->Clear();
		}
		catch (...)
		{
			return;
		}
	}

	//JSON_OBJECT_PTR GetJsonBaseNode(const std::string &sParam)
	//{
	//	return GetJsonBaseNode(sParam.c_str());
	//}

	JSON_OBJECT_PTR Create()
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			if (m_pBaseNode != 0)
			{
				m_pBaseNode->Clear();

				m_pBaseNode = 0;
			}

			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			JSON_PARSER * newNode = new JSON_PARSER;
			if (newNode != 0)
			{
				m_pJsonParser->PushBack(*newNode, alloc);

				m_pBaseNode = newNode;
				m_pParentNode = 0;
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			return NULL;
		}
	}

	JSON_OBJECT_PTR Create(const std::string &sName)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			if (m_pBaseNode != 0)
			{
				m_pBaseNode->Clear();

				m_pBaseNode = 0;
			}

			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			JSON_PARSER * newNode = new JSON_PARSER;
			if (newNode != 0)
			{
				newNode->SetString(sName.c_str(), sName.length());

				m_pJsonParser->PushBack(*newNode, alloc);

				m_pBaseNode = newNode;
				m_pParentNode = 0;
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			return NULL;
		}
	}

	bool AllocJsonBuffer(int nLen);

	void FreeJsonBuffer()
	{
		if (m_pJsonBuffer != 0)
		{
			free(m_pJsonBuffer);
		}

		if (m_pFileReader != 0)
		{
			delete m_pFileReader;
			m_pFileReader = 0;
		}

		if (m_pInFile != 0)
		{
			std::fclose(m_pInFile);
			m_pInFile = 0;
		}

		if (pFileReadBuf != 0)
		{
			free(pFileReadBuf);
			pFileReadBuf = 0;
		}

		m_pJsonBuffer = NULL;
				
		m_pInFile = 0;
		m_pFileReader = 0;
		pFileReadBuf = 0;

		m_bJsonLoadedFromFile = false;
		m_bJsonLoadedFromBuf = false;
	}

	char * GetJsonBuffer()
	{
		return m_pJsonBuffer;
	}

	bool Parse();

	bool LoadJsonBuffer(std::string sBuffer) throw(std::runtime_error)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			unsigned long lLen = sBuffer.length();

			if ((long) lLen >= m_lJsonBufLen)
			{
				FreeJsonBuffer();

				if (AllocJsonBuffer(lLen + 1) == false)
				{
					return false;
				}
			}

			memcpy(m_pJsonBuffer, sBuffer.c_str(), lLen);

			return true;
		}
		catch (...)
		{

		}

		return false;
	}

	bool LoadJsonFile(std::string sFolder, std::string sFileName) throw(std::runtime_error)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			if (m_pFileReader != 0)
			{
				delete m_pFileReader;
			}

			if (m_pInFile != 0)
			{
				std::fclose(m_pInFile);
			}

			if (pFileReadBuf != 0)
			{
				free(pFileReadBuf);
			}

			m_pFileReader = 0;
			m_pInFile = 0;
			pFileReadBuf = 0;

			pFileReadBuf = (char *) malloc(READBUF_SIZE);

			if (pFileReadBuf == 0)
			{
				return false;
			}

			memset(pFileReadBuf, 0, READBUF_SIZE);

			std::string sInFile = "";

			if (sFolder == "")
			{
				sInFile = sFileName;
			}
			else
			{
				sInFile = sFolder;

				if (sFolder[sFolder.length() - 1] != _PATH_SEPERATOR_)
				{
					sInFile.append(_PATH_SEPERATOR_STR_);
				}

				sInFile.append(sFileName);
			}

			m_pInFile = std::fopen(sInFile.c_str(), "r");

			if (m_pInFile == 0)
			{
				free(pFileReadBuf);
				pFileReadBuf = 0;
				return false;
			}

			try
			{
				m_pFileReader = new FILEREADER(m_pInFile, pFileReadBuf, READBUF_SIZE);
			}
			catch (...)
			{
				std::fclose(m_pInFile);
				m_pInFile = 0;
				free(pFileReadBuf);
				pFileReadBuf = 0;
				return false;
			}

			return true;
		}
		catch (...)
		{

		}

		return false;
	}

#if 0
	bool WriteJsonFile(std::string sFolder, std::string sFileName) throw(std::runtime_error)
	{
		try
		{

		}
		catch(...)
		{

		}

		return false;
	}
#endif

#if 0
	bool DumpJsonBuffer(std::string sFolder, std::string sFileName = "", bool bSpaceFill = false) throw(std::runtime_error)
	{
		try
		{

		}
		catch(...)
		{

		}

		return false;
	}
#endif

	bool FindJsonParamGroup(JSON_OBJECT_PTR  pNode, std::string sParam, bool bFirst = false)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			 auto obj = FindJsonParam(pNode, sParam, bFirst);

			if (obj == 0)
			{
				return false;
			}

			m_pParentNode = pNode;
			m_pLastNode = obj;

			return true;
		}
		catch(...)
		{

		}

		return false;
	}
	bool FindJsonParamGroup(std::string sParam, bool bFirst = false)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			if (m_pLastNode == 0)
			{
				auto obj = FindJsonParam(m_pBaseNode, sParam, bFirst);

				if (obj == 0)
				{
					return false;
				}

				m_pParentNode = m_pBaseNode;
				m_pLastNode = obj;
			}
			else
			{
				auto obj = FindJsonParam(m_pLastNode, sParam, bFirst);

				if (obj == 0)
				{
					return false;
				}

				m_pParentNode = m_pLastNode;
				m_pLastNode = obj;
			}

			return true;
		}
		catch(...)
		{

		}

		return false;
	}

	bool FindJsonParamString(JSON_OBJECT_PTR pNode, std::string sParam, bool bFirst = false)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			auto obj = FindJsonParam(pNode, sParam, bFirst);

			if (obj == 0)
			{
				return false;
			}

			if (obj->GetType() != rapidjson::Type::kStringType)
			{
				return false;
			}

			m_pParentNode = pNode;
			m_pLastNode = obj;

			return true;
		}
		catch (...)
		{

		}

		return false;
	}
	bool FindJsonParamString(std::string sParam, bool bFirst = false)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			JSON_OBJECT_PTR pCurr = 0;
			
			if (m_pLastNode == 0)
			{
				pCurr = FindJsonParam(m_pBaseNode, sParam, bFirst);
			}
			else
			{
				pCurr = FindJsonParam(m_pLastNode, sParam, bFirst);
			}

			if (pCurr == 0)
			{
				return false;
			}

			if (pCurr->GetType() != rapidjson::Type::kStringType)
			{
				return false;
			}

			m_pParentNode = m_pBaseNode;
			m_pLastNode = pCurr;

			return true;
		}
		catch (...)
		{

		}

		return false;
	}

	JSON_OBJECT_PTR GetJsonParamGroup(JSON_OBJECT_PTR  pNode, std::string sParam, bool bFirst = false)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		if (pNode == NULL)
		{
			return 0;
		}

		try
		{
			auto obj = FindJsonParam(pNode, sParam, bFirst);

			if (obj == 0)
			{
				return 0;
			}

			m_pParentNode = pNode;
			m_pLastNode = obj;

			return obj;
		}
		catch (...)
		{

		}

		return 0;
	}

	JSON_OBJECT_PTR GetJsonParamGroup(std::string sParam, bool bFirst = false)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			JSON_OBJECT_PTR pCurr = 0;

			if (m_pLastNode == 0)
			{
				pCurr = FindJsonParam(m_pBaseNode, sParam, bFirst);
			}
			else
			{
				pCurr = FindJsonParam(m_pLastNode, sParam, bFirst);
			}

			if (pCurr == 0)
			{
				return false;
			}

			m_pParentNode = m_pBaseNode;
			m_pLastNode = pCurr;

			return m_pLastNode;
		}
		catch (...)
		{

		}

		return 0;
	}

	bool HasJsonChildNode(JSON_OBJECT_PTR pNode)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		if (pNode == NULL)
		{
			return 0;
		}

		try
		{
			if (pNode->MemberCount() > 0)
			{
				return true;
			}
		}
		catch (...)
		{

		}

		return false;
	}

	JSON_OBJECT_PTR  GetFirstJsonChild(JSON_OBJECT_PTR pNode)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		if (pNode == NULL)
		{
			return 0;
		}

		try
		{
			if (pNode->MemberCount() > 1)
			{
				return 0;
			}

			m_iIT = pNode->MemberBegin();

			if (m_iIT == pNode->MemberEnd())
			{
				return 0;
			}

			m_pParentNode = pNode;
			m_pLastNode = (JSON_OBJECT_PTR) &(*m_iIT);

			return m_pLastNode;
		}
		catch (...)
		{

		}

		return false;
	}

	JSON_OBJECT_PTR GetNextJsonChild(JSON_OBJECT_PTR pNode, std::string sParam = "")
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		if (pNode == NULL)
		{
			return 0;
		}

		try
		{
			if (m_iIT == pNode->MemberEnd())
			{
				return 0;
			}

			m_iIT++;

			if (m_iIT == pNode->MemberEnd())
			{
				return 0;
			}

			if (sParam == "")
			{
				m_pParentNode = pNode;
				m_pLastNode = (JSON_OBJECT_PTR)&(*m_iIT);

				return m_pLastNode;
			}

			if (m_iIT->name == sParam.c_str())
			{
				m_pParentNode = pNode;
				m_pLastNode = (JSON_OBJECT_PTR)&(*m_iIT);

				return m_pLastNode;
			}
		}
		catch(...)
		{

		}

		return false;
	}

	bool GetNodeName(JSON_OBJECT_PTR pNode, std::string &sName)
	{
		if (m_pJsonParser == NULL)
		{
			return false;
		}

		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			if (pNode->Empty())
			{
				return false;
			}

			sName.assign(pNode->GetString(), pNode->GetStringLength());

			m_pLastNode = pNode;

			return true;
		}
		catch (...)
		{

		}

		return false;
	}

	JSON_ObjectType_def GetSubNodeType(JSON_OBJECT_PTR pNode)
	{
		if (m_pJsonParser == NULL)
		{
			return JSON_ObjectType_def::ObjectType_Unknown;
		}

		if (pNode == NULL)
		{
			return JSON_ObjectType_def::ObjectType_Unknown;
		}

		try
		{
			if (pNode->Empty())
			{
				return JSON_ObjectType_def::ObjectType_Unknown;
			}

			JSON_OBJECT subObj = pNode->GetObject();

			auto type = subObj.GetType();

			switch (type)
			{
			case rapidjson::Type::kNullType:
				return ObjectType_Empty;

			case rapidjson::Type::kObjectType:
				return ObjectType_SubObject;

			case rapidjson::Type::kStringType:
				return ObjectType_String;

			case rapidjson::Type::kNumberType:
				return ObjectType_Numeric;

			case rapidjson::Type::kTrueType:
			case rapidjson::Type::kFalseType:
				return ObjectType_Bool;

			case rapidjson::Type::kArrayType:
				return ObjectType_Array;
			}
		}
		catch (...)
		{

		}

		return JSON_ObjectType_def::ObjectType_Unknown;
	}

	bool GetNodeValue(JSON_OBJECT_PTR pNode, std::string &sVal)
	{
		if (m_pJsonParser == NULL)
		{
			return false;
		}

		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			if (pNode->Empty())
			{
				return false;
			}

			JSON_OBJECT subObj = pNode->GetObject();

			sVal.assign(subObj.GetString(), subObj.GetStringLength());

			m_pLastNode = pNode;

			return true;
		}
		catch (...)
		{

		}

		return false;
	}

	bool LoadJsonParamString
		(
			JSON_OBJECT_PTR pNode, 
			std::string sParam, 
			std::string &sTarget, 
			bool bUseDef = false,
			const std::string &sDefault = "",
			bool bFirst = false
		)
	{
		if (m_pJsonParser == NULL)
		{
			return false;
		}

		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			auto obj = FindJsonParam(pNode, sParam, bFirst);

			if (obj != 0)
			{
				return GetNodeValue(obj, sTarget);
			}
		}
		catch(...)
		{

		}

		if (bUseDef == true)
		{
			sTarget = sDefault;
		}

		return false;
	}
	bool LoadJsonParamString
		(
			std::string sParam, 
			std::string &sTarget, 
			bool bUseDef = false,
			const std::string &sDefault = "",
			bool bFirst = false
		)
	{
		if (m_pJsonParser == NULL)
		{
			return false;
		}

		try
		{
			auto obj = FindJsonParam(m_pLastNode, sParam, bFirst);

			if (obj != 0)
			{
				JSON_OBJECT subObj = obj->GetObject();

				if (subObj.GetType() == rapidjson::Type::kStringType)
				{
					sTarget.assign(subObj.GetString(), subObj.GetStringLength());

					return true;
				}
			}

			return true;
		}
		catch (...)
		{

		}

		if (bUseDef == true)
		{
			sTarget = sDefault;
		}

		return false;
	}

	bool LoadJsonParamBool
		(
			JSON_OBJECT_PTR pNode, 
			std::string sParam, 
			bool *bTarget, 
			bool bUseDef = false, 
			bool bDefVal = 0, 
			bool bFirst = false
		)
	{
		if (m_pJsonParser == NULL)
		{
			return false;
		}

		if (pNode == NULL || bTarget == NULL)
		{
			return false;
		}

		try
		{
			auto obj = FindJsonParam(pNode, sParam, bFirst);

			if (obj != 0)
			{
				JSON_OBJECT subObj = obj->GetObject();

				auto type = subObj.GetType();

				if ((type == rapidjson::Type::kTrueType) || (type == rapidjson::Type::kFalseType))
				{
					*bTarget = subObj.GetBool();

					return true;
				}
			}
		}
		catch (...)
		{

		}

		if (bUseDef == true)
		{
			*bTarget = bDefVal;
		}

		return false;
	}
	bool LoadJsonParamBool
		(
			std::string sParam, 
			bool *bTarget, 
			bool bUseDef = false,
			bool bDefVal = 0,
			bool bFirst = false
		)
	{
		if (m_pJsonParser == NULL)
		{
			return false;
		}

		if (bTarget == NULL)
		{
			return false;
		}

		try
		{
			auto obj = FindJsonParam(m_pLastNode, sParam, bFirst);

			if (obj != 0)
			{
				JSON_OBJECT subObj = obj->GetObject();

				auto type = subObj.GetType();

				if ((type == rapidjson::Type::kTrueType) || (type == rapidjson::Type::kFalseType))
				{
					*bTarget = subObj.GetBool();

					return true;
				}
			}
		}
		catch (...)
		{

		}

		if (bUseDef == true)
		{
			*bTarget = bDefVal;
		}

		return false;
	}

	bool LoadJsonParamInt
		(
			JSON_OBJECT_PTR pNode, 
			std::string sParam, 
			int *nTarget, 
			bool bUseDef = false,
			int nDefVal = 0,
			bool bFirst = false
		)
	{
		if (m_pJsonParser == NULL)
		{
			return false;
		}

		if (pNode == NULL || nTarget == NULL)
		{
			return false;
		}

		try
		{
			auto obj = FindJsonParam(pNode, sParam, bFirst);

			if (obj != 0)
			{
				JSON_OBJECT subObj = obj->GetObject();

				auto type = subObj.GetType();

				if (type == rapidjson::Type::kNumberType)
				{
					*nTarget = subObj.GetInt();

					return true;
				}
			}
		}
		catch (...)
		{

		}

		if (bUseDef == true)
		{
			*nTarget = nDefVal;
		}

		return false;
	}
	bool LoadJsonParamInt
		(
			std::string sParam, 
			int *nTarget, 
			bool bUseDef = false,
			int nDefVal = 0,
			bool bFirst = false
		)
	{
		if (m_pJsonParser == NULL)
		{
			return false;
		}

		if (nTarget == NULL)
		{
			return false;
		}

		try
		{
			auto obj = FindJsonParam(m_pLastNode, sParam, bFirst);

			if (obj != 0)
			{
				JSON_OBJECT subObj = obj->GetObject();

				auto type = subObj.GetType();

				if (type == rapidjson::Type::kNumberType)
				{
					*nTarget = subObj.GetInt();

					return true;
				}
			}
		}
		catch (...)
		{

		}

		if (bUseDef == true)
		{
			*nTarget = nDefVal;
		}

		return false;
	}

	bool LoadJsonParamLong
		(
			JSON_OBJECT_PTR pNode, 
			std::string sParam, 
			long *lTarget, 
			bool bUseDef = false,
			long lDefVal = 0,
			bool bFirst = false
		)
	{
		if (m_pJsonParser == NULL)
		{
			return false;
		}

		if (pNode == NULL || lTarget == NULL)
		{
			return false;
		}

		try
		{
			auto obj = FindJsonParam(pNode, sParam, bFirst);

			if (obj != 0)
			{
				JSON_OBJECT subObj = obj->GetObject();

				auto type = subObj.GetType();

				if (type == rapidjson::Type::kNumberType)
				{
					*lTarget = (long) subObj.GetInt64();

					return true;
				}
			}
		}
		catch (...)
		{

		}

		if (bUseDef == true)
		{
			*lTarget = lDefVal;
		}

		return false;
	}

	JSON_OBJECT_PTR  GetLastNode()
	{
		return m_pLastNode;
	}

	void SetLastNode(JSON_OBJECT_PTR pNode)
	{
		m_pLastNode = pNode;
	}

	JSON_OBJECT_PTR Parent(JSON_OBJECT_PTR pNode)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		return m_pParentNode;
	}

	void AppendNode(JSON_OBJECT_PTR child)
	{
		if (m_pJsonParser == NULL || child == NULL)
		{
			return;
		}

		try
		{
			if (m_pLastNode == NULL)
			{
				return;
			}

			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			m_pLastNode->PushBack(*child, alloc);

			m_pLastNode = child;
		}
		catch (...)
		{

		}
	}

	void AppendNode(JSON_OBJECT_PTR parent, JSON_OBJECT_PTR child)
	{
		if (m_pJsonParser == NULL || parent == NULL || child == NULL)
		{
			return;
		}

		try
		{
			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			parent->PushBack(*child, alloc);

			m_pLastNode = child;
		}
		catch (...)
		{

		}
	}

	JSON_OBJECT_PTR NewNode(const std::string sName, const std::string sVal = "")
	{
		if (m_pJsonParser == NULL || sName == "")
		{
			return NULL;
		}

		try
		{
			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			JSON_OBJECT * newNode = new JSON_OBJECT;
			if (newNode != 0)
			{
				if (sVal != "")
				{
					JSON_OBJECT * newSubNode = new JSON_OBJECT;
					if (newNode != 0)
					{
						newSubNode->SetString(sVal.c_str(), sVal.length());

						newNode->PushBack(*newSubNode, alloc);
					}
				}

				newNode->SetString(sName.c_str(), sName.length());

				m_pLastNode->PushBack(*newNode, alloc);
				
				m_pParentNode = m_pLastNode;
				m_pLastNode = (JSON_OBJECT_PTR) newNode;
			}

			return (JSON_OBJECT_PTR) newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	JSON_OBJECT_PTR NewNode(JSON_OBJECT_PTR pParent, const std::string sName, const std::string sVal = "")
	{
		if (m_pJsonParser == NULL || pParent == NULL)
		{
			return NULL;
		}

		try
		{
			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			JSON_OBJECT * newNode = new JSON_OBJECT;
			if (newNode != 0)
			{
				if (sVal != "")
				{
					JSON_OBJECT * newSubNode = new JSON_OBJECT;
					if (newNode != 0)
					{
						newSubNode->SetString(sVal.c_str(), sVal.length());

						newNode->PushBack(*newSubNode, alloc);
					}
				}

				newNode->SetString(sName.c_str(), sName.length());

				m_pLastNode->PushBack(*newNode, alloc);

				m_pParentNode = pParent;
				m_pLastNode = (JSON_OBJECT_PTR) newNode;
			}

			return (JSON_OBJECT_PTR) newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	JSON_OBJECT_PTR NewStringNode(const std::string sName, const std::string sVal = "")
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			JSON_OBJECT * newNode = new JSON_OBJECT;
			if (newNode != 0)
			{
				if (sVal != "")
				{
					JSON_OBJECT * newSubNode = new JSON_OBJECT;
					if (newNode != 0)
					{
						newSubNode->SetString(sVal.c_str(), sVal.length());

						newNode->PushBack(*newSubNode, alloc);
					}
				}

				newNode->SetString(sName.c_str(), sName.length());

				m_pLastNode->PushBack(*newNode, alloc);

				m_pParentNode = m_pLastNode;
				m_pLastNode = (JSON_OBJECT_PTR) newNode;
			}

			return (JSON_OBJECT_PTR) newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewStringNode(JSON_OBJECT_PTR pParent, const std::string sName, const std::string sVal = "")
	{
		if (m_pJsonParser == NULL || pParent == NULL)
		{
			return NULL;
		}

		try
		{
			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			JSON_OBJECT * newNode = new JSON_OBJECT;
			if (newNode != 0)
			{
				if (sVal != "")
				{
					JSON_OBJECT * newSubNode = new JSON_OBJECT;
					if (newNode != 0)
					{
						newSubNode->SetString(sVal.c_str(), sVal.length());

						newNode->PushBack(*newSubNode, alloc);
					}
				}

				newNode->SetString(sName.c_str(), sName.length());

				m_pLastNode->PushBack(*newNode, alloc);

				m_pParentNode = pParent;
				m_pLastNode = (JSON_OBJECT_PTR) newNode;
			}

			return (JSON_OBJECT_PTR) newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewBoolNode(const std::string sName, bool bVal)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			JSON_OBJECT * newNode = new JSON_OBJECT;
			if (newNode != 0)
			{
				JSON_OBJECT * newSubNode = new JSON_OBJECT;
				if (newNode != 0)
				{
					newSubNode->SetBool(bVal);

					newNode->PushBack(*newSubNode, alloc);
				}

				newNode->SetString(sName.c_str(), sName.length());

				m_pLastNode->PushBack(*newNode, alloc);

				m_pParentNode = m_pLastNode;
				m_pLastNode = (JSON_OBJECT_PTR) newNode;
			}

			return (JSON_OBJECT_PTR) newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewBoolNode(JSON_OBJECT_PTR pParent, const std::string sName, bool bVal)
	{
		if (m_pJsonParser == NULL || pParent == NULL)
		{
			return NULL;
		}

		try
		{
			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			JSON_OBJECT * newNode = new JSON_OBJECT;
			if (newNode != 0)
			{
				JSON_OBJECT * newSubNode = new JSON_OBJECT;
				if (newNode != 0)
				{
					newSubNode->SetBool(bVal);

					newNode->PushBack(*newSubNode, alloc);
				}

				newNode->SetString(sName.c_str(), sName.length());

				m_pLastNode->PushBack(*newNode, alloc);

				m_pParentNode = pParent;
				m_pLastNode = (JSON_OBJECT_PTR) newNode;
			}

			return (JSON_OBJECT_PTR) newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewIntNode(const std::string sName, int nVal)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			JSON_OBJECT * newNode = new JSON_OBJECT;
			if (newNode != 0)
			{
				JSON_OBJECT * newSubNode = new JSON_OBJECT;
				if (newNode != 0)
				{
					newSubNode->SetInt(nVal);

					newNode->PushBack(*newSubNode, alloc);
				}

				newNode->SetString(sName.c_str(), sName.length());

				m_pLastNode->PushBack(*newNode, alloc);

				m_pParentNode = m_pLastNode;
				m_pLastNode = (JSON_OBJECT_PTR) newNode;
			}

			return (JSON_OBJECT_PTR) newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewIntNode(JSON_OBJECT_PTR pParent, const std::string sName, int nVal)
	{
		if (m_pJsonParser == NULL || pParent == NULL)
		{
			return NULL;
		}

		try
		{
			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			JSON_OBJECT * newNode = new JSON_OBJECT;
			if (newNode != 0)
			{
				JSON_OBJECT * newSubNode = new JSON_OBJECT;
				if (newNode != 0)
				{
					newSubNode->SetInt(nVal);

					newNode->PushBack(*newSubNode, alloc);
				}

				newNode->SetString(sName.c_str(), sName.length());

				m_pLastNode->PushBack(*newNode, alloc);

				m_pParentNode = pParent;
				m_pLastNode = (JSON_OBJECT_PTR) newNode;
			}

			return (JSON_OBJECT_PTR) newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewLongNode(const std::string sName, long lVal)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			JSON_OBJECT * newNode = new JSON_OBJECT;
			if (newNode != 0)
			{
				JSON_OBJECT * newSubNode = new JSON_OBJECT;
				if (newNode != 0)
				{
					newSubNode->SetInt64(lVal);

					newNode->PushBack(*newSubNode, alloc);
				}

				newNode->SetString(sName.c_str(), sName.length());

				m_pLastNode->PushBack(*newNode, alloc);

				m_pParentNode = m_pLastNode;
				m_pLastNode = (JSON_OBJECT_PTR) newNode;
			}

			return (JSON_OBJECT_PTR) newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewlongNode(JSON_OBJECT_PTR pParent, const std::string sName, long lVal)
	{
		if (m_pJsonParser == NULL || pParent == NULL)
		{
			return NULL;
		}

		try
		{
			rapidjson::Document::AllocatorType& alloc = m_pJsonParser->GetAllocator();

			JSON_OBJECT * newNode = new JSON_OBJECT;
			if (newNode != 0)
			{
				JSON_OBJECT * newSubNode = new JSON_OBJECT;
				if (newNode != 0)
				{
					newSubNode->SetInt64(lVal);

					newNode->PushBack(*newSubNode, alloc);
				}

				newNode->SetString(sName.c_str(), sName.length());

				m_pLastNode->PushBack(*newNode, alloc);

				m_pParentNode = pParent;
				m_pLastNode = (JSON_OBJECT_PTR) newNode;
			}

			return (JSON_OBJECT_PTR) newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	//JSON_OBJECT_PTR  FindChild(JSON_OBJECT_PTR  jsonNode, char *name)
	//{
	//	return jsonNode->
	//}
#if 0
	void JsonOutput(char *target, JSON_OBJECT_PTR  jsonNode, bool bAddHeader = false)
	{
		if (m_pJsonParser == NULL || target == NULL || jsonNode == NULL)
		{
			return;
		}

		try
		{
			long nLen = 0;

			if (bAddHeader == true)
			{
				sprintf(target, "<?json version=\"1.0\" encoding=\"utf-8\"?> \n ");
				nLen = (long) strlen(target);
			}

			print((target + nLen), *jsonNode, 0);
		}
		catch(...)
		{

		}
	}

#endif

#if 0
class CJsonWriterUtil
{
public:

	typedef enum
	{
		NODE_TYPE_UNKNOWN,
		NODE_TYPE_BASE,
		NODE_TYPE_GROUP,
		NODE_TYPE_KEY
	
	} JsonNodeType_def;

	typedef struct
	{
		void			*pParent;

		JsonNodeType_def	eType;

		char			*pNodeStart;
		//char			*pNodeEnd;

		char			*pNodeName;

		int				nNameLen;

		char			*pNodeValue;

		int				nValueLen;

	} JsonNode_def;

private:

	char					*m_pOutputBuffer;

	std::string				m_sWorkingBuffer;

	long					m_lBufferIndex;

	bool					m_bWriteJsonToFile;

	JsonNode_def				m_LastNode;

public:

	CJsonWriterUtil();
	~CJsonWriterUtil();

	void Init()
	{
		m_pOutputBuffer = NULL;

		m_lBufferIndex = 0;

		m_bWriteJsonToFile = false;

	}

	void Init(char *pBuffer);

	void SetJsonBuffer(char *pBuffer);

	void Create()
	{


	}

	void Create(char *pName)
	{


	}

	void Create(char *pName, char *pValue)
	{

	}

	void AppendNode(JsonNode_def *parent, JsonNode_def *child)
	{

	}

	JsonNode_def * NewGroupgNode(char *name)
	{

		return NULL;
	}

	JsonNode_def * NewStringNode(char *name, string value)
	{
		return NewStringNode(name, (char *) (value.c_str()));
	}

	JsonNode_def * NewStringNode(JsonNode_def *parent, char *name, char *value)
	{

		return NULL;
	}

	JsonNode_def * NewStringNode(JsonNode_def *parent, char *name, string value)
	{
		return NewStringNode(parent, name, (char *) (value.c_str()));
	}

	JSON_OBJECT_PTR  NewIntNode(char *name, int value)
	{

		return NULL;
	}

	JsonNode_def * NewIntNode(JsonNode_def *parent, char *name, int value)
	{

		return NULL;
	}

	JsonNode_def * NewBoolNode(char *name, bool value)
	{

		return NULL;
	}

	JsonNode_def * NewBoolNode(JsonNode_def *parent, char *name, bool value)
	{

		return NULL;
	}

};
#endif

};

#endif //  RapidJsonUtils_H_
