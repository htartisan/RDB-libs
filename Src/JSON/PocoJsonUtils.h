//**************************************************************************************************
//* FILE:		PocoJsonUtils.h
//*
//* DESCRIP:	
//*
//*


#ifndef PocoJsonUtils_H_
#define PocoJsonUtils_H_

#include <string>
#include <ostream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstring>
//#include <vector>
//#include <map>
//#include <filebuf>

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Query.h"
#include "Poco/Dynamic/Var.h"


#include "stringUtils.h"

using namespace std;
using namespace Poco::JSON;


#define JSON_PARSER			Poco::JSON::Parser

#define JSON_PARSER_RESULT	Poco::Dynamic::Var

#define JSON_VAR			Poco::Dynamic::Var
//#define JSON_VAR_PTR		Poco::Dynamic::Var::Ptr

#define JSON_VAR_ITR		Poco::Dynamic::Var::Iterator
#define JSON_VAR_CITR		Poco::Dynamic::Var::ConstIterator

#define JSON_OBJECT			Poco::JSON::Object
#define JSON_OBJECT_PTR		Poco::JSON::Object::Ptr

#define JSON_OBJECT_ITR		Poco::JSON::Object::Iterator
#define JSON_OBJECT_CITR	Poco::JSON::Object::ConstIterator

#define JSON_ARRAY			Poco::JSON::Array
#define JSON_ARRAY_PTR		Poco::JSON::Array::Ptr

#define JSON_ARRAY_ITR		Poco::JSON::Array::Iterator
#define JSON_ARRAY_CITR		Poco::JSON::Array::ConstIterator


#ifndef E_OBJECT_TYPE_DEF
#define E_OBJECT_TYPE_DEF
typedef enum
{
	ObjectType_Unknown,
	ObjectType_Empty,
	ObjectType_String,
	ObjectType_Bool,
	ObjectType_Int,
	ObjectType_UInt,
	ObjectType_Int64,
	ObjectType_UInt64,
	ObjectType_Float,
	ObjectType_Double,
	ObjectType_Numeric,
	ObjectType_UnsignedNumeric,
	ObjectType_Timestamp,
	ObjectType_Array,
	ObjectType_Vector,
	ObjectType_List,
	ObjectType_SubObject,
	ObjectType_Root,

} JSON_ObjectType_def;
#endif


#ifdef UNICODE
#define FILEBUF_TYPE		char_w
#define FILEBUF_DEF			std::filebuf<char_w>
#else
#define FILEBUF_TYPE		char
//#define FILEBUF_DEF			std::filebuf<char>
#define FILEBUF_DEF			std::filebuf
#endif

#define FILEBUF_PTR			std::filebuf *

#define ISTREAM_PTR			std::istream *


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

typedef struct JsonEntryInfo_decl
{
	JSON_ObjectType_def	m_eType;

	std::string			m_sName;

	JSON_OBJECT_PTR		m_pParent;

	JSON_OBJECT_PTR		m_pObject;

	JSON_ARRAY_PTR		m_pArray;

	JsonEntryInfo_decl()
	{
		m_eType = JSON_ObjectType_def::ObjectType_Unknown;
		m_sName = "";
		m_pParent = 0;
		m_pObject = 0;
		m_pArray = 0;
	}

	void copy(JsonEntryInfo_decl &ref)
	{
		m_eType =		ref.m_eType;
		m_sName =		ref.m_sName;
		m_pParent =		ref.m_pParent;
		m_pObject =		ref.m_pObject;
		m_pArray =		ref.m_pArray;
	}

} JsonEntryInfo_def;



class CPocoJsonParserUtil
{
private:

	JSON_PARSER				*m_pJsonParser;

	JSON_PARSER_RESULT		m_parseResult;

	JSON_OBJECT_PTR			m_pBaseNode;

	JsonEntryInfo_def		m_lastNode;

	//JsonEntryInfo_def		m_parentNode;

	JSON_VAR_ITR			*m_pItEntry;

	JSON_OBJECT_ITR			m_itObject;

	char					*m_pJsonBuffer;

	long					m_lJsonBufLen;

	bool					m_bJsonLoadedFromFile;
	bool					m_bJsonLoadedFromBuf;
	
	FILEBUF_PTR				m_pFileBuf;
	ISTREAM_PTR				m_pIsFile;

	//int					m_nParseFlags;

	//bool					m_bDumpJsonOnError;

	//std::string			m_sJsonDumpFile;

protected:

	inline bool ObjIsNull(JSON_OBJECT_PTR pNode)
	{
		if (pNode == (JSON_OBJECT_PTR) 0)
		{
			return true;
		}

		return false;
	}

	inline bool ArrayIsNull(JSON_ARRAY_PTR pArray)
	{
		if (pArray == (JSON_ARRAY_PTR) 0)
		{
			return true;
		}

		return false;
	}

	inline JSON_ObjectType_def GetEntryType(JSON_VAR &rVar)
	{
		try
		{
			if (rVar.isEmpty() == true)
			{
				return JSON_ObjectType_def::ObjectType_Empty;
			}

			if (rVar.isString() == true)
			{
				return JSON_ObjectType_def::ObjectType_String;
			}
			
			if (rVar.isBoolean() == true)
			{
				return JSON_ObjectType_def::ObjectType_Bool;
			}
			
			if (rVar.isNumeric() == true)
			{
				if (rVar.isSigned() == false)
				{
					return JSON_ObjectType_def::ObjectType_UnsignedNumeric;
				}

				return JSON_ObjectType_def::ObjectType_Numeric;
			}
			
			if (rVar.isArray() == true)
			{
				return JSON_ObjectType_def::ObjectType_Array;
			}
			
			if (rVar.isVector() == true)
			{
				return JSON_ObjectType_def::ObjectType_Vector;
			}
			
			if (rVar.isList() == true)
			{
				return JSON_ObjectType_def::ObjectType_List;
			}
			
			//if (rVar.isStruct() == true)
			//{
			//	return JSON_ObjectType_def::ObjectType_SubObject;
			//}
			
			//if (rVar.isDeque() == true)
			//{
			//	return JSON_ObjectType_def::ObjectType_SubObject;
			//}
			
			return JSON_ObjectType_def::ObjectType_SubObject;
		}
		catch (...)
		{
		}

		return JSON_ObjectType_def::ObjectType_Unknown;
	}

