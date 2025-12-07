//**************************************************************************************************
//* FILE:		xmlHandler.h
//*
//* DESCRIP:	
//*
//*


#define _CRT_SECURE_NO_WARNINGS


#ifndef xmlHandler_H_
#define xmlHandler_H_


#include <string>
#include <ostream>
#include <stdexcept>
//#include <vector>
//#include <map>

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

#include "strUtils.h"


using namespace std;
using namespace rapidxml;

#define XML_TYPE				char

#define MAX_XML_BUFFER_SIZE		20480		// this is the maximum size buffer we can send via a "weblet"

#define LoadXmlAttributeString	LoadXmlSubParamString

#define DEFAULT_XML_DUMP_FILE	"XML_Dump.txt"


class CXmlHandler
{
private:

	xml_document<>			*m_pXmlParser;

	xml_node<XML_TYPE>		*m_pBaseNode;
	xml_node<XML_TYPE>		*m_pLastNode;

	char					*m_pXmlBuffer;

	long					m_lXmlBufLen;

	bool					m_bXmlLoadedFromFile;
	bool					m_bXmlLoadedFromBuf;

	//int						m_nParseFlags;

	bool					m_bDumpXmlOnError;

	std::string				m_sXmlDumpFile;

public:

	CXmlHandler() :
		m_pXmlParser(NULL),
		m_pXmlBuffer(NULL),
		m_lXmlBufLen(0),
		m_bXmlLoadedFromFile(false),
		m_bXmlLoadedFromBuf(false)
	{
		Init();
	}

	CXmlHandler(bool bDumpOnError) :
		m_pXmlParser(NULL),
		m_pXmlBuffer(NULL),
		m_lXmlBufLen(0),
		m_bXmlLoadedFromFile(false),
		m_bXmlLoadedFromBuf(false)
	{
		Init();

		SetDumpOnError(bDumpOnError);
	}

	CXmlHandler(bool bDumpOnError, const char *pDumpFile) :
		m_pXmlParser(NULL),
		m_pXmlBuffer(NULL),
		m_lXmlBufLen(0),
		m_bXmlLoadedFromFile(false),
		m_bXmlLoadedFromBuf(false)
	{
		Init();

		SetDumpOnError(bDumpOnError, pDumpFile);
	}


	CXmlHandler(bool bDumpOnError, const std::string &sDumpFile) :
		m_pXmlParser(NULL),
		m_pXmlBuffer(NULL),
		m_lXmlBufLen(0),
		m_bXmlLoadedFromFile(false),
		m_bXmlLoadedFromBuf(false)
	{
		Init();

		SetDumpOnError(bDumpOnError, sDumpFile);
	}

