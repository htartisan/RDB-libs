//**************************************************************************************************
//* FILE:		xmlHandler.cpp
//*
//* DESCRIP:	
//*
//*


#ifdef WINDOWS
#include <windows.h>
#else
#include "win32_unix.h"
#endif

#include <stdio.h>
#include <sys/types.h>

#include <string>
#include <iostream>       // std::cerr
#include <exception> 

#include "CFileIO.h"

#include "strUtils.h"
#include "xmlHandler.h"


//using namespace std;
//using namespace stringUtil;
//using namespace rapidxml;


void CXmlHandler::Init()
{
	m_pXmlBuffer = NULL;

	m_lXmlBufLen = 0;

	m_pBaseNode = NULL;
	m_pLastNode = NULL;

	m_bXmlLoadedFromFile = false;
	m_bXmlLoadedFromBuf = false;

	m_sXmlDumpFile = DEFAULT_XML_DUMP_FILE;

	m_bDumpXmlOnError = false;

	try
	{
		m_pXmlParser = new xml_document<>;
	}
	catch(...)
	{
		m_pXmlParser = NULL;
	}
}

void CXmlHandler::DeInit()
{
	if (m_pXmlBuffer != NULL)
	{
		FreeXmlBuffer();
	}

	try
	{
		if (m_pXmlParser != NULL)
		{
			delete m_pXmlParser;

			m_pXmlParser = NULL;
		}
	}
	catch(...)
	{
	
	};
}


xml_node<XML_TYPE>* CXmlHandler::GetXmlBaseNode(char *pParam)
{
	if (m_pXmlParser == NULL)
	{
		return NULL;
	}

	try
	{
		m_pBaseNode = m_pXmlParser->first_node(pParam);

		if (m_pBaseNode != NULL)
		{
			m_pLastNode = m_pBaseNode;
		}

		return (m_pBaseNode);
	}
	catch(...)
	{

	}

	return NULL;
}


bool CXmlHandler::Parse()
{
	if (m_pXmlParser == NULL)
	{
		return false;
	}

	try 
	{
		//* replace any characters that might cause parsing problems with spaces

		char *pChr = m_pXmlBuffer;

		for (long x = 0; x < m_lXmlBufLen; x++)
		{
			if (*pChr == '\n' || *pChr == '\r' || *pChr == '\t' || *pChr == '\v' || *pChr == '\a' || *pChr == '\f' || *pChr == 0)
			{
				*pChr = ' ';
			}

			pChr++;
		}

		m_pXmlParser->parse<0>(m_pXmlBuffer);		// 0 = default XML parsing flags
	}
	catch (std::exception& eErr)
	{
		std::string sErrText = eErr.what();
		std::cerr << "ERROR: Exception while parsing XML text ( " << sErrText << " ) " << std::endl;
		return false;
	}	
	catch (...)
	{
		std::cerr << "ERROR: Unknown exception while parsing XML text " << std::endl;
		return false;
	}

	return true;
}