	int GetEntryInt(JSON_VAR &rVar)
	{
		try
		{
			int nVal = 0;

			rVar.convert<int>(nVal);

			return nVal;
		}
		catch (...)
		{
		}

		return 0;
	}

	float GetEntryFloat(JSON_VAR &rVar)
	{
		try
		{
			float fVal = 0;

			rVar.convert<float>(fVal);

			return fVal;
		}
		catch (...)
		{
		}

		return 0;
	}

	double GetEntryDouble(JSON_VAR &rVar)
	{
		try
		{
			double dVal = 0;

			rVar.convert<double>(dVal);

			return dVal;
		}
		catch (...)
		{
		}

		return 0;
	}

	bool GetEntryBool(JSON_VAR &rVar)
	{
		try
		{
			bool bVal = false;

			rVar.convert<bool>(bVal);

			return bVal;
		}
		catch (...)
		{
		}

		return 0;
	}

	std::string GetEntryStr(JSON_VAR &rVar)
	{
		try
		{
			std::string sVal = "";

			rVar.convert<string>(sVal);

			return sVal;
		}
		catch (...)
		{
		}

		return "";
	}

	Poco::Timestamp GetEntryTimestamp(JSON_VAR &rVar)
	{
		try
		{
			Poco::Timestamp tVal = 0;

			rVar.convert<Poco::Timestamp>(tVal);

			return tVal;
		}
		catch (...)
		{
		}

		return 0;
	}

	JSON_VAR FindEntryPtr(JSON_OBJECT_PTR pObj, const std::string &sName = "", bool bFirst = true)
	{
		JSON_VAR ret;

		try
		{
			JSON_OBJECT_ITR itObj;

			if (bFirst == true)
			{
				if (pObj == (JSON_OBJECT_PTR) 0)
				{
					return ret;
				}

				itObj = pObj->begin();
			}
			else
			{
				itObj = m_itObject;

				itObj++;
			}

			while (true) 
			{
				if (itObj == pObj->end())
				{
					break;
				}

				std::string sCurr = itObj->first;

				if ((sName == "") || (sCurr == sName))
				{
					m_itObject = itObj;

					ret = (itObj->second);

					return ret;
				}

				itObj++;
			}
		}
		catch (...)
		{
		}

		return ret;
	}

	JSON_OBJECT_PTR FindObjectPtr(JSON_OBJECT_PTR pObj, const std::string &sName = "", bool bFirst = true)
	{
		try
		{
			JSON_OBJECT_ITR itObj;

			if (bFirst == true)
			{
				if (pObj == (JSON_OBJECT_PTR)0)
				{
					return 0;
				}

				itObj = pObj->begin();
			}
			else
			{
				itObj = m_itObject;

				itObj++;
			}

			while (true)
			{
				if (itObj == pObj->end())
				{
					break;
				}

				std::string sCurr = itObj->first;

				JSON_VAR Var = (itObj->second);

				if ((sName == "") || (sCurr == sName))
				{
					m_itObject = itObj;

					return (Var.extract<JSON_OBJECT_PTR>());
				}

				itObj++;
			}

			m_itObject = itObj;
		}
		catch (...)
		{
		}

		return 0;
	}

	JSON_ARRAY_PTR FindArrayPtr(JSON_OBJECT_PTR pObj, const std::string &sName = "", bool bFirst = true)
	{
		try
		{
			JSON_OBJECT_ITR itObj;

			if (bFirst == true)
			{
				if (pObj == (JSON_OBJECT_PTR)0)
				{
					return 0;
				}

				itObj = pObj->begin();
			}
			else
			{
				itObj = m_itObject;

				itObj++;
			}

			while (true)
			{
				if (itObj == pObj->end())
				{
					break;
				}

				std::string sCurr = itObj->first;

				JSON_VAR Var = (itObj->second);

				if ((sName == "") || (sCurr == sName))
				{
					m_itObject = itObj;

					return (Var.extract<JSON_ARRAY_PTR>());
				}

				itObj++;
			}

			m_itObject = itObj;
		}
		catch (...)
		{
		}

		return 0;
	}

	JSON_OBJECT_PTR GetObjectPtrAt(JSON_ARRAY_PTR pArray, const unsigned nIdx = 0)
	{
		try
		{
			if (pArray == (JSON_ARRAY_PTR) 0)
			{
				return 0;
			}

			return (pArray->getObject(nIdx));
		}
		catch (...)
		{
		}

		return ((JSON_OBJECT_PTR)0);
	}


	std::string FindString(JSON_OBJECT_PTR pObj, const std::string &sName = "", bool bFirst = true)
	{
		try
		{
			JSON_OBJECT_ITR itObj = pObj->end();

			if (bFirst == true)
			{
				if (pObj == (JSON_OBJECT_PTR) 0)
				{
					return "";
				}

				JSON_OBJECT_ITR itObj = pObj->begin();
			}
			else
			{
				itObj = m_itObject;

				itObj++;
			}

			for (; itObj != pObj->end(); itObj++)
			{
				std::string sCurr = itObj->first;

				JSON_VAR Var = (itObj->second);

				if ((sName == "") || (sCurr == sName))
				{
					return (Var.toString());
				}
			}

			m_itObject = itObj;
		}
		catch (...)
		{
		}

		return "";
	}

