//**************************************************************************************************
//* FILE:		PocoXmlUtils.h
//*
//* DESCRIP:	
//*
//*


#define _CRT_SECURE_NO_WARNINGS


#ifndef PocoXmlUtils_H_
#define PocoXmlUtils_H_

#include <string>
#include <ostream>
#include <stdexcept>
//#include <vector>
//#include <map>


#include "Poco/SAX/DeclHandler.h"

#include "stringUtils.h"


using namespace std;

using namespace Poco::SAX;


#define XML_TYPE				char

#define MAX_XML_BUFFER_SIZE		20480		// this is the maximum size buffer we can send via a "weblet"

#define LoadXmlAttributeString	LoadXmlSubParamString

#define DEFAULT_XML_DUMP_FILE	"XML_Dump.txt"


#define POCO_XML_PARSER			Poco::

#define POCO_XML_NODE			Poco::XML::Node
#define POCO_XML_NODE_PTR		Poco::XML::Node*


class PocoXmlParserUtil
{
private:

	POCO_XML_PARSER			*m_pPocoXmlParser;

	POCO_XML_NODE	 		*m_pBaseNode;
	POCO_XML_NODE			*m_pLastNode;

	char					*m_pXmlBuffer;

	long					m_lXmlBufLen;

	bool					m_bXmlLoadedFromFile;
	bool					m_bXmlLoadedFromBuf;

	//int					m_nParseFlags;

	bool					m_bDumpXmlOnError;

	std::string				m_sXmlDumpFile;

public:

	PocoXmlParserUtil() :
		m_pPocoXmlParser(NULL),
		m_pXmlBuffer(NULL),
		m_lXmlBufLen(0),
		m_bXmlLoadedFromFile(false),
		m_bXmlLoadedFromBuf(false)
	{
		Init();
	}

	PocoXmlParserUtil(bool bDumpOnError) :
		m_pPocoXmlParser(NULL),
		m_pXmlBuffer(NULL),
		m_lXmlBufLen(0),
		m_bXmlLoadedFromFile(false),
		m_bXmlLoadedFromBuf(false)
	{
		Init();

		SetDumpOnError(bDumpOnError);
	}

	PocoXmlParserUtil(bool bDumpOnError, const char *pDumpFile) :
		m_pPocoXmlParser(NULL),
		m_pXmlBuffer(NULL),
		m_lXmlBufLen(0),
		m_bXmlLoadedFromFile(false),
		m_bXmlLoadedFromBuf(false)
	{
		Init();

		SetDumpOnError(bDumpOnError, pDumpFile);
	}


	PocoXmlParserUtil(bool bDumpOnError, const std::string &sDumpFile) :
		m_pPocoXmlParser(NULL),
		m_pXmlBuffer(NULL),
		m_lXmlBufLen(0),
		m_bXmlLoadedFromFile(false),
		m_bXmlLoadedFromBuf(false)
	{
		Init();

		SetDumpOnError(bDumpOnError, sDumpFile);
	}

	~PocoXmlParserUtil()
	{
		DeInit();
	}

	void Init();
	void DeInit();

	void SetDumpOnError(bool bVal)
	{
		m_bDumpXmlOnError = bVal;
	}
	void SetDumpOnError(bool bDumpOnError, const char *pDumpFile)
	{
		SetDumpOnError(bDumpOnError);
		SetDumpFile(pDumpFile);
	}
	void SetDumpOnError(bool bDumpOnError, const std::string &sDumpFile)
	{
		SetDumpOnError(bDumpOnError, (const char *) sDumpFile.c_str());
	}

	void SetDumpFile(const char *pDumpFile)
	{
		if (pDumpFile == NULL)
		{
			m_sXmlDumpFile = DEFAULT_XML_DUMP_FILE;
		}
		else
		{
			m_sXmlDumpFile = pDumpFile;
		}
	}
	void SetDumpFile(std::string &sDumpFile)
	{
		SetDumpFile((const char *) sDumpFile.c_str());
	}

	void Clear()
	{
		try 
		{
			m_pPocoXmlParser->clear();
		}
		catch (...)
		{
			return;
		}
	}