bool CXmlHandler::AllocXmlBuffer(int nLen)
{
	if (nLen < 3)
	{
		return false;
	}

	try
	{
		if (m_pXmlBuffer != NULL)
		{
			CXmlHandler::FreeXmlBuffer();
		}

		m_pXmlBuffer = (char *) calloc((nLen + 2), 1);
		if (m_pXmlBuffer == NULL)
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

bool CXmlHandler::LoadXmlBuffer(char *pBuffer, int nLen) throw(std::runtime_error)
{
	if (m_pXmlParser == NULL)
	{
		return false;
	}

	if (pBuffer == NULL || nLen < 1)
	{
		return false;
	}

	bool bStatus = false;

	try
	{
		bStatus = AllocXmlBuffer(nLen);
		if (bStatus == false)
		{
			return false;
		}

		memcpy(m_pXmlBuffer, pBuffer, nLen);

		m_lXmlBufLen = nLen;

		//* Point the XML parser at the file data buffer 

		bStatus = Parse();
		if (bStatus == false)
		{
			if (m_bDumpXmlOnError == true)
			{
				DumpXmlBuffer();
			}

			return false;
		}

		return true;
	}
	catch(...)
	{
		bStatus = false;
	}

	return false;
}



bool CXmlHandler::LoadXmlFile(char *pFolder, char *pFileName) throw(std::runtime_error)
{
	if (m_pXmlParser == NULL)
	{
		return false;
	}

	if (pFileName == NULL)
	{
		//throw std::runtime_error("Invalid param");
		return false;
	}

	bool bStatus = FALSE;

	CFileIO		cFileManager;

	try
	{
		//* open the XML file and load the XML into a buffer

		if (pFolder != NULL)
		{
			bStatus = cFileManager.SetFileDir((char *) pFolder);
			if (bStatus == false)
			{
				return false;
			}
		}

		bStatus = cFileManager.SetFileName((char *) pFileName);
		if (bStatus == false)
		{
			return false;
		}

		//* open file for "r" (read only) mode
		cFileManager.SetFileMode("r");

		bStatus = cFileManager.Open();
		if (bStatus == false)
		{
			return false;
		}

		LONG lFileLen = cFileManager.GetFileLen();
		if (lFileLen < 1)
		{
			goto CloseAndExit;
		}

		bStatus = CXmlHandler::AllocXmlBuffer(lFileLen);
		if (bStatus == false)
		{
			goto CloseAndExit;
		}

		cFileManager.SetFilePointer(0);
	
		bStatus = cFileManager.ReadBuffer(m_pXmlBuffer, lFileLen);
		if (bStatus == false)
		{
			if (cFileManager.CheckEOF() != true)
			{
				goto CloseAndExit;
			}
		}

		m_bXmlLoadedFromFile = true;

		m_lXmlBufLen = lFileLen;


		//* Point the XML parser at the file data buffer 

		bStatus = Parse();
	}
	catch(...)
	{
		bStatus = false;
	}

	if (bStatus == false && m_bDumpXmlOnError == true)
	{
		DumpXmlBuffer(NULL, (char *) (m_sXmlDumpFile.c_str()), true);
	}

  CloseAndExit:

	cFileManager.Close();

	return  bStatus;
}



bool CXmlHandler::WriteXmlFile(char *pFolder, char *pFileName) throw(std::runtime_error)
{
	if (m_pXmlParser == NULL)
	{
		return false;
	}

	if (m_pXmlBuffer == NULL)
	{
		// if the buffer is currently NULL, then we don't have anything to write to the file

		return false;
	}

	long lXmlLen = strlen(m_pXmlBuffer);
	if (lXmlLen < 1)
	{
		return false;
	}

	if (pFileName == NULL)
	{
		return false;
	}

	bool bStatus = false;

	//* open the XML file and load the XML into a buffer

	CFileIO		cFileManager;

	try
	{
		if (pFolder != NULL)
		{
			bStatus = cFileManager.SetFileDir((char *) pFolder);
			if (bStatus == FALSE)
			{
				return false;
			}
		}

		bStatus = cFileManager.SetFileName((char *) pFileName);
		if (bStatus == false)
		{
			return false;
		}

		//* open file for "w" (write only) mode
		cFileManager.SetFileMode("w");

		bStatus = cFileManager.Open();
		if (bStatus == false)
		{
			return false;
		}

		std::string sOut = "";

		XmlOutput(sOut, m_pBaseNode, true);
	
		bStatus = cFileManager.WriteBuffer((char *) (sOut.c_str()), (sOut.length()));
	}
	catch(...)
	{
		bStatus = false;
	}

	cFileManager.Close();

	return ((bStatus == TRUE) ? true : false);
}


bool CXmlHandler::DumpXmlBuffer(char *pFolder, char *pFileName, bool bSpaceFill) throw(std::runtime_error)
{
	if (m_pXmlBuffer == NULL)
	{
		// if the buffer is currently NULL, then we don't have anything to write to the file

		return false;
	}

	bool bStatus = false;

	//* open the XML file and load the XML into a buffer

	CFileIO		cFileManager;

	try
	{
		if (m_lXmlBufLen < 1)
		{
			long lBufLen = strlen(m_pXmlBuffer);
			if (lBufLen < 1)
			{
				return false;
			}

			m_lXmlBufLen = lBufLen;

			bSpaceFill = false;
		}

		std::string sDumpFile = "";

		if (pFileName != NULL)
		{
			sDumpFile.append(pFileName);
		}
		else
		{
			sDumpFile.append(m_sXmlDumpFile);
		}

		if (pFolder != NULL)
		{
			bStatus = cFileManager.SetFileDir((char *) pFolder);
			if (bStatus == false)
			{
				return false;
			}
		}

		bStatus = cFileManager.SetFileName((char *) sDumpFile.c_str());
		if (bStatus == false)
		{
			return false;
		}

		//* open file for "w" (write only) mode
		cFileManager.SetFileMode("w");

		bStatus = cFileManager.Open();
		if (bStatus == false)
		{
			return false;
		}

		if (bSpaceFill == true)
		{
			char *pTemp = (char *) calloc((m_lXmlBufLen + 2), 1);
			if (pTemp != NULL)
			{
				char *pC1 = m_pXmlBuffer;
				char *pC2 = pTemp;

				for (long x = 0; x < m_lXmlBufLen; x++)
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

				bStatus = cFileManager.WriteBuffer(pTemp, m_lXmlBufLen);
			}
			else
			{
				bStatus = cFileManager.WriteBuffer((char *) m_pXmlBuffer, m_lXmlBufLen);
			}
		}
		else
		{
			bStatus = cFileManager.WriteBuffer((char *) m_pXmlBuffer, m_lXmlBufLen);
		}
	}
	catch(...)
	{

	}

	cFileManager.Close();

	return bStatus;
}


bool CXmlHandler::FindXmlParamGroup(xml_node<XML_TYPE>*pNode, char *pParam, bool bFirst)
{
	if (m_pXmlParser == NULL)
	{
		return false;
	}

	if (pNode == NULL)
	{
		return false;
    } 

	bool bRet = false;

	std::string sParam = "";
	std::string sName = "";

	sParam.assign(pParam);

	try
	{
		sName.assign(pNode->name(), pNode->name_size());
		if (sName == sParam)	
		{												
			//* we're already there

			m_pLastNode = pNode;

			return true;
		}		

		//* start searching for the desired node

		xml_node<> *xmlSearch;

		if (bFirst)
		{
			xmlSearch = pNode->first_node(0, 0, true);
		}
		else
		{
			xmlSearch = pNode->first_node(pParam, 0, true);
		}

		while (1)
		{
			if (xmlSearch == NULL)
			{
				break;
			}

			sName.assign(xmlSearch->name(), xmlSearch->name_size());
			if (sName == sParam)	
			{												
				bRet = true;
				m_pLastNode = xmlSearch;

				break;
			}		

			xmlSearch = xmlSearch->next_sibling(0, 0, true);
		}
	}
	catch (...)
	{
		return false;
	}

	return bRet;
}

xml_node<XML_TYPE>*  CXmlHandler::GetXmlParamGroup(xml_node<XML_TYPE>*pNode, char *pParam, bool bFirst)
{
	if (m_pXmlParser == NULL)
	{
		return NULL;
	}

	if (pNode == NULL)
	{
		return NULL;
    } 

	bool bRet = false;

	bRet = FindXmlParamGroup(pNode, pParam, bFirst);
	if (bRet == false)
	{
		return NULL;
	}

	xml_node<XML_TYPE>* pFoundNode = GetLastNode();

	return pFoundNode;
}

xml_node<XML_TYPE>*  CXmlHandler::GetNextXmlSibbling(xml_node<XML_TYPE>*pNode, char *pParam)
{
	if (m_pXmlParser == NULL)
	{
		return NULL;
	}

	if (pNode == NULL)
	{
		return NULL;
    } 

	std::string sNodeName = "";
	std::string sCompName = "";

	if (pParam != NULL)
	{
		sCompName.assign(pParam);
	}

	xml_node<XML_TYPE>* pCurrNode = pNode;
		
	pCurrNode = pCurrNode->next_sibling();
	while (1)
	{
		if (pCurrNode == NULL)
		{
			return NULL;
		}

		if (sCompName != "")
		{
			sNodeName = pCurrNode->name();
			if (sNodeName == sCompName)
			{
				return pCurrNode;
			}

			pCurrNode = pCurrNode->next_sibling();
		}
		else
		{
			return pCurrNode;
		}
	}

	return NULL;
}

bool CXmlHandler::LoadXmlParamBuf(xml_node<XML_TYPE>*pNode, char *pParam, char *pTarget, int nLen, char *pDefault, bool bFirst)
{
	if (m_pXmlParser == NULL)
	{
		return false;
	}

	if (pNode == NULL)
	{
		return false;
    } 

	bool bRet = false;

	std::string sName = "";

	try
	{
		sName.assign(pNode->name(), pNode->name_size());
		if (sName == pParam)	
		{												
			//* we're already there

			m_pLastNode = pNode;

			return true;
		}		

		//* start searching for the desired node

		xml_node<> *xmlSearch;

		if (bFirst)
		{
			xmlSearch = pNode->first_node(0, 0, true);
		}
		else
		{
			xmlSearch = pNode->first_node(pParam, 0, true);
		}

		while (1)
		{
			if (xmlSearch == NULL)
			{
				break;
			}

			sName.assign(xmlSearch->name(), xmlSearch->name_size());
			if (sName == pParam)	
			{												
				int nParamLen = xmlSearch->value_size();			
				if (nParamLen > 0)							
				{
					char *pParamValue = xmlSearch->value();			
					if (nParamLen > nLen)
					{
						strncpy(pTarget, pParamValue, nLen);	
					}
					else
					{
						strncpy(pTarget, pParamValue, nParamLen);	
					}
					m_pLastNode = xmlSearch;
					bRet = true;
				}

				break;
			}		

			xmlSearch = xmlSearch->next_sibling(0, 0, true);
		}

		if ((bRet == false) && (xmlSearch != NULL))
		{
			if (pDefault != NULL)
			{
				int nDefLen = strlen(pDefault);
				if (nDefLen > 0)
				{
					char *pParamValue = xmlSearch->value();			
					if (nDefLen > nLen)
					{
						strncpy(pTarget, pParamValue, nLen);	
					}
					else
					{
						strncpy(pTarget, pParamValue, nDefLen);	
					}
				}
			}
		}
	}
	catch (...)
	{
		return false;
	}

	return bRet;
}

bool CXmlHandler::LoadXmlParamString(xml_node<XML_TYPE>*pNode, char *pParam, std::string &sTarget, char *pDefault, bool bFirst)
{
	if (m_pXmlParser == NULL)
	{
		return false;
	}

	if (pNode == NULL)
	{
		return false;
    } 

	bool bRet = false;

	std::string sName = "";

	try
	{
		xml_node<> *xmlSearch;

		sName.assign(pNode->name(), pNode->name_size());
		if (sName == pParam)	
		{												
			//* we're already there

			xmlSearch = pNode;
		}		
		else
		{
			//* start searching for the desired node

			if (bFirst)
			{
				xmlSearch = pNode->first_node(0, 0, true);
			}
			else
			{
				xmlSearch = pNode->first_node(pParam, 0, true);
			}
		}

		while (1)
		{
			if (xmlSearch == NULL)
			{
				break;
			}

			sName.assign(xmlSearch->name(), xmlSearch->name_size());
			if (sName == pParam)	
			{												
				int nParamLen = xmlSearch->value_size();			
				if (nParamLen > 0)							
				{											
					char *pParamValue = xmlSearch->value();			
					sTarget.assign(pParamValue, nParamLen);		
					m_pLastNode = xmlSearch;
					bRet = true;
				}

				break;
			}		

			xmlSearch = xmlSearch->next_sibling(0, 0, true);
		}

		if (bRet == false)
		{
			if (pDefault != NULL)
			{
				int nLen = strlen(pDefault);

				if (nLen > 0)
				{
					sTarget.assign(pDefault, nLen);
				}
			}
		}
	}
	catch (...)
	{
		return false;
	}

	return bRet;
}


bool CXmlHandler::LoadXmlSubParamString(xml_node<XML_TYPE>*pNode, char *pSubParam, std::string &sTarget, char *pDefault)
{
	if (m_pXmlParser == NULL)
	{
		return false;
	}

	if (pNode == NULL)
	{
		return false;
    } 

	if (pSubParam == NULL)
	{
		return false;
    } 

	bool bRet = false;

	std::string sName = "";

	try
	{
		xml_attribute<> *pAttribute = 
			pNode->first_attribute(pSubParam);
		if (pAttribute != NULL)
		{
			sTarget = pAttribute->value();

			bRet = true;
		}
		else
		{
			sTarget.assign(pDefault);
		}
	}
	catch (...)
	{
		return false;
	}

	return bRet;
}


bool CXmlHandler::LoadXmlParamBool(xml_node<XML_TYPE>*pNode, char *pParam, bool *bTarget, bool bUseDef, bool bDefVal, bool bFirst)
{
	if (m_pXmlParser == NULL)
	{
		return false;
	}

	if (pNode == NULL)
	{
		return false;
    } 

	bool bRet = false;

	std::string sName = "";

	try
	{
		xml_node<> *xmlSearch;

		sName.assign(pNode->name(), pNode->name_size());
		if (sName == pParam)	
		{												
			//* we're already there

			xmlSearch = pNode;
		}		
		else
		{
			//* start searching for the desired node

			if (bFirst)
			{
				xmlSearch = pNode->first_node(0, 0, true);
			}
			else
			{
				xmlSearch = pNode->first_node(pParam, 0, true);
			}
		}

		while (1)
		{
			if (xmlSearch == NULL)
			{
				break;
			}

			sName.assign(xmlSearch->name(), xmlSearch->name_size());
			if (sName == pParam)	
			{												
				int nParamLen = xmlSearch->value_size();			
				if (nParamLen > 0)							
				{
					char *pParamValue = xmlSearch->value();		
					for (int x = 0; x < nParamLen; x++)
					{
						*(pParamValue + x) = tolower(*(pParamValue + x));
					}
					if ((strncmp(pParamValue, "true", nParamLen) == 0) || (strncmp(pParamValue, "1", nParamLen) == 0))
					{
						*bTarget = true;		
						bRet = true;
					}
					else if ((strncmp(pParamValue, "false", nParamLen) == 0) || (strncmp(pParamValue, "0", nParamLen) == 0))
					{
						*bTarget = false;		
						bRet = true;
					}
					else
					{
						bRet = false;
					}
					m_pLastNode = xmlSearch;
				}

				break;
			}	

			xmlSearch = xmlSearch->next_sibling(0, 0, true);
		}
	
		if (bRet == false)
		{
			if (bUseDef == true)
			{
				*bTarget = bDefVal;
			}
		}
	}
	catch (...)
	{
		return false;
	}

	return bRet;
}


bool CXmlHandler::LoadXmlParamInt(xml_node<XML_TYPE>*pNode, char *pParam, int *nTarget, bool bUseDef, int nDefVal, bool bFirst)
{
	if (m_pXmlParser == NULL)
	{
		return false;
	}

	if (pNode == NULL)
	{
		return false;
    } 

	bool bRet = false;

	std::string sName = "";

	try
	{
		xml_node<> *xmlSearch;

		sName.assign(pNode->name(), pNode->name_size());
		if (sName == pParam)	
		{												
			//* we're already there

			xmlSearch = pNode;
		}		
		else
		{
			//* start searching for the desired node

			if (bFirst)
			{
				xmlSearch = pNode->first_node(0, 0, true);
			}
			else
			{
				xmlSearch = pNode->first_node(pParam, 0, true);
			}
		}

		while (1)
		{
			if (xmlSearch == NULL)
			{
				break;
			}

			sName.assign(xmlSearch->name(), xmlSearch->name_size());
			if (sName == pParam)	
			{												
				int nParamLen = xmlSearch->value_size();			
				if (nParamLen > 0)							
				{
					char *pParamValue = xmlSearch->value();
					*nTarget = atoi(pParamValue);
					bRet = true;
					m_pLastNode = xmlSearch;
				}

				break;
			}													
			
			xmlSearch = xmlSearch->next_sibling(0, 0, true);
		}

		if (bRet == false)
		{
			if (bUseDef == true)
			{
				*nTarget = nDefVal;
			}
		}
	}
	catch (...)
	{
		return false;
	}

	return bRet;
}


bool CXmlHandler::LoadXmlParamLong(xml_node<XML_TYPE>*pNode, char *pParam, long *lTarget, bool bUseDef, long lDefVal, bool bFirst)
{
	if (m_pXmlParser == NULL)
	{
		return false;
	}

	if (pNode == NULL)
	{
		return false;
    } 

	bool bRet = false;

	std::string sName = "";

	try
	{
		xml_node<> *xmlSearch;

		sName.assign(pNode->name(), pNode->name_size());
		if (sName == pParam)	
		{												
			//* we're already there

			xmlSearch = pNode;
		}		
		else
		{
			//* start searching for the desired node

			if (bFirst)
			{
				xmlSearch = pNode->first_node(0, 0, true);
			}
			else
			{
				xmlSearch = pNode->first_node(pParam, 0, true);
			}
		}

		while (1)
		{
			if (xmlSearch == NULL)
			{
				break;
			}

			sName.assign(xmlSearch->name(), xmlSearch->name_size());
			if (sName == pParam)	
			{												
				int nParamLen = xmlSearch->value_size();			
				if (nParamLen > 0)							
				{
					char *pParamValue = xmlSearch->value();
					*lTarget = atol(pParamValue);
					bRet = true;
					m_pLastNode = xmlSearch;
				}

				break;
			}													
			
			xmlSearch = xmlSearch->next_sibling(0, 0, true);
		}

		if (bRet == false)
		{
			if (bUseDef == true)
			{
				*lTarget = lDefVal;
			}
		}
	}
	catch (...)
	{
		return false;
	}

	return bRet;
}