	JSON_OBJECT_PTR GetVarObjectPtr(JSON_VAR &rVar)
	{
		try
		{
			if (rVar.isEmpty() == true)
			{
				return 0;
			}

			return rVar.extract<JSON_OBJECT_PTR>();
		}
		catch (...)
		{

		}

		return 0;
	}

	JSON_OBJECT_PTR GetEntryObjectPtr(JsonEntryInfo_def *pEntry)
	{
		try
		{
			if (pEntry == 0)
			{
				return 0;
			}

			return pEntry->m_pObject;
		}
		catch (...)
		{

		}

		return 0;
	}

	JSON_OBJECT_PTR GetSubObjectPtr(JSON_OBJECT_PTR pNode, const std::string &sKey = "")
	{
		try
		{
			if (ObjIsNull(pNode) == true)
			{
				return 0;
			}

			if (pNode->isObject(sKey) == true)
			{
				return pNode->getObject(sKey);
			}
		}
		catch (...)
		{

		}

		return 0;
	}

	JSON_ARRAY_PTR GetVarArrayPtr(JSON_VAR &rVar)
	{
		try
		{
			if (rVar.isEmpty() == true)
			{
				return 0;
			}

			return rVar.extract<JSON_ARRAY_PTR>();
		}
		catch (...)
		{

		}

		return 0;
	}

	JSON_ARRAY_PTR GetEntryArrayPtr(JsonEntryInfo_def *pEntry)
	{
		try
		{
			if (pEntry == 0)
			{
				return 0;
			}

			return pEntry->m_pArray;
		}
		catch (...)
		{

		}

		return 0;
	}

	bool GetJsonChild
		(
			JSON_VAR &rVar,
			bool bFirst = false
		)
	{
		try
		{
			auto iEnd = rVar.end();

			if (bFirst == true)
			{
				if (m_pItEntry != 0)
				{
					delete m_pItEntry;
					m_pItEntry = 0;
				}

				m_pItEntry = new JSON_VAR_ITR(&rVar, true);

				if (m_pItEntry == 0)
				{
					return false;
				}

				(*m_pItEntry) = rVar.begin();
			}
			else
			{
				if (m_pItEntry == 0)
				{
					return false;
				}

				if ((*m_pItEntry) == iEnd)
				{
					return false;
				}

				(*m_pItEntry)++;
			}

			if ((*m_pItEntry) == iEnd)
			{
				return false;
			}

			return true;
		}
		catch (...)
		{

		}

		return false;
	}


public:

	CPocoJsonParserUtil() :
		m_pJsonParser(NULL),
		m_pJsonBuffer(NULL),
		m_lJsonBufLen(0),
		m_bJsonLoadedFromFile(false),
		m_bJsonLoadedFromBuf(false)
	{
		InitCls();
	}

	CPocoJsonParserUtil(bool bDumpOnError) :
		m_pJsonParser(NULL),
		m_pJsonBuffer(NULL),
		m_lJsonBufLen(0),
		m_bJsonLoadedFromFile(false),
		m_bJsonLoadedFromBuf(false)
	{
		InitCls();

		//SetDumpOnError(bDumpOnError);
	}

	//CPocoJsonParserUtil(bool bDumpOnError, const char *pDumpFile) :
	//	m_pJsonParser(NULL),
	//	m_pJsonBuffer(NULL),
	//	m_lJsonBufLen(0),
	//	m_bJsonLoadedFromFile(false),
	//	m_bJsonLoadedFromBuf(false)
	//{
	//	InitCls();

	//	SetDumpOnError(bDumpOnError, pDumpFile);
	//}

	//CPocoJsonParserUtil(bool bDumpOnError, const std::string &sDumpFile) :
	//	m_pJsonParser(NULL),
	//	m_pJsonBuffer(NULL),
	//	m_lJsonBufLen(0),
	//	m_bJsonLoadedFromFile(false),
	//	m_bJsonLoadedFromBuf(false)
	//{
	//	Init()Cls;

	//	SetDumpOnError(bDumpOnError, sDumpFile);
	//}

	~CPocoJsonParserUtil()
	{
		DeInitCls();
	}

	void InitCls();
	void DeInitCls();

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
			m_pJsonParser->reset();
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
			JSON_OBJECT_PTR pNewNode = new Poco::JSON::Object();
			if (ObjIsNull(pNewNode) == false)
			{
				m_parseResult = (*pNewNode);

				m_pBaseNode = m_parseResult.extract<JSON_OBJECT_PTR>();

				m_lastNode.m_eType =	JSON_ObjectType_def::ObjectType_Root;
				m_lastNode.m_sName =	"";
				m_lastNode.m_pParent =	0;
				m_lastNode.m_pObject =	m_parseResult.extract<JSON_OBJECT_PTR>();
				m_lastNode.m_pArray =	0;
			}