	//long GetXmlLength();

	//void SetXmlBuffer(char *pBuffer);
	//void SetXmlBuffer(std::string sBuffer)
	//{
	//	SetXmlBuffer((char *) (sBuffer.c_str()));
	//}

	POCO_XML_NODE* GetXmlBaseNode(char *pParam);
	POCO_XML_NODE* GetXmlBaseNode(const std::string &sParam)
	{
		return GetXmlBaseNode(sParam.c_str());
	}

	POCO_XML_NODE* Create()
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try 
		{
			//xml_attribute<> *attr = m_pPocoXmlParser->allocate_attribute(name, value);

			POCO_XML_NODE *newNode = m_pPocoXmlParser->allocate_node(node_element, NULL, NULL);
			if (newNode != NULL)
			{
				m_pPocoXmlParser->append_node(newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			return NULL;
		}
	}

	POCO_XML_NODE* Create(const std::string &sName)
	{
		return Create(sName.c_str());
	}
	POCO_XML_NODE* Create(char *pName)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = m_pPocoXmlParser->allocate_string(pName);
			POCO_XML_NODE *newNode = m_pPocoXmlParser->allocate_node(node_element, nodeName, NULL);
			if (newNode != NULL)
			{
				m_pPocoXmlParser->append_node(newNode);
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			return NULL;
		}
	}