	~CXmlHandler()
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
			m_pXmlParser->clear();
		}
		catch (...)
		{
			return;
		}
	}

	xml_node<XML_TYPE>*GetXmlBaseNode(char *pParam);
	xml_node<XML_TYPE>*GetXmlBaseNode(const std::string &sParam)
	{
		return GetXmlBaseNode(sParam.c_str());
	}

	xml_node<XML_TYPE>*Create()
	{
		if (m_pXmlParser == NULL)
		{
			return NULL;
		}

		try 
		{
			xml_node<> *newNode = m_pXmlParser->allocate_node(node_element, NULL, NULL);
			if (newNode != NULL)
			{
				m_pXmlParser->append_node(newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			return NULL;
		}
	}

	xml_node<XML_TYPE>*Create(const std::string &sName)
	{
		return Create(sName.c_str());
	}
	xml_node<XML_TYPE>*Create(char *pName)
	{
		if (m_pXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = m_pXmlParser->allocate_string(pName);
			xml_node<> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, NULL);
			if (newNode != NULL)
			{
				m_pXmlParser->append_node(newNode);
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			return NULL;
		}
	}

	xml_node<XML_TYPE>*Create(const std::string &sName, char *pValue)
	{
		return Create(sName.c_str(), pValue);
	}
	xml_node<XML_TYPE>*Create(char *pName, char *pValue)
	{
		if (m_pXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = m_pXmlParser->allocate_string(pName);
			xml_node<> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, pValue);
			if (newNode != NULL)
			{
				m_pXmlParser->append_node(newNode);
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			return NULL;
		}
	}

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
			int nLen = (sBuffer.length());
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

	bool FindXmlParamGroup(xml_node<XML_TYPE>* pNode, char *pParam, bool bFirst);
	bool FindXmlParamGroup(xml_node<XML_TYPE>* pNode, std::string sParam, bool bFirst)
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
	bool FindXmlParamGroup(char *pParam, bool bFirst)
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
	bool FindXmlParamGroup(std::string sParam, bool bFirst)
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

	bool FindXmlParamString(xml_node<XML_TYPE>* pNode, char *pParam, bool bFirst)
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
	bool FindXmlParamString(xml_node<XML_TYPE>* pNode, std::string sParam, bool bFirst)
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
	bool FindXmlParamString(char *pParam, bool bFirst)
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
	bool FindXmlParamString(std::string sParam, bool bFirst)
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

	xml_node<XML_TYPE>* GetXmlParamGroup(xml_node<XML_TYPE>* pNode, char *pParam, bool bFirst);
	xml_node<XML_TYPE>* GetXmlParamGroup(xml_node<XML_TYPE>* pNode, std::string sParam, bool bFirst)
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
	xml_node<XML_TYPE>* GetXmlParamGroup(char *pParam, bool bFirst)
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
	xml_node<XML_TYPE>* GetXmlParamGroup(std::string sParam, bool bFirst)
	{
		char *pPrm = (char *) (sParam.c_str());
		return GetXmlParamGroup(m_pLastNode, pPrm, bFirst);
	}

	xml_node<XML_TYPE>* GetNextXmlSibbling(xml_node<XML_TYPE>* pNode, char *pParam);
	xml_node<XML_TYPE>* GetNextXmlSibbling(xml_node<XML_TYPE>* pNode, std::string sParam)
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

	bool LoadXmlParamBuf(xml_node<XML_TYPE>*pNode, char *pParam, char *pTarget, int nLen, char *pDefault, bool bFirst);
	bool LoadXmlParamBuf(xml_node<XML_TYPE>*pNode, std::string sParam, char *pTarget, int nLen, char *pDefault, bool bFirst)
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
	bool LoadXmlParamBuf(char *pParam, char *pTarget, int nLen, char *pDefault, bool bFirst)
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
	bool LoadXmlParamBuf(std::string sParam, char *pTarget, int nLen, char *pDefault, bool bFirst)
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

	bool LoadXmlParamString(xml_node<XML_TYPE>* pNode, char *pParam, std::string &sTarget, char *pDefault, bool bFirst);
	bool LoadXmlParamString(xml_node<XML_TYPE>* pNode, std::string sParam, std::string &sTarget, char *pDefault, bool bFirst)
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
	bool LoadXmlParamString(char *pParam, std::string &sTarget, char *pDefault, bool bFirst)
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
	bool LoadXmlParamString(std::string sParam, std::string &sTarget, char *pDefault, bool bFirst)
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

	bool LoadXmlSubParamString(xml_node<XML_TYPE>* pNode, char *pSubParam, std::string &sTarget, char *pDefault);
	bool LoadXmlSubParamString(xml_node<XML_TYPE>* pNode, std::string sSubParam, std::string &sTarget, char *pDefault)
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

	bool LoadXmlParamBool(xml_node<XML_TYPE>* pNode, char *pParam, bool *bTarget, bool bUseDef, bool bDefVal, bool bFirst);
	bool LoadXmlParamBool(xml_node<XML_TYPE>* pNode, std::string sParam, bool *bTarget, bool bUseDef, bool bDefVal, bool bFirst)
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
	bool LoadXmlParamBool(char *pParam, bool *bTarget, bool bUseDef, bool bDefVal, bool bFirst)
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
	bool LoadXmlParamBool(std::string sParam, bool *bTarget, bool bUseDef, bool bDefVal, bool bFirst)
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

	bool LoadXmlParamInt(xml_node<XML_TYPE>* pNode, char *pParam, int *nTarget, bool bUseDef, int nDefVal, bool bFirst);
	bool LoadXmlParamInt(xml_node<XML_TYPE>* pNode, std::string sParam, int *nTarget, bool bUseDef, int nDefVal, bool bFirst)
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
	bool LoadXmlParamInt(char *pParam, int *nTarget, bool bUseDef, int nDefVal, bool bFirst)
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
	bool LoadXmlParamInt(std::string sParam, int *nTarget, bool bUseDef, int nDefVal, bool bFirst)
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

	bool LoadXmlParamLong(xml_node<XML_TYPE>* pNode, char *pParam, long *lTarget, bool bUseDef, long lDefVal, bool bFirst);
	bool LoadXmlParamLong(xml_node<XML_TYPE>* pNode, std::string sParam, long *lTarget, bool bUseDef, long *lDefVal, bool bFirst)
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
	bool LoadXmlParamLong(char *pParam, long *lTarget, bool bUseDef, long *lDefVal, bool bFirst)
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
	bool LoadXmlParamLong(std::string sParam, long *lTarget, bool bUseDef, long *lDefVal, bool bFirst)
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

	xml_node<XML_TYPE>* GetLastNode()
	{
		return m_pLastNode;
	}

	void SetLastNode(xml_node<XML_TYPE>* pNode)
	{
		m_pLastNode = pNode;
	}

	xml_node<XML_TYPE>* Parent(xml_node<XML_TYPE>* xmlNode)
	{
		if (m_pXmlParser == NULL)
		{
			return NULL;
		}

		return xmlNode->parent();
	}

	bool NextSibbling(xml_node<XML_TYPE>* pNode)
	{
		if (m_pXmlParser == NULL)
		{
			return false;
		}

		try
		{
			xml_node<XML_TYPE>* pNext =
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

	xml_node<XML_TYPE> *AllocNode(node_type type, char *name, char *value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pXmlParser->allocate_string(name);
			}
			if (value != NULL)
			{
				nodeValue = m_pXmlParser->allocate_string(value);
			}
			xml_node<XML_TYPE> *newNode = m_pXmlParser->allocate_node(type, nodeName, nodeValue);
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
	xml_node<XML_TYPE> *AllocNode(node_type type, std::string name, char *value)
	{
		if (m_pXmlParser == NULL)
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

	xml_node<XML_TYPE> *AllocStringNode(char *name, char *value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pXmlParser->allocate_string(name);
			}
			if (value != NULL)
			{
				nodeValue = m_pXmlParser->allocate_string(value);
			}
			xml_node<XML_TYPE> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, nodeValue);
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
	xml_node<XML_TYPE> *AllocStringNode(std::string name, char *value)
	{
		if (m_pXmlParser == NULL)
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

	xml_node<XML_TYPE> *AllocIntNode(char *name, int value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pXmlParser->allocate_string(name);
			}

			string sTmp = strUtils::tos(value);
			
			nodeValue = m_pXmlParser->allocate_string((char *) sTmp.c_str());
			xml_node<XML_TYPE> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, nodeValue);
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
	xml_node<XML_TYPE> *AllocIntNode(std::string name, int value)
	{
		if (m_pXmlParser == NULL)
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

	xml_node<XML_TYPE> *AllocLongNode(char *name, long value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pXmlParser->allocate_string(name);
			}

			string sTmp = strUtils::tos(value);
			
			nodeValue = m_pXmlParser->allocate_string((char *) sTmp.c_str());
			xml_node<XML_TYPE> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, nodeValue);
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
	xml_node<XML_TYPE> *AllocLongNode(std::string name, long value)
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

	xml_node<XML_TYPE> *AllocBoolNode(char *name, bool value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			char *nodeName = NULL;
			char *nodeValue = NULL;
			if (name != NULL)
			{
				nodeName = m_pXmlParser->allocate_string(name);
			}
			if (value == true)
			{
				nodeValue = m_pXmlParser->allocate_string("true");
			}
			else
			{
				nodeValue = m_pXmlParser->allocate_string("false");
			}
			xml_node<XML_TYPE> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, nodeValue);
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
	xml_node<XML_TYPE> *AllocBoolNode(std::string name, bool value)
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

	void AppendNode(xml_node<XML_TYPE> *child)
	{
		if (m_pXmlParser == NULL || child == NULL)
		{
			return;
		}

		try
		{
			m_pXmlParser->append_node(child);
			m_pLastNode = child;
		}
		catch (...)
		{

		}
	}

	void AppendNode(xml_node<XML_TYPE> *parent, xml_node<XML_TYPE> *child)
	{
		if (m_pXmlParser == NULL || child == NULL)
		{
			return;
		}

		try
		{
			if (parent == NULL)
			{
				m_pXmlParser->append_node(child);
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

	xml_node<XML_TYPE> * NewNode(char *name, char *value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocStringNode(name, value);
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

	xml_node<XML_TYPE> * NewNode(xml_node<XML_TYPE> *parent, char *name, char *value = NULL)
	{
		if (m_pXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocStringNode(name, value);
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

	xml_node<XML_TYPE> * NewStringNode(char *name, char *value)
	{
		if (m_pXmlParser == NULL)
		{
			return NULL;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocStringNode(name, value);
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
	xml_node<XML_TYPE> * NewStringNode(std::string name, std::string value)
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

	xml_node<XML_TYPE> * NewStringNode(xml_node<XML_TYPE> *parent, char *name, char *value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocStringNode(name, value);
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
	xml_node<XML_TYPE> * NewStringNode(xml_node<XML_TYPE> *parent, std::string name, std::string value)
	{
		if (m_pXmlParser == NULL)
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

	xml_node<XML_TYPE> * NewIntNode(char *name, int value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocIntNode(name, value);
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
	xml_node<XML_TYPE> * NewIntNode(std::string name, int value)
	{
		if (m_pXmlParser == NULL)
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

	xml_node<XML_TYPE> * NewIntNode(xml_node<XML_TYPE> *parent, char *name, int value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocIntNode(name, value);
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
	xml_node<XML_TYPE> * NewIntNode(xml_node<XML_TYPE> *parent, std::string name, int value)
	{
		if (m_pXmlParser == NULL)
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

	xml_node<XML_TYPE> * NewLongNode(char *name, long value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocIntNode(name, value);
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
	xml_node<XML_TYPE> * NewLongNode(std::string name, long value)
	{
		if (m_pXmlParser == NULL)
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

	xml_node<XML_TYPE> * NewLongNode(xml_node<XML_TYPE> *parent, char *name, long value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocIntNode(name, value);
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
	xml_node<XML_TYPE> * NewlongNode(xml_node<XML_TYPE> *parent, std::string name, long value)
	{
		if (m_pXmlParser == NULL)
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

	xml_node<XML_TYPE> * NewBoolNode(char *name, bool value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocBoolNode(name, value);
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
	xml_node<XML_TYPE> * NewBoolNode(std::string name, bool value)
	{
		if (m_pXmlParser == NULL)
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

	xml_node<XML_TYPE> * NewBoolNode(xml_node<XML_TYPE> *parent, char *name, bool value)
	{
		if (m_pXmlParser == NULL || name == NULL)
		{
			return NULL;
		}


		try
		{
			xml_node<XML_TYPE> *newNode = AllocBoolNode(name, value);
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
	xml_node<XML_TYPE> * NewBoolNode(xml_node<XML_TYPE> *parent, std::string name, bool value)
	{
		if (m_pXmlParser == NULL)
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

	xml_node<XML_TYPE> * FindNext(xml_node<XML_TYPE> * pNode, char *name)
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

	bool GetNodeName(xml_node<XML_TYPE> * pNode, string name)
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

	bool GetNodeValue(xml_node<XML_TYPE> * pNode, std::string &value)
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

	//xml_node<XML_TYPE> * FindChild(xml_node<XML_TYPE> * xmlNode, char *name)
	//{
	//	return xmlNode->
	//}

	void XmlOutput(char *target, xml_node<XML_TYPE>* xmlNode, bool bAddHeader = false)
	{
		if (m_pXmlParser == NULL || target == NULL || xmlNode == NULL)
		{
			return;
		}

		try
		{
			int nLen = 0;

			if (bAddHeader == true)
			{
				sprintf(target, "<?xml version=\"1.0\" encoding=\"utf-8\"?> \n ");
				nLen = strlen(target);
			}

			print((target + nLen), *xmlNode, 0);
		}
		catch(...)
		{

		}
	}

	void XmlOutput(std::string &target, xml_node<XML_TYPE>* xmlNode, bool bAddHeader = false)
	{
		if (xmlNode == NULL)
		{
			return;
		}

		try
		{
			std::string sOut = "";

			print_str(sOut, *xmlNode, 0);

			target = "";

			target.append("<?xml version=\"1.0\" encoding=\"utf-8\"?> \n ");

			target.append(sOut);
		}
		catch(...)
		{

		}
	}
};


#endif xmlHandler_H_
