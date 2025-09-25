//**************************************************************************************************
//* FILE:		xmlUtils.h
//*
//* DESCRIP:	
//*
//*


#ifndef xmlUtils_H_
#define xmlUtils_H_

#include <string>
#include <ostream>
#include <stdexcept>
//#include <vector>
//#include <map>

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"

#include <String/StrUtils.h>


using namespace std;
using namespace rapidxml;


#define XML_TYPE				char

#define MAX_XML_BUFFER_SIZE		20480		// this is the maximum size buffer we can send via a "weblet"

#define MAX_XML_STRING_LEN		1024

#define LoadXmlAttributeString	LoadXmlSubParamString

#define DEFAULT_XML_DUMP_FILE	"XML_Dump.txt"

class XmlParserUtil
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

	XmlParserUtil() :
		m_pXmlParser(nullptr),
		m_pXmlBuffer(nullptr),
		m_lXmlBufLen(0),
		m_bXmlLoadedFromFile(false),
		m_bXmlLoadedFromBuf(false)
	{
		Init();
	}

	XmlParserUtil(bool bDumpOnError) :
		m_pXmlParser(nullptr),
		m_pXmlBuffer(nullptr),
		m_lXmlBufLen(0),
		m_bXmlLoadedFromFile(false),
		m_bXmlLoadedFromBuf(false)
	{
		Init();

		SetDumpOnError(bDumpOnError);
	}

	XmlParserUtil(bool bDumpOnError, const char *pDumpFile) :
		m_pXmlParser(nullptr),
		m_pXmlBuffer(nullptr),
		m_lXmlBufLen(0),
		m_bXmlLoadedFromFile(false),
		m_bXmlLoadedFromBuf(false)
	{
		Init();

		SetDumpOnError(bDumpOnError, pDumpFile);
	}


	XmlParserUtil(bool bDumpOnError, const std::string &sDumpFile) :
		m_pXmlParser(nullptr),
		m_pXmlBuffer(nullptr),
		m_lXmlBufLen(0),
		m_bXmlLoadedFromFile(false),
		m_bXmlLoadedFromBuf(false)
	{
		Init();

		SetDumpOnError(bDumpOnError, sDumpFile);
	}

	~XmlParserUtil()
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
		if (pDumpFile == nullptr)
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

	//long GetXmlLength();

	//void SetXmlBuffer(char *pBuffer);
	//void SetXmlBuffer(std::string sBuffer)
	//{
	//	SetXmlBuffer((char *) (sBuffer.c_str()));
	//}

	xml_node<XML_TYPE>*GetXmlBaseNode(char *pParam);
	xml_node<XML_TYPE>*GetXmlBaseNode(const std::string &sParam)
	{
		return GetXmlBaseNode(sParam.c_str());
	}

	xml_node<XML_TYPE>*Create()
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try 
		{
			//xml_attribute<> *attr = m_pXmlParser->allocate_attribute(name, value);

			xml_node<> *newNode = m_pXmlParser->allocate_node(node_element, nullptr, nullptr);
			if (newNode != nullptr)
			{
				m_pXmlParser->append_node(newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			return nullptr;
		}
	}

	xml_node<XML_TYPE>*Create(const std::string &sName)
	{
		return Create(sName.c_str());
	}
	xml_node<XML_TYPE>*Create(char *pName)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *nodeName = m_pXmlParser->allocate_string(pName);
			xml_node<> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, nullptr);
			if (newNode != nullptr)
			{
				m_pXmlParser->append_node(newNode);
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			return nullptr;
		}
	}

	xml_node<XML_TYPE>*Create(const std::string &sName, char *pValue)
	{
		return Create(sName.c_str(), pValue);
	}
	xml_node<XML_TYPE>*Create(char *pName, char *pValue)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *nodeName = m_pXmlParser->allocate_string(pName);
			xml_node<> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, pValue);
			if (newNode != nullptr)
			{
				m_pXmlParser->append_node(newNode);
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			return nullptr;
		}
	}

	bool AllocXmlBuffer(int nLen);  //throw(std::runtime_error);

	void FreeXmlBuffer()
	{
		try
		{
			if (m_pXmlBuffer != nullptr)
			{
				//* if the buffer is not currently nullptr... free it

				free(m_pXmlBuffer);
			}
		}
		catch(...)
		{

		}

		m_pXmlBuffer = nullptr;

		m_bXmlLoadedFromFile = false;
		m_bXmlLoadedFromBuf = false;
	}

	char * GetXmlBuffer()
	{
		return m_pXmlBuffer;
	}

	bool Parse();

	bool LoadXmlBuffer(char *pBuffer, int nLen);  //throw(std::runtime_error);
	bool LoadXmlBuffer(std::string sBuffer)  //throw(std::runtime_error)
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

	bool LoadXmlFile(char *pFolder, char *pFileName);  //throw(std::runtime_error);
	bool LoadXmlFile(std::string sFolder, std::string sFileName)  //throw(std::runtime_error)
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

	bool WriteXmlFile(char *pFolder, char *pFileName);  //throw(std::runtime_error);
	bool WriteXmlFile(std::string sFolder, std::string sFileName)  //throw(std::runtime_error)
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

	bool DumpXmlBuffer(char *pFolder = nullptr, char *pFileName = nullptr, bool bSpaceFill = false);  //throw(std::runtime_error);
	bool DumpXmlBuffer(std::string sFolder, std::string sFileName = "", bool bSpaceFill = false)  //throw(std::runtime_error)
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

	bool FindXmlParamGroup(xml_node<XML_TYPE>* pNode, char *pParam, bool bFirst = false);
	bool FindXmlParamGroup(xml_node<XML_TYPE>* pNode, std::string sParam, bool bFirst = false)
	{
		if (pNode == nullptr)
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
		if (pParam == nullptr)
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

	bool FindXmlParamString(xml_node<XML_TYPE>* pNode, char *pParam, bool bFirst = false)
	{
		if (pNode == nullptr || pParam == nullptr)
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
	bool FindXmlParamString(xml_node<XML_TYPE>* pNode, std::string sParam, bool bFirst = false)
	{
		if (pNode == nullptr)
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
		if (pParam == nullptr)
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

	xml_node<XML_TYPE>* GetXmlParamGroup(xml_node<XML_TYPE>* pNode, char *pParam, bool bFirst = false);
	xml_node<XML_TYPE>* GetXmlParamGroup(xml_node<XML_TYPE>* pNode, std::string sParam, bool bFirst = false)
	{
		if (pNode == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return GetXmlParamGroup(pNode, pPrm, bFirst);
		}
		catch(...)
		{

		}

		return nullptr;
	}
	xml_node<XML_TYPE>* GetXmlParamGroup(char *pParam, bool bFirst = false)
	{
		if (pParam == nullptr)
		{
			return nullptr;
		}

		try
		{
			return GetXmlParamGroup(m_pLastNode, pParam, bFirst);
		}
		catch(...)
		{

		}

		return nullptr;
	}
	xml_node<XML_TYPE>* GetXmlParamGroup(std::string sParam, bool bFirst = false)
	{
		char *pPrm = (char *) (sParam.c_str());
		return GetXmlParamGroup(m_pLastNode, pPrm, bFirst);
	}

	bool HasXmlChildNode(xml_node<XML_TYPE>* pNode);

	xml_node<XML_TYPE>* GetFirstXmlChild(xml_node<XML_TYPE>* pNode);

	xml_node<XML_TYPE>* GetNextXmlSibbling(xml_node<XML_TYPE>* pNode, char *pParam);
	xml_node<XML_TYPE>* GetNextXmlSibbling(xml_node<XML_TYPE>* pNode, std::string sParam)
	{
		if (pNode == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pPrm = (char *) (sParam.c_str());
			return GetNextXmlSibbling(pNode, pPrm);
		}
		catch(...)
		{

		}

		return nullptr;
	}
	xml_node<XML_TYPE>* GetNextXmlSibbling(xml_node<XML_TYPE>* pNode)
	{
		return GetNextXmlSibbling(pNode, nullptr);
	}

	bool LoadXmlParamBuf(xml_node<XML_TYPE>*pNode, char *pParam, char *pTarget, int nLen, char *pDefault, bool bFirst = false);
	bool LoadXmlParamBuf(xml_node<XML_TYPE>*pNode, std::string sParam, char *pTarget, int nLen, char *pDefault, bool bFirst = false)
	{
		if (pNode == nullptr || pTarget == nullptr)
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
		if (pParam == nullptr || pTarget == nullptr)
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
		if (pTarget == nullptr)
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

	bool LoadXmlParamString(xml_node<XML_TYPE>* pNode, const char *pParam, std::string &sTarget, const char *pDefault, bool bFirst = false);
	bool LoadXmlParamString(xml_node<XML_TYPE>* pNode, const std::string &sParam, std::string &sTarget, const char *pDefault, bool bFirst = false)
	{
		if (pNode == nullptr)
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
	bool LoadXmlParamString(const char *pParam, std::string &sTarget, char *pDefault, bool bFirst = false)
	{
		if (pParam == nullptr)
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
	bool LoadXmlParamString(const std::string &sParam, std::string &sTarget, char *pDefault, bool bFirst = false)
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
		if (pNode == nullptr)
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
		if (pSubParam == nullptr)
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

	bool LoadXmlParamBool(xml_node<XML_TYPE>* pNode, char *pParam, bool *bTarget, bool bUseDef, bool bDefVal, bool bFirst = false);
	bool LoadXmlParamBool(xml_node<XML_TYPE>* pNode, std::string sParam, bool *bTarget, bool bUseDef, bool bDefVal, bool bFirst = false)
	{
		if (pNode == nullptr || bTarget == nullptr)
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
		if (pParam == nullptr || bTarget == nullptr)
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
		if (bTarget == nullptr)
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

	bool LoadXmlParamInt(xml_node<XML_TYPE>* pNode, char *pParam, int *nTarget, bool bUseDef, int nDefVal, bool bFirst = false);
	bool LoadXmlParamInt(xml_node<XML_TYPE>* pNode, std::string sParam, int *nTarget, bool bUseDef, int nDefVal, bool bFirst = false)
	{
		if (pNode == nullptr || nTarget == nullptr)
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
		if (pParam == nullptr || nTarget == nullptr)
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
		if (nTarget == nullptr)
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

	bool LoadXmlParamLong(xml_node<XML_TYPE>* pNode, char *pParam, long *lTarget, bool bUseDef, long lDefVal, bool bFirst = false);
	bool LoadXmlParamLong(xml_node<XML_TYPE>* pNode, std::string sParam, long *lTarget, bool bUseDef, long *lDefVal, bool bFirst = false)
	{
		if (pNode == nullptr || lTarget == nullptr)
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
		if (pParam == nullptr || lTarget == nullptr)
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
		if (lTarget == nullptr)
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
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		return xmlNode->parent();
	}

	bool NextSibbling(xml_node<XML_TYPE>* pNode)
	{
		if (m_pXmlParser == nullptr)
		{
			return false;
		}

		try
		{
			xml_node<XML_TYPE>* pNext =
				pNode->next_sibling(0, 0, false);

			if (pNext != nullptr)
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
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *nodeName = nullptr;
			char *nodeValue = nullptr;
			if (name != nullptr)
			{
				nodeName = m_pXmlParser->allocate_string(name);
			}
			if (value != nullptr)
			{
				nodeValue = m_pXmlParser->allocate_string(value);
			}
			xml_node<XML_TYPE> *newNode = m_pXmlParser->allocate_node(type, nodeName, nodeValue);
			if (newNode != nullptr)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
	}
	xml_node<XML_TYPE> *AllocNode(node_type type, std::string name, char *value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return AllocNode(type, pName, value);
		}
		catch (...)
		{
			
		}

		return nullptr;
	}

	xml_node<XML_TYPE> *AllocStringNode(char *name, char *value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *nodeName = nullptr;
			char *nodeValue = nullptr;
			if (name != nullptr)
			{
				nodeName = m_pXmlParser->allocate_string(name);
			}
			if (value != nullptr)
			{
				nodeValue = m_pXmlParser->allocate_string(value);
			}
			xml_node<XML_TYPE> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != nullptr)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
	}
	xml_node<XML_TYPE> *AllocStringNode(std::string name, char *value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return AllocStringNode(pName, value);
		}
		catch (...)
		{
			
		}

		return nullptr;
	}

	xml_node<XML_TYPE> *AllocIntNode(char *name, int value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *nodeName = nullptr;
			char *nodeValue = nullptr;
			if (name != nullptr)
			{
				nodeName = m_pXmlParser->allocate_string(name);
			}

			string sTmp = StrUtils::tos(value);
			
			nodeValue = m_pXmlParser->allocate_string((char *) sTmp.c_str());
			xml_node<XML_TYPE> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != nullptr)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
	}
	xml_node<XML_TYPE> *AllocIntNode(std::string name, int value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return AllocIntNode(pName, value);
		}
		catch (...)
		{
			
		}

		return nullptr;
	}


	xml_node<XML_TYPE> *AllocUIntNode(char *name, unsigned int value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *nodeName = nullptr;
			char *nodeValue = nullptr;
			if (name != nullptr)
			{
				nodeName = m_pXmlParser->allocate_string(name);
			}

			string sTmp = StrUtils::tos(value);

			nodeValue = m_pXmlParser->allocate_string((char *)sTmp.c_str());
			xml_node<XML_TYPE> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != nullptr)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{

		}

		return nullptr;
	}
	xml_node<XML_TYPE> *AllocUIntNode(std::string name, unsigned int value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *)(name.c_str());
			return AllocUIntNode(pName, value);
		}
		catch (...)
		{

		}

		return nullptr;
	}

	xml_node<XML_TYPE> *AllocLongNode(char *name, long value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *nodeName = nullptr;
			char *nodeValue = nullptr;
			if (name != nullptr)
			{
				nodeName = m_pXmlParser->allocate_string(name);
			}

			string sTmp = StrUtils::tos(value);
			
			nodeValue = m_pXmlParser->allocate_string((char *) sTmp.c_str());
			xml_node<XML_TYPE> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != nullptr)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
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

		return nullptr;
	}

	xml_node<XML_TYPE> *AllocULongNode(char *name, unsigned long value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *nodeName = nullptr;
			char *nodeValue = nullptr;
			if (name != nullptr)
			{
				nodeName = m_pXmlParser->allocate_string(name);
			}

			string sTmp = StrUtils::tos(value);

			nodeValue = m_pXmlParser->allocate_string((char *) sTmp.c_str());
			xml_node<XML_TYPE> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != nullptr)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{

		}

		return nullptr;
	}
	xml_node<XML_TYPE> *AllocLongNode(std::string name, unsigned long value)
	{
		try
		{
			char *pName = (char *)(name.c_str());
			return AllocULongNode(pName, value);
		}
		catch (...)
		{

		}

		return nullptr;
	}

	xml_node<XML_TYPE> *AllocDWordNode(char *name, long long value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *nodeName = nullptr;
			char *nodeValue = nullptr;
			if (name != nullptr)
			{
				nodeName = m_pXmlParser->allocate_string(name);
			}

			string sTmp = StrUtils::tos(value);

			nodeValue = m_pXmlParser->allocate_string((char *) sTmp.c_str());
			xml_node<XML_TYPE> *newNode = m_pXmlParser->allocate_node(node_element, nodeName, nodeValue);
			if (newNode != nullptr)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{

		}

		return nullptr;
	}
	xml_node<XML_TYPE> *AllocDWordNode(std::string name, long long value)
	{
		try
		{
			char *pName = (char *)(name.c_str());
			return AllocDWordNode(pName, value);
		}
		catch (...)
		{

		}

		return nullptr;
	}

	xml_node<XML_TYPE> *AllocBoolNode(char *name, bool value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *nodeName = nullptr;
			char *nodeValue = nullptr;
			if (name != nullptr)
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
			if (newNode != nullptr)
			{
				m_pLastNode = newNode;
			}
			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
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

		return nullptr;
	}

	void AppendNode(xml_node<XML_TYPE> *child)
	{
		if (m_pXmlParser == nullptr || child == nullptr)
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
		if (m_pXmlParser == nullptr || child == nullptr)
		{
			return;
		}

		try
		{
			if (parent == nullptr)
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
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocStringNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewNode(xml_node<XML_TYPE> *parent, char *name, char *value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocStringNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewStringNode(char *name, char *value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocStringNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
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

		return nullptr;
	}

	xml_node<XML_TYPE> * NewStringNode(xml_node<XML_TYPE> *parent, char *name, char *value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocStringNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewStringNode(xml_node<XML_TYPE> *parent, std::string name, std::string value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
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

		return nullptr;
	}

	xml_node<XML_TYPE> * NewIntNode(char *name, int value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocIntNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewIntNode(std::string name, int value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewIntNode(pName, value);
		}
		catch (...)
		{
			
		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewIntNode(xml_node<XML_TYPE> *parent, char *name, int value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocIntNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewIntNode(xml_node<XML_TYPE> *parent, std::string name, int value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewIntNode(parent, pName, value);
		}
		catch (...)
		{
			
		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewUIntNode(char *name, unsigned int value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocUIntNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{

		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewUIntNode(std::string name, unsigned int value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewUIntNode(pName, value);
		}
		catch (...)
		{

		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewUIntNode(xml_node<XML_TYPE> *parent, char *name, unsigned int value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocUIntNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{

		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewUIntNode(xml_node<XML_TYPE> *parent, std::string name, unsigned int value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewUIntNode(parent, pName, value);
		}
		catch (...)
		{

		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewLongNode(char *name, long value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocLongNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewLongNode(std::string name, long value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewLongNode(pName, value);
		}
		catch (...)
		{
			
		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewLongNode(xml_node<XML_TYPE> *parent, char *name, long value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocLongNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewlongNode(xml_node<XML_TYPE> *parent, std::string name, long value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewLongNode(parent, pName, value);
		}
		catch (...)
		{
			
		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewULongNode(char *name, unsigned long value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocULongNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{

		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewULongNode(std::string name, unsigned long value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewULongNode(pName, value);
		}
		catch (...)
		{

		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewULongNode(xml_node<XML_TYPE> *parent, char *name, unsigned long value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocULongNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{

		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewUlongNode(xml_node<XML_TYPE> *parent, std::string name, unsigned long value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewULongNode(parent, pName, value);
		}
		catch (...)
		{

		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewDWordNode(char *name, long long value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocDWordNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{

		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewDWordNode(std::string name, long long value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewDWordNode(pName, value);
		}
		catch (...)
		{

		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewDWordNode(xml_node<XML_TYPE> *parent, char *name, long long value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocDWordNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{

		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewDWordNode(xml_node<XML_TYPE> *parent, std::string name, long long value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *)(name.c_str());
			return NewDWordNode(parent, pName, value);
		}
		catch (...)
		{

		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewBoolNode(char *name, bool value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			xml_node<XML_TYPE> *newNode = AllocBoolNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(m_pBaseNode, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewBoolNode(std::string name, bool value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewBoolNode(pName, value);
		}
		catch (...)
		{
			
		}

		return nullptr;
	}

	xml_node<XML_TYPE> * NewBoolNode(xml_node<XML_TYPE> *parent, char *name, bool value)
	{
		if (m_pXmlParser == nullptr || name == nullptr)
		{
			return nullptr;
		}


		try
		{
			xml_node<XML_TYPE> *newNode = AllocBoolNode(name, value);
			if (newNode != nullptr)
			{
				AppendNode(parent, newNode);
				m_pLastNode = newNode;
			}

			return newNode;
		}
		catch (...)
		{
			
		}

		return nullptr;
	}
	xml_node<XML_TYPE> * NewBoolNode(xml_node<XML_TYPE> *parent, std::string name, bool value)
	{
		if (m_pXmlParser == nullptr)
		{
			return nullptr;
		}

		try
		{
			char *pName = (char *) (name.c_str());
			return NewBoolNode(parent, pName, value);
		}
		catch (...)
		{
			
		}

		return nullptr;
	}

	xml_node<XML_TYPE> * FindNext(xml_node<XML_TYPE> * pNode, char *name)
	{
		if (pNode == nullptr || name == nullptr)
		{
			return nullptr;
		}

		try
		{
			return pNode->next_sibling(name, 0, true);
		}
		catch (...)
		{
			
		}

		return nullptr;
	}

	bool GetNodeName(xml_node<XML_TYPE> * pNode, std::string &name)
	{
		if (pNode == nullptr)
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
		if (pNode == nullptr)
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
		if (m_pXmlParser == nullptr || target == nullptr || xmlNode == nullptr)
		{
			return;
		}

		try
		{
			long nLen = 0;

			if (bAddHeader == true)
			{
#ifdef WINDOWS
				sprintf_s(target, MAX_XML_STRING_LEN, "<?xml version=\"1.0\" encoding=\"utf-8\"?> \n ");
#else
				sprintf(target, "<?xml version=\"1.0\" encoding=\"utf-8\"?> \n ");
#endif
				nLen = (long) strlen(target);
			}

			print((target + nLen), *xmlNode, 0);
		}
		catch(...)
		{

		}
	}

	void XmlOutput(std::string &target, xml_node<XML_TYPE>* xmlNode, bool bAddHeader = false)
	{
		if (xmlNode == nullptr)
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

			//print_str(sOut, *xmlNode, 0);
			sOut = StrUtils::strPrintf((char *) xmlNode);

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
		m_pOutputBuffer = nullptr;

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

		return nullptr;
	}

	XmlNode_def * NewStringNode(char *name, string value)
	{
		return NewStringNode(name, (char *) (value.c_str()));
	}

	XmlNode_def * NewStringNode(XmlNode_def *parent, char *name, char *value)
	{

		return nullptr;
	}

	XmlNode_def * NewStringNode(XmlNode_def *parent, char *name, string value)
	{
		return NewStringNode(parent, name, (char *) (value.c_str()));
	}

	xml_node<XML_TYPE> * NewIntNode(char *name, int value)
	{

		return nullptr;
	}

	XmlNode_def * NewIntNode(XmlNode_def *parent, char *name, int value)
	{

		return nullptr;
	}

	XmlNode_def * NewBoolNode(char *name, bool value)
	{

		return nullptr;
	}

	XmlNode_def * NewBoolNode(XmlNode_def *parent, char *name, bool value)
	{

		return nullptr;
	}

};
#endif



#endif  //  xmlUtils_H_