	POCO_XML_NODE* Create(const std::string &sName, char *pValue)
	{
		return Create(sName.c_str(), pValue);
	}
	POCO_XML_NODE* Create(char *pName, char *pValue)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = m_pPocoXmlParser->allocate_string(pName);
			POCO_XML_NODE *newNode = m_pPocoXmlParser->allocate_node(node_element, nodeName, pValue);
			if (newNode != NULL)
			{
				m_pPocoXmlParser->append_node(newNode);
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			return NULL;
		}
	}

	//bool AllocXmlBuffer(int nLen) throw(std::runtime_error);
	bool AllocXmlBuffer(int nLen);

	void FreeXmlBuffer()
	{
		try
		{
			if (m_pXmlBuffer != NULL)
			{
				//* if the buffer is not currently NULL... free it

				free(m_pXmlBuffer);
			}
		}
		catch(...)
		{

		}

		m_pXmlBuffer = NULL;

		m_bXmlLoadedFromFile = false;
		m_bXmlLoadedFromBuf = false;
	}

	char * GetXmlBuffer()
	{
		return m_pXmlBuffer;
	}

	bool Parse();

	bool LoadXmlBuffer(char *pBuffer, int nLen) throw(std::runtime_error);
	bool LoadXmlBuffer(std::string sBuffer) throw(std::runtime_error)
	{
		try
		{
			char *pBuf = (char *) (sBuffer.c_str());
			long nLen = (long) (sBuffer.length());
			return LoadXmlBuffer(pBuf, nLen);
		}
		catch(...)
		{

		}

		return false;
	}

	bool LoadXmlFile(char *pFolder, char *pFileName) throw(std::runtime_error);
	bool LoadXmlFile(std::string sFolder, std::string sFileName) throw(std::runtime_error)
	{
		try
		{
			char *pDir = (char *) (sFolder.c_str());
			char *pFile = (char *) (sFileName.c_str());
			return LoadXmlFile(pDir, pFile);
		}
		catch(...)
		{

		}

		return false;
	}

	bool WriteXmlFile(char *pFolder, char *pFileName) throw(std::runtime_error);
	bool WriteXmlFile(std::string sFolder, std::string sFileName) throw(std::runtime_error)
	{
		try
		{
			char *pDir = (char *) (sFolder.c_str());
			char *pFile = (char *) (sFileName.c_str());
			return WriteXmlFile(pDir, pFile);
		}
		catch(...)
		{

		}

		return false;
	}

	bool DumpXmlBuffer(char *pFolder = NULL, char *pFileName = NULL, bool bSpaceFill = false) throw(std::runtime_error);
	bool DumpXmlBuffer(std::string sFolder, std::string sFileName = "", bool bSpaceFill = false) throw(std::runtime_error)
	{
		try
		{
			char *pDir = (char *) (sFolder.c_str());
			char *pFile = (char *) (sFileName.c_str());
			return DumpXmlBuffer(pDir, pFile);
		}
		catch(...)
		{

		}

		return false;
	}

	bool FindXmlParamGroup(POCO_XML_NODE* pNode, char *pParam, bool bFirst = false);
	bool FindXmlParamGroup(POCO_XML_NODE* pNode, std::string sParam, bool bFirst = false)
	{
		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return FindXmlParamGroup(pNode, pPrm, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool FindXmlParamGroup(char *pParam, bool bFirst = false)
	{
		if (pParam == NULL)
		{
			return false;
		}

		try
		{
			return FindXmlParamGroup(m_pLastNode, pParam, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool FindXmlParamGroup(std::string sParam, bool bFirst = false)
	{
		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return FindXmlParamGroup(m_pLastNode, pPrm, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}

	bool FindXmlParamString(POCO_XML_NODE* pNode, char *pParam, bool bFirst = false)
	{
		if (pNode == NULL || pParam == NULL)
		{
			return false;
		}

		try
		{
			return FindXmlParamGroup(pNode, pParam, bFirst);
		}
		catch(...)
		{

		}

		return false;

	}
	bool FindXmlParamString(POCO_XML_NODE* pNode, std::string sParam, bool bFirst = false)
	{
		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return FindXmlParamGroup(pNode, pPrm, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool FindXmlParamString(char *pParam, bool bFirst = false)
	{
		if (pParam == NULL)
		{
			return false;
		}

		try
		{
			return FindXmlParamGroup(m_pLastNode, pParam, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool FindXmlParamString(std::string sParam, bool bFirst = false)
	{
		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return FindXmlParamGroup(m_pLastNode, pPrm, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}

	POCO_XML_NODE* GetXmlParamGroup(POCO_XML_NODE* pNode, char *pParam, bool bFirst = false);
	POCO_XML_NODE* GetXmlParamGroup(POCO_XML_NODE* pNode, std::string sParam, bool bFirst = false)
	{
		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return GetXmlParamGroup(pNode, pPrm, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	POCO_XML_NODE* GetXmlParamGroup(char *pParam, bool bFirst = false)
	{
		if (pParam == NULL)
		{
			return false;
		}

		try
		{
			return GetXmlParamGroup(m_pLastNode, pParam, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	POCO_XML_NODE* GetXmlParamGroup(std::string sParam, bool bFirst = false)
	{
		char *pPrm = (char *) (sParam.c_str());
		return GetXmlParamGroup(m_pLastNode, pPrm, bFirst);
	}

	bool HasXmlChildNode(POCO_XML_NODE* pNode);

	POCO_XML_NODE* GetFirstXmlChild(POCO_XML_NODE* pNode);

	POCO_XML_NODE* GetNextXmlSibbling(POCO_XML_NODE* pNode, char *pParam);
	POCO_XML_NODE* GetNextXmlSibbling(POCO_XML_NODE* pNode, std::string sParam)
	{
		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return GetNextXmlSibbling(pNode, pPrm);
		}
		catch(...)
		{

		}

		return false;
	}
	POCO_XML_NODE* GetNextXmlSibbling(POCO_XML_NODE* pNode)
	{
		return GetNextXmlSibbling(pNode, NULL);
	}

	bool LoadXmlParamBuf(POCO_XML_NODE* pNode, char *pParam, char *pTarget, int nLen, char *pDefault, bool bFirst = false);
	bool LoadXmlParamBuf(POCO_XML_NODE* pNode, std::string sParam, char *pTarget, int nLen, char *pDefault, bool bFirst = false)
	{
		if (pNode == NULL || pTarget == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			LoadXmlParamBuf(pNode, pPrm, pTarget, nLen, pDefault, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadXmlParamBuf(char *pParam, char *pTarget, int nLen, char *pDefault, bool bFirst = false)
	{
		if (pParam == NULL || pTarget == NULL)
		{
			return false;
		}

		try
		{
			LoadXmlParamBuf(m_pLastNode, pParam, pTarget, nLen, pDefault, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadXmlParamBuf(std::string sParam, char *pTarget, int nLen, char *pDefault, bool bFirst = false)
	{
		if (pTarget == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			LoadXmlParamBuf(m_pLastNode, pPrm, pTarget, nLen, pDefault, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}

	bool LoadXmlParamString(POCO_XML_NODE* pNode, char *pParam, std::string &sTarget, char *pDefault, bool bFirst = false);
	bool LoadXmlParamString(POCO_XML_NODE* pNode, std::string sParam, std::string &sTarget, char *pDefault, bool bFirst = false)
	{
		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return LoadXmlParamString(pNode, pPrm, sTarget, pDefault, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadXmlParamString(char *pParam, std::string &sTarget, char *pDefault, bool bFirst = false)
	{
		if (pParam == NULL)
		{
			return false;
		}

		try
		{
			return LoadXmlParamString(m_pLastNode, pParam, sTarget, pDefault, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadXmlParamString(std::string sParam, std::string &sTarget, char *pDefault, bool bFirst = false)
	{
		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return LoadXmlParamString(m_pLastNode, pPrm, sTarget, pDefault, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}

	bool LoadXmlSubParamString(POCO_XML_NODE* pNode, char *pSubParam, std::string &sTarget, char *pDefault);
	bool LoadXmlSubParamString(POCO_XML_NODE* pNode, std::string sSubParam, std::string &sTarget, char *pDefault)
	{
		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			char *pSubPrm = (char *) (sSubParam.c_str());
			return LoadXmlSubParamString(pNode, pSubPrm, sTarget, pDefault);
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadXmlSubParamString(char *pSubParam, std::string &sTarget, char *pDefault)
	{
		if (pSubParam == NULL)
		{
			return false;
		}

		try
		{
			return LoadXmlSubParamString(m_pLastNode, pSubParam, sTarget, pDefault);
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadXmlSubParamString(std::string sSubParam, std::string &sTarget, char *pDefault)
	{
		try
		{
			char *pSubPrm = (char *) (sSubParam.c_str());
			return LoadXmlSubParamString(m_pLastNode, pSubPrm, sTarget, pDefault);
		}
		catch(...)
		{

		}

		return false;
	}

	bool LoadXmlParamBool(POCO_XML_NODE* pNode, char *pParam, bool *bTarget, bool bUseDef, bool bDefVal, bool bFirst = false);
	bool LoadXmlParamBool(POCO_XML_NODE* pNode, std::string sParam, bool *bTarget, bool bUseDef, bool bDefVal, bool bFirst = false)
	{
		if (pNode == NULL || bTarget == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return LoadXmlParamBool(pNode, pPrm, bTarget, bUseDef, bDefVal, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadXmlParamBool(char *pParam, bool *bTarget, bool bUseDef, bool bDefVal, bool bFirst = false)
	{
		if (pParam == NULL || bTarget == NULL)
		{
			return false;
		}

		try
		{
			return LoadXmlParamBool(m_pLastNode, pParam, bTarget, bUseDef, bDefVal, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadXmlParamBool(std::string sParam, bool *bTarget, bool bUseDef, bool bDefVal, bool bFirst = false)
	{
		if (bTarget == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return LoadXmlParamBool(m_pLastNode, pPrm, bTarget, bUseDef, bDefVal, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}

	bool LoadXmlParamInt(POCO_XML_NODE* pNode, char *pParam, int *nTarget, bool bUseDef, int nDefVal, bool bFirst = false);
	bool LoadXmlParamInt(POCO_XML_NODE* pNode, std::string sParam, int *nTarget, bool bUseDef, int nDefVal, bool bFirst = false)
	{
		if (pNode == NULL || nTarget == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return LoadXmlParamInt(pNode, pPrm, nTarget, bUseDef, nDefVal, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadXmlParamInt(char *pParam, int *nTarget, bool bUseDef, int nDefVal, bool bFirst = false)
	{
		if (pParam == NULL || nTarget == NULL)
		{
			return false;
		}

		try
		{
			return LoadXmlParamInt(m_pLastNode, pParam, nTarget, bUseDef, nDefVal, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadXmlParamInt(std::string sParam, int *nTarget, bool bUseDef, int nDefVal, bool bFirst = false)
	{
		if (nTarget == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return LoadXmlParamInt(m_pLastNode, pPrm, nTarget, bUseDef, nDefVal, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}

	bool LoadXmlParamLong(POCO_XML_NODE* pNode, char *pParam, long *lTarget, bool bUseDef, long lDefVal, bool bFirst = false);
	bool LoadXmlParamLong(POCO_XML_NODE* pNode, std::string sParam, long *lTarget, bool bUseDef, long *lDefVal, bool bFirst = false)
	{
		if (pNode == NULL || lTarget == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return LoadXmlParamLong(pNode, pPrm, lTarget, bUseDef, lDefVal, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadXmlParamLong(char *pParam, long *lTarget, bool bUseDef, long *lDefVal, bool bFirst = false)
	{
		if (pParam == NULL || lTarget == NULL)
		{
			return false;
		}

		try
		{
			return LoadXmlParamLong(m_pLastNode, pParam, lTarget, bUseDef, lDefVal, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}
	bool LoadXmlParamLong(std::string sParam, long *lTarget, bool bUseDef, long *lDefVal, bool bFirst = false)
	{
		if (lTarget == NULL)
		{
			return false;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return LoadXmlParamLong(m_pLastNode, pPrm, lTarget, bUseDef, lDefVal, bFirst);
		}
		catch(...)
		{

		}

		return false;
	}

	POCO_XML_NODE* GetLastNode()
	{
		return m_pLastNode;
	}

	void SetLastNode(POCO_XML_NODE* pNode)
	{
		m_pLastNode = pNode;
	}

	POCO_XML_NODE* Parent(POCO_XML_NODE* xmlNode)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		return xmlNode->parent();
	}

	bool NextSibbling(POCO_XML_NODE* pNode)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return false;
		}

		try
		{
			POCO_XML_NODE* pNext =
				pNode->next_sibling(0, 0, false);

			if (pNext != NULL)
			{
				m_pLastNode = pNext;

				return true;
			}
		}
		catch (...)
		{
			;
		}

		return false;
	}
	bool NextSibbling()
	{
		try
		{
			return NextSibbling(m_pLastNode);
		}
		catch (...)
		{
			;
		}

		return false;
	}

	POCO_XML_NODE *AllocNode(node_type type, char *name, char *value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pPocoXmlParser->allocate_string(name);
			}
			if (value != NULL)
			{
				nodeValue = m_pPocoXmlParser->allocate_string(value);
			}
			POCO_XML_NODE *newNode = m_pPocoXmlParser->allocate_node(type, nodeName, nodeValue);
			if (newNode != NULL)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE *AllocNode(node_type type, std::string name, char *value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return AllocNode(type, pName, value);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE *AllocStringNode(char *name, char *value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pPocoXmlParser->allocate_string(name);
			}
			if (value != NULL)
			{
				nodeValue = m_pPocoXmlParser->allocate_string(value);
			}
			POCO_XML_NODE *newNode = m_pPocoXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != NULL)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE *AllocStringNode(std::string name, char *value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return AllocStringNode(pName, value);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE *AllocIntNode(char *name, int value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pPocoXmlParser->allocate_string(name);
			}

			string sTmp = stringUtil::tos(value);
			
			nodeValue = m_pPocoXmlParser->allocate_string((char *) sTmp.c_str());
			POCO_XML_NODE *newNode = m_pPocoXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != NULL)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE *AllocIntNode(std::string name, int value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return AllocIntNode(pName, value);
		}
		catch (...)
		{
			
		}

		return NULL;
	}


	POCO_XML_NODE *AllocUIntNode(char *name, unsigned int value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pPocoXmlParser->allocate_string(name);
			}

			string sTmp = stringUtil::tos(value);

			nodeValue = m_pPocoXmlParser->allocate_string((char *)sTmp.c_str());
			POCO_XML_NODE *newNode = m_pPocoXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != NULL)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}
	POCO_XML_NODE *AllocUIntNode(std::string name, unsigned int value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *)(name.c_str());
			return AllocUIntNode(pName, value);
		}
		catch (...)
		{

		}

		return NULL;
	}

	POCO_XML_NODE *AllocLongNode(char *name, long value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pPocoXmlParser->allocate_string(name);
			}

			string sTmp = stringUtil::tos(value);
			
			nodeValue = m_pPocoXmlParser->allocate_string((char *) sTmp.c_str());
			POCO_XML_NODE *newNode = m_pPocoXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != NULL)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE *AllocLongNode(std::string name, long value)
	{
		try
		{
			char *pName = (char *) (name.c_str());
			return AllocLongNode(pName, value);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE *AllocULongNode(char *name, unsigned long value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pPocoXmlParser->allocate_string(name);
			}

			string sTmp = stringUtil::tos(value);

			nodeValue = m_pPocoXmlParser->allocate_string((char *) sTmp.c_str());
			POCO_XML_NODE *newNode = m_pPocoXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != NULL)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}
	POCO_XML_NODE *AllocLongNode(std::string name, unsigned long value)
	{
		try
		{
			char *pName = (char *)(name.c_str());
			return AllocULongNode(pName, value);
		}
		catch (...)
		{

		}

		return NULL;
	}

	POCO_XML_NODE *AllocDWordNode(char *name, long long value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pPocoXmlParser->allocate_string(name);
			}

			string sTmp = stringUtil::tos(value);

			nodeValue = m_pPocoXmlParser->allocate_string((char *) sTmp.c_str());
			POCO_XML_NODE *newNode = m_pPocoXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != NULL)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}
	POCO_XML_NODE *AllocDWordNode(std::string name, long long value)
	{
		try
		{
			char *pName = (char *)(name.c_str());
			return AllocDWordNode(pName, value);
		}
		catch (...)
		{

		}

		return NULL;
	}

	POCO_XML_NODE *AllocBoolNode(char *name, bool value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pPocoXmlParser->allocate_string(name);
			}
			if (value == true)
			{
				nodeValue = m_pPocoXmlParser->allocate_string("true");
			}
			else
			{
				nodeValue = m_pPocoXmlParser->allocate_string("false");
			}
			POCO_XML_NODE *newNode = m_pPocoXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != NULL)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE *AllocBoolNode(std::string name, bool value)
	{
		try
		{
			char *pName = (char *) (name.c_str());
			return AllocBoolNode(pName, value);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	void AppendNode(POCO_XML_NODE *child)
	{
		if (m_pPocoXmlParser == NULL || child == NULL)
		{
			return;
		}

		try
		{
			m_pPocoXmlParser->append_node(child);
			m_pLastNode = child;
		}
		catch (...)
		{

		}
	}

	void AppendNode(POCO_XML_NODE *parent, POCO_XML_NODE *child)
	{
		if (m_pPocoXmlParser == NULL || child == NULL)
		{
			return;
		}

		try
		{
			if (parent == NULL)
			{
				m_pPocoXmlParser->append_node(child);
			}
			else
			{
				parent->append_node(child);
			}
			m_pLastNode = child;
		}
		catch (...)
		{

		}
	}

	POCO_XML_NODE * NewNode(char *name, char *value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocStringNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE * NewNode(POCO_XML_NODE *parent, char *name, char *value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocStringNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE * NewStringNode(char *name, char *value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocStringNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE * NewStringNode(std::string name, std::string value)
	{
		try
		{
			char *pName = (char *) (name.c_str());
			char *pVal = (char *) (value.c_str());
			return NewStringNode(pName, pVal);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE * NewStringNode(POCO_XML_NODE *parent, char *name, char *value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocStringNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE * NewStringNode(POCO_XML_NODE *parent, std::string name, std::string value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			char *pVal = (char *) (value.c_str());
			return NewStringNode(parent, pName, pVal);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE * NewIntNode(char *name, int value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocIntNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE * NewIntNode(std::string name, int value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewIntNode(pName, value);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE * NewIntNode(POCO_XML_NODE *parent, char *name, int value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocIntNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE * NewIntNode(POCO_XML_NODE *parent, std::string name, int value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewIntNode(parent, pName, value);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE * NewUIntNode(char *name, unsigned int value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocUIntNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}
	POCO_XML_NODE * NewUIntNode(std::string name, unsigned int value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewUIntNode(pName, value);
		}
		catch (...)
		{

		}

		return NULL;
	}

	POCO_XML_NODE * NewUIntNode(POCO_XML_NODE *parent, char *name, unsigned int value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocUIntNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}
	POCO_XML_NODE * NewUIntNode(POCO_XML_NODE *parent, std::string name, unsigned int value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewUIntNode(parent, pName, value);
		}
		catch (...)
		{

		}

		return NULL;
	}

	POCO_XML_NODE * NewLongNode(char *name, long value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocLongNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE * NewLongNode(std::string name, long value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewLongNode(pName, value);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE * NewLongNode(POCO_XML_NODE *parent, char *name, long value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocLongNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE * NewlongNode(POCO_XML_NODE *parent, std::string name, long value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewLongNode(parent, pName, value);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE * NewULongNode(char *name, unsigned long value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocULongNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}
	POCO_XML_NODE * NewULongNode(std::string name, unsigned long value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewULongNode(pName, value);
		}
		catch (...)
		{

		}

		return NULL;
	}

	POCO_XML_NODE * NewULongNode(POCO_XML_NODE *parent, char *name, unsigned long value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocULongNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}
	POCO_XML_NODE * NewUlongNode(POCO_XML_NODE *parent, std::string name, unsigned long value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewULongNode(parent, pName, value);
		}
		catch (...)
		{

		}

		return NULL;
	}

	POCO_XML_NODE * NewDWordNode(char *name, long long value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocDWordNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}
	POCO_XML_NODE * NewDWordNode(std::string name, long long value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewDWordNode(pName, value);
		}
		catch (...)
		{

		}

		return NULL;
	}

	POCO_XML_NODE * NewDWordNode(POCO_XML_NODE *parent, char *name, long long value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocDWordNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{

		}

		return NULL;
	}
	POCO_XML_NODE * NewDWordNode(POCO_XML_NODE *parent, std::string name, long long value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *)(name.c_str());
			return NewDWordNode(parent, pName, value);
		}
		catch (...)
		{

		}

		return NULL;
	}

	POCO_XML_NODE * NewBoolNode(char *name, bool value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			POCO_XML_NODE *newNode = AllocBoolNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE * NewBoolNode(std::string name, bool value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewBoolNode(pName, value);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE * NewBoolNode(POCO_XML_NODE *parent, char *name, bool value)
	{
		if (m_pPocoXmlParser == NULL || name == NULL)
		{
			return NULL;
		}


		try
		{
			POCO_XML_NODE *newNode = AllocBoolNode(name, value);
			if (newNode != NULL)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return NULL;
	}
	POCO_XML_NODE * NewBoolNode(POCO_XML_NODE *parent, std::string name, bool value)
	{
		if (m_pPocoXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewBoolNode(parent, pName, value);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	POCO_XML_NODE * FindNext(POCO_XML_NODE * pNode, char *name)
	{
		if (pNode == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			return pNode->next_sibling(name, 0, true);
		}
		catch (...)
		{
			
		}

		return NULL;
	}

	bool GetNodeName(POCO_XML_NODE * pNode, std::string &name)
	{
		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			name.assign(pNode->name(), pNode->name_size());
			m_pLastNode = pNode;
			return true;
		}
		catch (...)
		{
			
		}

		return false;
	}

	bool GetNodeValue(POCO_XML_NODE * pNode, std::string &value)
	{
		if (pNode == NULL)
		{
			return false;
		}

		try
		{
			value.assign(pNode->value(), pNode->value_size());
			m_pLastNode = pNode;
			return true;
		}
		catch (...)
		{
			
		}

		return false;
	}

	//POCO_XML_NODE * FindChild(POCO_XML_NODE * xmlNode, char *name)
	//{
	//	return xmlNode->
	//}

	void XmlOutput(char *target, POCO_XML_NODE* xmlNode, bool bAddHeader = false)
	{
		if (m_pPocoXmlParser == NULL || target == NULL || xmlNode == NULL)
		{
			return;
		}

		try
		{
			long nLen = 0;

			if (bAddHeader == true)
			{
				sprintf(target, "<?xml version=\"1.0\" encoding=\"utf-8\"?> \n ");
				nLen = (long) strlen(target);
			}

			print((target + nLen), *xmlNode, 0);
		}
		catch(...)
		{

		}
	}

	void XmlOutput(std::string &target, POCO_XML_NODE* xmlNode, bool bAddHeader = false)
	{
		if (xmlNode == NULL)
		{
			return;
		}

		try
		{
#if 0
			char szBuffer[MAX_XML_BUFFER_SIZE];

			memset(szBuffer, 0, sizeof(szBuffer));

			int nLen = 0;

			if (bAddHeader == true)
			{
				sprintf(szBuffer, "<?xml version=\"1.0\" encoding=\"utf-8\"?> \n ");
				nLen = strlen(szBuffer);
			}

			print((szBuffer + nLen), *xmlNode, 0);

			nLen = strlen(szBuffer);
			if (nLen > 0)
			{
				target.assign(szBuffer, nLen);
			}
#else
			std::string sOut = "";

			print_str(sOut, *xmlNode, 0);

			target = "";

			target.append("<?xml version=\"1.0\" encoding=\"utf-8\"?> \n ");

			target.append(sOut);
#endif
		}
		catch(...)
		{

		}
	}
};


#if 0
class XmlWriterUtil
{
public:

	typedef enum
	{
		NODE_TYPE_UNKNOWN,
		NODE_TYPE_BASE,
		NODE_TYPE_GROUP,
		NODE_TYPE_KEY
	
	} XmlNodeType_def;

	typedef struct
	{
		void			*pParent;

		XmlNodeType_def	eType;

		char			*pNodeStart;
		//char			*pNodeEnd;

		char			*pNodeName;

		int				nNameLen;

		char			*pNodeValue;

		int				nValueLen;

	} XmlNode_def;

private:

	char					*m_pOutputBuffer;

	std::string				m_sWorkingBuffer;

	long					m_lBufferIndex;

	bool					m_bWriteXmlToFile;

	XmlNode_def				m_LastNode;

public:

	XmlWriterUtil();
	~XmlWriterUtil();

	void Init()
	{
		m_pOutputBuffer = NULL;

		m_lBufferIndex = 0;

		m_bWriteXmlToFile = false;

	}

	void Init(char *pBuffer);

	void SetXmlBuffer(char *pBuffer);

	void Create()
	{


	}

	void Create(char *pName)
	{


	}

	void Create(char *pName, char *pValue)
	{

	}

	void AppendNode(XmlNode_def *parent, XmlNode_def *child)
	{

	}

	XmlNode_def * NewGroupgNode(char *name)
	{

		return NULL;
	}

	XmlNode_def * NewStringNode(char *name, string value)
	{
		return NewStringNode(name, (char *) (value.c_str()));
	}

	XmlNode_def * NewStringNode(XmlNode_def *parent, char *name, char *value)
	{

		return NULL;
	}

	XmlNode_def * NewStringNode(XmlNode_def *parent, char *name, string value)
	{
		return NewStringNode(parent, name, (char *) (value.c_str()));
	}

	POCO_XML_NODE * NewIntNode(char *name, int value)
	{

		return NULL;
	}

	XmlNode_def * NewIntNode(XmlNode_def *parent, char *name, int value)
	{

		return NULL;
	}

	XmlNode_def * NewBoolNode(char *name, bool value)
	{

		return NULL;
	}

	XmlNode_def * NewBoolNode(XmlNode_def *parent, char *name, bool value)
	{

		return NULL;
	}

};
#endif



#endif PocoXmlUtils_H_