			return pNewNode;
		}
		catch (...)
		{
			return NULL;
		}
	}

	JSON_OBJECT_PTR Create(const std::string &sName, const std::string &sVal = "")
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			JSON_OBJECT_PTR pNewNode = new Poco::JSON::Object();
			if (ObjIsNull(pNewNode) == false)
			{
				if (sVal != "")
				{
					pNewNode->set(sName, sVal);
				}

				m_parseResult = (*pNewNode);

				m_pBaseNode = m_parseResult.extract<JSON_OBJECT_PTR>();

				m_lastNode.m_eType =	JSON_ObjectType_def::ObjectType_Root;
				m_lastNode.m_sName =	sName;
				m_lastNode.m_pParent =	0;
				m_lastNode.m_pObject =	m_parseResult.extract<JSON_OBJECT_PTR>();
				m_lastNode.m_pArray =	0;
			}
			return pNewNode;
		}
		catch (...)
		{
			return NULL;
		}
	}

	bool AllocJsonBuffer(int nLen);

	void FreeJsonBuffer()
	{
		try
		{
			if (m_pJsonBuffer != NULL)
			{
				//* if the buffer is not currently NULL... free it

				free(m_pJsonBuffer);
			}

			if (m_pIsFile != NULL)
			{
				delete m_pIsFile;
			}
			
			if (m_pFileBuf != NULL)
			{
				m_pFileBuf->close();
				
				delete m_pFileBuf;
			}
		}
		catch(...)
		{

		}

		m_pJsonBuffer = NULL;
				
		m_pIsFile = NULL;
		m_pFileBuf = NULL;

		m_bJsonLoadedFromFile = false;
		m_bJsonLoadedFromBuf = false;
	}

	char * GetJsonBuffer()
	{
		return m_pJsonBuffer;
	}

	bool Parse();

	bool LoadJsonBuffer(const std::string &sBuffer) throw(std::runtime_error)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			unsigned long lLen = sBuffer.length();

			if (lLen >= m_lJsonBufLen)
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

	bool LoadJsonFile(const std::string &sFolder, const std::string &sFileName) throw(std::runtime_error)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{
			if (m_pIsFile != 0)
			{
				delete m_pIsFile;
				m_pIsFile = 0;
			}

			if (m_pFileBuf != 0)
			{
				m_pFileBuf->close();
				delete m_pFileBuf;
				m_pFileBuf = 0;
			}

			m_pFileBuf = new FILEBUF_DEF;

			if (m_pFileBuf == 0)
			{
				return false;
			}

			std::string sInFile = sFolder;
			if (!sInFile.empty() && sInFile[sInFile.length() - 1] != _PATH_SEPERATOR_)
			{
				sInFile.append(_PATH_SEPERATOR_STR_);
			}
			sInFile.append(sFileName);

			std::filebuf *pStatus =
				m_pFileBuf->open(sInFile.c_str(), std::ios::in);

			if (pStatus == NULL)
			{
				delete m_pFileBuf;
				m_pFileBuf = 0;
				return false;
			}

			m_pIsFile = new std::istream(m_pFileBuf);

			if (m_pIsFile == 0)
			{
				m_pFileBuf->close();
				delete m_pFileBuf;
				m_pFileBuf = 0;

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
	bool WriteJsonFile(const std::string &sFolder, const std::string &sFileName) throw(std::runtime_error)
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
	bool DumpJsonBuffer(const std::string &sFolder, const std::string &sFileName = "", bool bSpaceFill = false) throw(std::runtime_error)
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

	bool FindJsonParamGroup(JSON_OBJECT_PTR pNode, const std::string &sParam, bool bFirst = false)
	{
		try
		{
			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			m_lastNode.m_eType =	eType;
			m_lastNode.m_sName =	sParam;
			m_lastNode.m_pParent =	0;

			if (eType == JSON_ObjectType_def::ObjectType_Array)
			{
				m_lastNode.m_pObject = 0;
				m_lastNode.m_pArray = Var.extract<JSON_ARRAY_PTR>();
			}
			else
			{
				m_lastNode.m_pObject = Var.extract<JSON_OBJECT_PTR>();
				m_lastNode.m_pArray = 0;
			}

			return true;
		}
		catch(...)
		{

		}

		return false;
	}
	bool FindJsonParamGroup(const std::string &sParam, bool bFirst = false)
	{
		try
		{
			JSON_OBJECT_PTR pNode;
			JSON_OBJECT_PTR pLastObj;

			if (ObjIsNull(m_lastNode.m_pObject) == true)
			{
				if (ObjIsNull(m_lastNode.m_pParent) == true)
				{
					pNode = m_parseResult.extract<JSON_OBJECT_PTR>();
					pLastObj = 0;
				}
				else
				{
					pNode = m_lastNode.m_pParent;
					pLastObj = 0;
				}
			}
			else
			{
				pNode = m_lastNode.m_pObject;
				pLastObj = m_lastNode.m_pParent;
			}

			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			m_lastNode.m_eType =	eType;
			m_lastNode.m_sName =	sParam;
			m_lastNode.m_pParent =	pLastObj;

			if (eType == JSON_ObjectType_def::ObjectType_Array)
			{
				m_lastNode.m_pArray = Var.extract<JSON_ARRAY_PTR>();
				m_lastNode.m_pObject = 0;
			}
			else
			{
				m_lastNode.m_pObject = Var.extract<JSON_OBJECT_PTR>();
				m_lastNode.m_pArray = 0;
			}

			return true;
		}
		catch(...)
		{

		}

		return false;
	}

	bool FindJsonParamArray(JSON_OBJECT_PTR pNode, const std::string &sParam, bool bFirst = false)
	{
		try
		{
			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			m_lastNode.m_eType = JSON_ObjectType_def::ObjectType_Array;
			m_lastNode.m_sName = sParam;
			m_lastNode.m_pParent = 0;
			m_lastNode.m_pObject = 0;
			m_lastNode.m_pArray = Var.extract<JSON_ARRAY_PTR>();

			return true;
		}
		catch (...)
		{

		}

		return false;
	}
	bool FindJsonParamArray(const std::string &sParam, bool bFirst = false)
	{
		try
		{
			JSON_OBJECT_PTR pNode;
			JSON_OBJECT_PTR pLastObj;

			if (ObjIsNull(m_lastNode.m_pObject) == true)
			{
				if (ObjIsNull(m_lastNode.m_pParent) == true)
				{
					pNode = m_parseResult.extract<JSON_OBJECT_PTR>();
					pLastObj = 0;
				}
				else
				{
					pNode = m_lastNode.m_pParent;
					pLastObj = 0;
				}
			}
			else
			{
				pNode = m_lastNode.m_pObject;
				pLastObj = m_lastNode.m_pParent;
			}

			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			m_lastNode.m_eType = JSON_ObjectType_def::ObjectType_Array;
			m_lastNode.m_sName = sParam;
			m_lastNode.m_pParent = 0;
			m_lastNode.m_pObject = 0;
			m_lastNode.m_pArray = Var.extract<JSON_ARRAY_PTR>();

			return true;
		}
		catch (...)
		{

		}

		return false;
	}

	bool FindJsonParamString(JSON_OBJECT_PTR pNode, std::string sParam, bool bFirst = false)
	{
		try
		{
			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			if (Var.isString() == false)
			{
				return false;
			}

			m_lastNode.m_eType =	GetEntryType(Var);
			m_lastNode.m_sName =	sParam;
			m_lastNode.m_pParent =  0;
			m_lastNode.m_pObject =	pNode;
			m_lastNode.m_pArray =	0;

			return true;
		}
		catch(...)
		{

		}

		return false;
	}
	bool FindJsonParamString(std::string sParam, bool bFirst = false)
	{
		try
		{
			JSON_OBJECT_PTR pNode;
			JSON_OBJECT_PTR pLastObj;

			if (ObjIsNull(m_lastNode.m_pObject) == true)
			{
				if (ObjIsNull(m_lastNode.m_pParent) == true)
				{
					pNode = m_parseResult.extract<JSON_OBJECT_PTR>();
					pLastObj = 0;
				}
				else
				{
					pNode = m_lastNode.m_pParent;
					pLastObj = 0;
				}
			}
			else
			{
				pNode = m_lastNode.m_pObject;
				pLastObj = m_lastNode.m_pParent;
			}

			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			if (Var.isString() == false)
			{
				return false;
			}

			m_lastNode.m_eType =	GetEntryType(Var);
			m_lastNode.m_sName =	sParam;
			m_lastNode.m_pParent =	pLastObj;
			m_lastNode.m_pObject =	pNode;
			m_lastNode.m_pArray =	0;

			return true;
		}
		catch(...)
		{

		}

		return false;
	}

	JSON_OBJECT_PTR GetJsonParamGroup(JSON_OBJECT_PTR pNode, std::string sParam, bool bFirst = false)
	{
		try
		{
			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			m_lastNode.m_eType =	eType;
			m_lastNode.m_sName =	sParam;
			m_lastNode.m_pParent =	0;

			if (eType == JSON_ObjectType_def::ObjectType_Array)
			{
				m_lastNode.m_pArray = Var.extract<JSON_ARRAY_PTR>();
				m_lastNode.m_pObject = 0;
			}
			else
			{
				m_lastNode.m_pArray = 0;
				m_lastNode.m_pObject = Var.extract<JSON_OBJECT_PTR>();
			}

			return m_lastNode.m_pObject;
		}
		catch(...)
		{

		}

		return 0;
	}
	JSON_OBJECT_PTR  GetJsonParamGroup(std::string sParam, bool bFirst = false)
	{
		try
		{
			JSON_OBJECT_PTR pNode;
			JSON_OBJECT_PTR pLastObj;

			if (ObjIsNull(m_lastNode.m_pObject) == true)
			{
				if (ObjIsNull(m_lastNode.m_pParent) == true)
				{
					pNode = m_parseResult.extract<JSON_OBJECT_PTR>();
					pLastObj = 0;
				}
				else
				{
					pNode = m_lastNode.m_pParent;
					pLastObj = 0;
				}
			}
			else
			{
				pNode = m_lastNode.m_pObject;
				pLastObj = m_lastNode.m_pParent;
			}

			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			m_lastNode.m_eType =	eType;
			m_lastNode.m_sName =	sParam;
			m_lastNode.m_pParent =	pLastObj;

			if (eType == JSON_ObjectType_def::ObjectType_Array)
			{
				m_lastNode.m_pArray = Var.extract<JSON_ARRAY_PTR>();
				m_lastNode.m_pObject = 0;
			}
			else
			{
				m_lastNode.m_pArray = 0;
				m_lastNode.m_pObject = Var.extract<JSON_OBJECT_PTR>();
			}

			return m_lastNode.m_pObject;
		}
		catch (...)
		{

		}

		return 0;
	}

	long GetJsonArrayLen(JSON_ARRAY_PTR pArray)
	{
		try
		{
			if (ArrayIsNull(pArray) == true)
			{
				return -1;
			}

			return pArray->size();
		}
		catch (...)
		{

		}

		return -1;
	}
	long GetJsonArrayLen()
	{
		try
		{
			JSON_ARRAY_PTR pArray;

			if (m_lastNode.m_eType != JSON_ObjectType_def::ObjectType_Array)
			{
				return -1;
			}

			pArray = m_lastNode.m_pArray;

			if (ArrayIsNull(pArray) == true)
			{
				return -1;
			}

			return pArray->size();
		}
		catch (...)
		{

		}

		return -1;
	}


	JSON_OBJECT_PTR GetJsonObjectAtIdx(JSON_ARRAY_PTR pArray, unsigned int nIdx)
	{
		try
		{
			if (ArrayIsNull(pArray) == true)
			{
				return 0;
			}

			JSON_OBJECT_PTR pNode = GetObjectPtrAt(pArray, nIdx);

			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			return pNode;
		}
		catch (...)
		{

		}

		return false;
	}
	JSON_OBJECT_PTR GetJsonObjectAtIdx(unsigned int nIdx)
	{
		try
		{
			JSON_ARRAY_PTR pArray;

			if (m_lastNode.m_eType != JSON_ObjectType_def::ObjectType_Array)
			{
				return 0;
			}

			pArray = m_lastNode.m_pArray;

			if (ArrayIsNull(pArray) == true)
			{
				return 0;
			}

			JSON_OBJECT_PTR pNode = GetObjectPtrAt(pArray, nIdx);

			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			m_lastNode.m_eType =	JSON_ObjectType_def::ObjectType_Array;
			m_lastNode.m_sName =	"";
			m_lastNode.m_pParent =	m_lastNode.m_pObject;
			m_lastNode.m_pObject =	pNode;
			m_lastNode.m_pArray =	pArray;

			return pNode;
		}
		catch (...)
		{

		}

		return 0;
	}

	bool HasJsonChildNode(JSON_OBJECT_PTR  pNode);

	bool LoadJsonParamString
		(
			JSON_OBJECT_PTR pNode, 
			const std::string &sParam, 
			std::string &sTarget, 
			std::string sDefault = "", 
			bool bFirst = false
		)
	{
		try
		{
			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			switch (eType)
			{
			case JSON_ObjectType_def::ObjectType_String:
				sTarget = Var.toString();

				m_lastNode.m_eType =	eType;
				m_lastNode.m_sName =	sParam;
				m_lastNode.m_pParent =	0;
				m_lastNode.m_pObject =	pNode;
				m_lastNode.m_pArray =	0;

				return true;
			}
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadJsonParamString
		(
			const std::string &sParam,
			std::string &sTarget,
			std::string sDefault = "",
			bool bFirst = false
		)
	{
		try
		{
			JSON_OBJECT_PTR pNode;
			JSON_OBJECT_PTR pLastObj;

			if (ObjIsNull(m_lastNode.m_pObject) == true)
			{
				if (ObjIsNull(m_lastNode.m_pParent) == true)
				{
					pNode = m_parseResult.extract<JSON_OBJECT_PTR>();
					pLastObj = 0;
				}
				else
				{
					pNode = m_lastNode.m_pParent;
					pLastObj = 0;
				}
			}
			else
			{
				pNode = m_lastNode.m_pObject;
				pLastObj = m_lastNode.m_pParent;
			}

			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			switch (eType)
			{
			case JSON_ObjectType_def::ObjectType_String:
				sTarget = Var.toString();

				m_lastNode.m_eType =	eType;
				m_lastNode.m_sName =	sParam;
				m_lastNode.m_pParent =	pLastObj;
				m_lastNode.m_pObject =	pNode;
				m_lastNode.m_pArray =	0;

				return true;
			}
		}
		catch (...)
		{

		}

		return false;
	}

	bool LoadJsonParamBool
		(
			JSON_OBJECT_PTR  pNode, 
			const std::string &sParam, 
			bool *bTarget, 
			bool bUseDef = false,
			bool bDefVal = false, 
			bool bFirst = false
		)
	{
		if (bTarget == NULL)
		{
			return false;
		}

		try
		{
			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			switch (eType)
			{
			case JSON_ObjectType_def::ObjectType_Bool:
				*bTarget = (bool) Var;

				m_lastNode.m_eType =	eType;
				m_lastNode.m_sName =	sParam;
				m_lastNode.m_pParent =	0;
				m_lastNode.m_pObject =	pNode;
				m_lastNode.m_pArray =	0;

				return true;
			}
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadJsonParamBool
		(
			const std::string &sParam,
			bool *bTarget,
			bool bUseDef = false,
			bool bDefVal = false,
			bool bFirst = false
		)
	{
		if (bTarget == NULL)
		{
			return false;
		}

		try
		{
			JSON_OBJECT_PTR pNode;
			JSON_OBJECT_PTR pLastObj;

			if (ObjIsNull(m_lastNode.m_pObject) == true)
			{
				if (ObjIsNull(m_lastNode.m_pParent) == true)
				{
					pNode = m_parseResult.extract<JSON_OBJECT_PTR>();
					pLastObj = 0;
				}
				else
				{
					pNode = m_lastNode.m_pParent;
					pLastObj = 0;
				}
			}
			else
			{
				pNode = m_lastNode.m_pObject;
				pLastObj = m_lastNode.m_pParent;
			}

			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			switch (eType)
			{
			case JSON_ObjectType_def::ObjectType_Bool:
				*bTarget = (bool) Var;

				m_lastNode.m_eType =	eType;
				m_lastNode.m_sName =	sParam;
				m_lastNode.m_pParent =	pLastObj;
				m_lastNode.m_pObject =	pNode;
				m_lastNode.m_pArray =	0;

				return true;
			}
		}
		catch (...)
		{

		}

		return false;
	}

	bool LoadJsonParamInt
		(
			JSON_OBJECT_PTR pNode, 
			const std::string &sParam, 
			int *nTarget, 
			bool bUseDef = false,
			int nDefVal = 0,
			bool bFirst = false
		)
	{
		if (nTarget == NULL)
		{
			return false;
		}

		try
		{
			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			switch (eType)
			{
			case JSON_ObjectType_def::ObjectType_Int:
			case JSON_ObjectType_def::ObjectType_UInt:
			case JSON_ObjectType_def::ObjectType_Numeric:
				*nTarget = (int) Var;

				m_lastNode.m_eType =	eType;
				m_lastNode.m_sName =	sParam;
				m_lastNode.m_pParent =	0;
				m_lastNode.m_pObject =	pNode;
				m_lastNode.m_pArray =	0;

				return true;
			}
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadJsonParamInt
		(
			const std::string &sParam, 
			int *nTarget, 
			bool bUseDef = false,
			int nDefVal = 0,
			bool bFirst = false
		)
	{
		if (nTarget == NULL)
		{
			return false;
		}

		try
		{
			JSON_OBJECT_PTR pNode;
			JSON_OBJECT_PTR pLastObj;

			if (ObjIsNull(m_lastNode.m_pObject) == true)
			{
				if (ObjIsNull(m_lastNode.m_pParent) == true)
				{
					pNode = m_parseResult.extract<JSON_OBJECT_PTR>();
					pLastObj = 0;
				}
				else
				{
					pNode = m_lastNode.m_pParent;
					pLastObj = 0;
				}
			}
			else
			{
				pNode = m_lastNode.m_pObject;
				pLastObj = m_lastNode.m_pParent;
			}

			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			switch (eType)
			{
			case JSON_ObjectType_def::ObjectType_Int:
			case JSON_ObjectType_def::ObjectType_UInt:
			case JSON_ObjectType_def::ObjectType_Numeric:
				*nTarget = (int) Var;

				m_lastNode.m_eType =	eType;
				m_lastNode.m_sName =	sParam;
				m_lastNode.m_pParent =	pLastObj;
				m_lastNode.m_pObject =	pNode;
				m_lastNode.m_pArray =	0;

				return true;
			}
		}
		catch(...)
		{

		}

		return false;
	}

	bool LoadJsonParamLong
		(
			JSON_OBJECT_PTR pNode,
			const std::string &sParam,
			long *lTarget,
			bool bUseDef = false,
			long lDefVal = 0,
			bool bFirst = false
		)
	{
		if (lTarget == NULL)
		{
			return false;
		}

		try
		{
			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			switch (eType)
			{
			case JSON_ObjectType_def::ObjectType_Int:
			case JSON_ObjectType_def::ObjectType_UInt:
			case JSON_ObjectType_def::ObjectType_Numeric:
				*lTarget = (long) Var;

				m_lastNode.m_eType =	eType;
				m_lastNode.m_sName =	sParam;
				m_lastNode.m_pParent =	0;
				m_lastNode.m_pObject =	pNode;
				m_lastNode.m_pArray =	0;

				return true;
			}
		}
		catch (...)
		{

		}

		return false;
	}
	bool LoadJsonParamLong
		(
			const std::string &sParam,
			long *lTarget,
			bool bUseDef = false,
			long lDefVal = 0,
			bool bFirst = false
		)
	{
		if (lTarget == NULL)
		{
			return false;
		}

		try
		{
			JSON_OBJECT_PTR pNode;
			JSON_OBJECT_PTR pLastObj;

			if (ObjIsNull(m_lastNode.m_pObject) == true)
			{
				if (ObjIsNull(m_lastNode.m_pParent) == true)
				{
					pNode = m_parseResult.extract<JSON_OBJECT_PTR>();
					pLastObj = 0;
				}
				else
				{
					pNode = m_lastNode.m_pParent;
					pLastObj = 0;
				}
			}
			else
			{
				pNode = m_lastNode.m_pObject;
				pLastObj = m_lastNode.m_pParent;
			}

			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			switch (eType)
			{
			case JSON_ObjectType_def::ObjectType_Int:
			case JSON_ObjectType_def::ObjectType_UInt:
			case JSON_ObjectType_def::ObjectType_Numeric:
				*lTarget = (long) Var;

				m_lastNode.m_eType =	eType;
				m_lastNode.m_sName =	sParam;
				m_lastNode.m_pParent =	pLastObj;
				m_lastNode.m_pObject =	pNode;
				m_lastNode.m_pArray =	0;

				return true;
			}
		}
		catch (...)
		{

		}

		return false;
	}

	bool GetFirstChildNode(JSON_OBJECT_PTR pNode)
	{
		if (m_pJsonParser == NULL)
		{
			return false;
		}

		try
		{
			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, "", true);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			m_lastNode.m_eType = eType;
			m_lastNode.m_sName = "";
			m_lastNode.m_pParent = 0;

			if (eType == JSON_ObjectType_def::ObjectType_Array)
			{
				m_lastNode.m_pArray = Var.extract<JSON_ARRAY_PTR>();
				m_lastNode.m_pObject = 0;
			}
			else
			{
				m_lastNode.m_pArray = 0;
				m_lastNode.m_pObject = Var.extract<JSON_OBJECT_PTR>();
			}

			return true;
		}
		catch (...)
		{
			;
		}

		return false;
	}

	bool GetNextChildNode()
	{
		if (m_pJsonParser == NULL)
		{
			return false;
		}

		try
		{
			JSON_VAR Var = FindEntryPtr(0, "", false);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			JSON_ObjectType_def eType = GetEntryType(Var);

			m_lastNode.m_eType = eType;
			m_lastNode.m_sName = "";
			m_lastNode.m_pParent = 0;

			if (eType == JSON_ObjectType_def::ObjectType_Array)
			{
				m_lastNode.m_pArray = Var.extract<JSON_ARRAY_PTR>();
				m_lastNode.m_pObject = 0;
			}
			else
			{
				m_lastNode.m_pArray = 0;
				m_lastNode.m_pObject = Var.extract<JSON_OBJECT_PTR>();
			}

			return true;
		}
		catch (...)
		{
			;
		}

		return false;
	}

	JSON_OBJECT_PTR GetLastNode()
	{
		return m_lastNode.m_pObject;
	}

	JSON_OBJECT_PTR GetLastParent()
	{
		return m_lastNode.m_pParent;
	}

	JSON_ARRAY_PTR GetLastArray()
	{
		return m_lastNode.m_pArray;
	}

	JsonEntryInfo_def GetLastNodeInfo()
	{
		JsonEntryInfo_def ret;

		ret.copy(m_lastNode);

		return ret;
	}

	void SetLastNodeInfo(JsonEntryInfo_def &info)
	{
		m_lastNode.copy(info);
	}

	JSON_OBJECT_ITR GetLastNodeItr()
	{
		return m_itObject;
	}

	void SetLastNodeItr(JSON_OBJECT_ITR it)
	{
		m_itObject = it;
	}

	JSON_VAR_ITR GetLastEntryItr()
	{
		if (m_pItEntry == 0)
		{
			static JSON_VAR emptyVar;
			return JSON_VAR_ITR(&emptyVar, true);
		}

		return (*m_pItEntry);
	}

	void SetLastEntryItr(JSON_VAR_ITR it)
	{
		if (m_pItEntry == 0)
		{
			m_pItEntry = new JSON_VAR_ITR(it);
			return;
		}

		(*m_pItEntry) = it;
	}

#if 0
	bool GetFirstChild(JSON_OBJECT_PTR pNode, JSON_VAR_PTR &pVar)
	{
		if (ObjIsNull(pNode) == true)
		{
			return false;
		}

		JSON_VAR_PTR pFound;

		if (GetJsonChild(pFound, true) == true)
		{
			pVar = pFound;

			return true;
		}

		return false;
	}

	bool  GetNextChild(JSON_OBJECT_PTR pNode, JSON_VAR_PTR &pVar)
	{
		if (ObjIsNull(pNode) == true)
		{
			return false;
		}

		JSON_VAR_PTR pFound;

		if (GetJsonChild(pFound, false) == true)
		{
			pVar = pFound;

			return true;
		}

		return false;
	}
#endif
#if 0
	void AppendNode(JSON_OBJECT_PTR child)
	{
		if (m_pJsonParser == NULL || child == NULL)
		{
			return;
		}

		try
		{

		}
		catch (...)
		{

		}
	}

	void AppendNode(JSON_OBJECT_PTR parent, JSON_OBJECT_PTR child)
	{
		if (m_pJsonParser == NULL || child == NULL)
		{
			return;
		}

		try
		{
			if (parent == NULL)
			{

			}
			else
			{

			}
			m_pLastNode = child;
		}
		catch (...)
		{

		}
	}

	JSON_OBJECT_PTR  NewStringNode(const std::string &sName, const std::string &sValue = "")
	{
		try
		{

		}
		catch (...)
		{
			
		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewStringNode(JSON_OBJECT_PTR pParent, const std::string &sName, const std::string &sValue = "")
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{
			
		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewIntNode(const std::string &sName, int nValue)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{
			
		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewIntNode(JSON_OBJECT_PTR pParent, const std::string &sName, int nValue)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{
			
		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewUIntNode(const std::string &sName, unsigned int nValue)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{

		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewUIntNode(JSON_OBJECT_PTR pParent, const std::string &sName, unsigned int nValue)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{

		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewLongNode(const std::string &sName, long lValue)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{
			
		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewlongNode(JSON_OBJECT_PTR pParent, const std::string &sName, long lValue)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{
			
		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewULongNode(const std::string &sName, unsigned long lValue)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{

		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewUlongNode(JSON_OBJECT_PTR pParent, const std::string &sName, unsigned long lValue)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{

		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewDWordNode(const std::string &sName, long long dwValue)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{

		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewDWordNode(JSON_OBJECT_PTR pParent, const std::string &sName, long long dwValue)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{

		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewBoolNode(const std::string &sName, bool bValue)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{
			
		}

		return NULL;
	}

	JSON_OBJECT_PTR  NewBoolNode(JSON_OBJECT_PTR pParent, const std::string &sName, bool bValue)
	{
		if (m_pJsonParser == NULL)
		{
			return NULL;
		}

		try
		{

		}
		catch (...)
		{
			
		}

		return NULL;
	}
#endif

	bool GetNodeName(JSON_OBJECT_ITR it, std::string &sName)
	{
		try
		{
			sName = it->first;

			return true;
		}
		catch (...)
		{

		}

		return false;
	}

	bool GetNodeValue(JSON_OBJECT_ITR it, std::string &sVal)
	{
		try
		{
			sVal = it->second.toString();

			return true;
		}
		catch (...)
		{

		}

		return false;
	}

	bool GetLastNodeName(std::string &sName)
	{
		try
		{
			sName = m_lastNode.m_sName;

			return true;
		}
		catch (...)
		{
			
		}

		return false;
	}

	//bool GetLastNodeValue(std::string &sValue)
	//{
	//	try
	//	{
	//		sValue = m_lastNode.m_pObject.get();

	//		return true;
	//	}
	//	catch (...)
	//	{
	//		
	//	}

	//	return false;
	//}

	bool GetNodeValue(JSON_OBJECT_PTR pNode, const std::string &sParam, std::string &sVal, bool bFirst = true)
	{
		try
		{
			if (ObjIsNull(pNode) == true)
			{
				return false;
			}

			JSON_VAR Var = FindEntryPtr(pNode, sParam, bFirst);

			if (Var.isEmpty() == true)
			{
				return false;
			}

			m_lastNode.m_eType =	GetEntryType(Var);
			m_lastNode.m_sName =	"";
			m_lastNode.m_pObject =	pNode;
			m_lastNode.m_pParent =	0;

			sVal = Var.toString();

			return true;
		}
		catch (...)
		{

		}

		return false;
	}


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
};

#if 0
class JsonWriterUtil
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

	JsonWriterUtil();
	~JsonWriterUtil();

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



#endif //  PocoJsonUtils_H_
