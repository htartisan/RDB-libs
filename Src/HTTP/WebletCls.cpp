//**************************************************************************************************
//* FILE:		WebletCls.cpp
//*
//* DESCRIP:	WebletCls (weblet utility) class
//*				Helps to sliplify using the "weblet" library code
//*




#include "weblet.h"
#include <cstring>

#ifdef WIN32
#include <windows.h>
//#else

#endif

#include <string>
#include <vector>

#include "webletCls.h"



//* global methods
//----------------------

void* NewCWebletCmdData()
{
	try
	{
		CWebletCmdData *pNew = (CWebletCmdData *) 
			malloc(sizeof(CWebletCmdData)); 

		if (pNew != NULL) 
		{
			//pNew->Init(); 
			new (pNew) CWebletCmdData();
		}

		return pNew;
	}
	catch(...)
	{

	}

	return NULL;
}

void DelCWebletCmdData(CWebletCmdData *pNode)
{
	try
	{
		if (pNode != NULL) 
		{
			//pNode->Free();
			pNode->~CWebletCmdData();
		}

		free(pNode);
	}
	catch(...)
	{

	}
}





//* class static data definitions

//WEBSERVERTYPE *	CWebletUtil::m_pServer;

bool				CWebletUtil::m_bClsInitialized;

WebletCmdList_def	CWebletUtil::m_CmdList;	
	
int					CWebletUtil::m_nCmdListLen;

//int				CWebletUtil::m_nError;




//* class method definitions


CWebletUtil::CWebletUtil()
{
	clsInit();
}

CWebletUtil::~CWebletUtil()
{
	clsDeInit();
}

void CWebletUtil::clsInit()
{  
	try
	{
		m_bClsInitialized = false;

		XP_MUTEX_INIT(&m_mutex);

		m_bCfgSet = false;
		m_bStarted = false;

		m_pServer = NULL;
	
		m_nError = 0;

		if (webInitSocketlib(1) < 0)
		{
			m_nError = 1;
			return;
		}

		m_bClsInitialized = true;
	}
	catch(...)
	{

	}
}


void CWebletUtil::clsDeInit()
{  
	if (m_bClsInitialized == false)
	{
		return;
	}

	try
	{
		Stop();
	
		webInitSocketlib(0);

		m_bClsInitialized = false;
	}
	catch(...)
	{

	}
}




int CWebletUtil::Configure
	(
		int nPort,
		int nMaxConnections,
		char *pUser,
		char *pPassword //,
		//int maxReplySize
	)
{  
	if (m_bClsInitialized == false)
	{
		//* not yet initialized
		m_nError = WEBLETCLS_ERROR_NOT_INITIALIZED;
		return m_nError;
    } 

	if (nPort == 0 || nMaxConnections == 0)
	{
		//* not yet initialized
		m_nError = WEBLETCLS_ERROR_INVALID_PARAM;
		return m_nError;
    } 

	try
	{
		m_CfgData.nPort =			nPort;
		m_CfgData.nMaxConnections = nMaxConnections;
	
		AssignCharPtr2String(m_CfgData.sUser, pUser);
		AssignCharPtr2String(m_CfgData.sPassword, pPassword);

		m_bCfgSet = true;

		return WEBLETCLS_NO_ERROR;
	}
	catch(...)
	{

	}

	return -1;
}


int CWebletUtil::webInit()
{  
	if (m_bClsInitialized == false)
	{
		// not yet initialized
		m_nError = WEBLETCLS_ERROR_NOT_INITIALIZED;
		return m_nError;
    } 

	if (m_pServer != NULL)
	{
		// already started... must be stopped first
		m_nError = WEBLETCLS_ERROR_ALREADY_STARTED;
		return m_nError;
    } 

	if (m_bCfgSet != true)
	{
		//* config data pointer has not bee
		m_nError = WEBLETCLS_ERROR_NOT_CONFIGURED;
		return m_nError;
	}

	WEBSERVERTYPE *pWebServer = NULL;

	try
	{
		// Open/initialize the webserver (Port,  MaxConnections, UserID, Password)

		pWebServer = 
			::webInit
			(
				(m_CfgData.nPort), 
				(m_CfgData.nMaxConnections), 
				(char *) (m_CfgData.sUser.c_str()), 
				(char *) (m_CfgData.sPassword.c_str()), 
				MAXDYNAMIC
			);
		if ( pWebServer == NULL )
		{
			m_nError = WEBLETCLS_ERROR_SERVER_START_FAILED;
			return m_nError;
		} 

		m_pServer = pWebServer;

		return m_nError;
	}
	catch(...)
	{

	}

	return -1;
}


int CWebletUtil::webInit(WebletCfgData *pCfg)
{  
	if (m_bClsInitialized == false)
	{
		//* not yet initialized
		m_nError = WEBLETCLS_ERROR_NOT_INITIALIZED;
		return m_nError;
    } 

	if (pCfg == NULL)
	{
		m_nError = WEBLETCLS_ERROR_INVALID_PARAM;
		return m_nError;
    } 

	try
	{
		m_CfgData.nPort =			pCfg->nPort;
		m_CfgData.nMaxConnections = pCfg->nMaxConnections;
		m_CfgData.sUser =			pCfg->sUser;
		m_CfgData.sPassword =		pCfg->sPassword;

		m_bCfgSet = true;

		return webInit();
	}
	catch(...)
	{

	}

	return -1;
}


int CWebletUtil::AddWebletCmdToList(int nCmdID, char *pPath, char *pContent, bool bRet)
{
	if (nCmdID < 1 || pPath == NULL)
	{
		return WEBLETCLS_ERROR_INVALID_PARAM;
	}

	try
	{
		CWebletCmdData	*pCmdData = NULL;

		pCmdData = NewCmdListEntry(nCmdID);
		if (pCmdData == NULL)
		{
			//* Invalid cmd table entry.
			//* NOTE: If there is already
			//* an entry with the same 
			//* CmdID, NewCmdListEntry
			//* will fail (return NULL).
			return WEBLETCLS_ERROR_INVALID_CMD_ENTRY;
		} 

		//pCmdData->nCmdID =		nCmdID;

		AssignCharPtr2String(pCmdData->sPath, pPath);
		if (*pPath != '/')
		{
			pCmdData->sPath = "/" + pCmdData->sPath;
		}
	
		AssignCharPtr2String(pCmdData->sContent, pContent);
	
		pCmdData->bRetType =	bRet;

		pCmdData->pCb =			&ClsCallbackMethod;

		pCmdData->pWebletCls =	(void *) this;

		//* save a pointer to this entry in the weblet's "userdata"
		pCmdData->pUserData =	(void *) pCmdData;

		pCmdData->bCreated	=	false;

		return WEBLETCLS_NO_ERROR;
	}
	catch(...)
	{

	}

	return -1;
}


int CWebletUtil::AddWebletWithCallback(WEBCALLBACK *pWebCallback, char *pPath, char *pContent)
{
	if (pWebCallback == NULL || pPath == NULL)
	{
		return WEBLETCLS_ERROR_INVALID_PARAM;
	}

	try
	{
		CWebletCmdData	*pCmdData = NULL;

		pCmdData = NewCmdListEntry(WEBLETCLS_NO_CMD_ID);
		if (pCmdData == NULL)
		{
			//* Invalid cmd table entry.
			//* NOTE: If there is already
			//* an entry with the same 
			//* CmdID, NewCmdListEntry
			//* will fail (return NULL).
			return WEBLETCLS_ERROR_INVALID_CMD_ENTRY;
		} 

		pCmdData->nCmdID =		WEBLETCLS_NO_CMD_ID;

		AssignCharPtr2String(pCmdData->sPath, pPath);
		AssignCharPtr2String(pCmdData->sContent, pContent);

		pCmdData->bRetType =	false;

		pCmdData->pCb =			(WEBCALLBACK *) pWebCallback;

		pCmdData->pWebletCls =	(void *) this;

		//* save a pointer to this entry in the weblet's "userdata"
		pCmdData->pUserData =	(void *) pCmdData;

		pCmdData->bCreated	=	false;

		return WEBLETCLS_NO_ERROR;
	}
	catch(...)
	{

	}

	return -1;
}


int CWebletUtil::Start()
{  
	int nStatus = 0;

	try
	{
		if (m_pServer == NULL)
		{
			//* If the weblet server is not already started
			//* try starting it

			nStatus = webInit();
			if (nStatus != WEBLETCLS_NO_ERROR)
			{
				return nStatus;
			}
		} 

		//* Set up the weblet sub-URLs and assign callback functions
		//* for all entries in the command list

		CWebletCmdData	*pCmdData = NULL;

		//for (int x = 0; x < m_nCmdListLen; x++)
		for (WebletCmdList_def::iterator x = m_CmdList.begin(); x != m_CmdList.end(); x++)
		{
			//pCmdData = GetCmdListEntryByIdx(x);
			pCmdData = (*x).second;
			if (pCmdData == NULL)
			{
				//* Invalid cmd table entry
				m_nError = 3;
				break;
			} 

			if (pCmdData->pWebletCls != (void *) this)
			{
				//* Entry was not created by this class
				//* so skip it
				continue;
			}

			// start this weblet

			nStatus = 
				webRegisterDynamicURL
				(
					m_pServer, 
					(char *) pCmdData->sPath.c_str(), 
					(char *) pCmdData->sContent.c_str(), 
					pCmdData->pUserData, 
					pCmdData->pCb
				);

			if (nStatus != 1)
			{
				return WEBLETCLS_ERROR_PATH_CREATED_FAILED;
			}

			pCmdData->bCreated = true;
		}

		m_bStarted = true;
	
		return WEBLETCLS_NO_ERROR;
	}
	catch(...)
	{

	}

	return -1;
}


int CWebletUtil::Stop()
{  
	if (m_bClsInitialized == false)
	{
		//* not yet initialized
		m_nError = WEBLETCLS_ERROR_NOT_INITIALIZED;
		return m_nError;
    } 

	int nStatus = 0;

	try
	{
		CWebletCmdData *pCmdData = NULL;
	
		//* Step everything 
		//for (int x = 1; x < m_nCmdListLen; x++)
		for (WebletCmdList_def::iterator x = m_CmdList.begin(); x != m_CmdList.end(); x++)
		{
			//pCmdData = GetCmdListEntryByIdx(x);
			pCmdData = (*x).second;
			if (pCmdData == NULL)
			{
				//* Invalid cmd table entry
				m_nError = 3;
				break;
			} 

			if (pCmdData->pWebletCls != (void *) this)
			{
				//* Entry was not created by this class
				//* so skip it
				continue;
			}

			//* stop this weblet
	
			nStatus = 
				webDeRegisterPath
				(
					m_pServer, 
					(char *) pCmdData->sPath.c_str()
				);

			if (nStatus != 0)
			{
				return WEBLETCLS_ERROR_PATH_DELETE_FAILED;
			}

			pCmdData->bCreated = false;
		}

		m_bStarted = false;
	
		return WEBLETCLS_NO_ERROR;
	}
	catch(...)
	{

	}

	return -1;
}


// weblet callback function

int CWebletUtil::ClsCallbackMethod(char *pParams, char *pBuf, int *pLen, void *pUserData)
{
	if (pUserData == NULL)
	{
		// unable to handle calls to this "callback" 
		// without valid UserData (pointer to CmdDataEntry)

		return WEBLETCLS_ERROR_INVALID_PARAM;
	}

	try
	{
		WEBCALLBACKDATA *pCbData = (WEBCALLBACKDATA *) pUserData;

		CWebletCmdData	*pCmdData = (CWebletCmdData *) (pCbData->user_data);

		CWebletUtil		*pCls = NULL;

		if (pCmdData != NULL)
		{
			pCmdData->nMaxReplyLen = (pCbData->buffer_len); 

			pCls = (CWebletUtil *) pCmdData->pWebletCls;
		}

		if (pCls == NULL)
		{
			// invalid weblet class pointer

			return WEBLETCLS_ERROR_INVALID_PARAM;
		}

		int nStatus = 0;

		int	 nCmdID = pCmdData->nCmdID;

		//AssignCharPtr2String(pCmdData->sLastParams, pParams);
		AssignCharPtr2String(pCmdData->sLastParams, pBuf);

		if (nCmdID == WEBLETCLS_NO_CMD_ID)
		{
			WEBCALLBACK *pCallback = pCmdData->pCb;

			nStatus =
				pCallback
				(
					pParams, 
					pBuf, 
					pLen, 
					pUserData
				);
		}
		else
		{
			nStatus =
				pCls->HandleWebEvent 
				(
					pCmdData, 
					pCmdData->nCmdID, 
					pCbData,
					pParams, 
					pBuf, 
					pLen
				);
		}

		if (nStatus != 0)
		{
			return WEBLETCLS_ERROR_UNKNOWN;
		}
	
		if (pCmdData->bRetType == true)
		{
			return 1;  // Weblet callbacks must return a "1" for HTTP data to be sent to client
		}
	}
	catch(...)
	{
		return WEBLETCLS_ERROR_UNKNOWN;
	}

	// else
	return 0; 
}

//*****************************************************************************

CWebletCmdData * CWebletUtil::GetCmdListEntryByIdx(int nIdx)
{
	try
	{
		if (nIdx < 0 || nIdx >= (int) m_CmdList.size())
		{
			return NULL;
		}

		int nCntr = 0;

		CWebletCmdData	*pCmdData = NULL;

		for (WebletCmdList_def::iterator x = m_CmdList.begin(); x != m_CmdList.end(); x++)
		{
			if (nCntr == nIdx)
			{
				pCmdData = (*x).second;

				return pCmdData;
			}
		}
	}
	catch(...)
	{

	}

	return NULL;
}

CWebletCmdData * CWebletUtil::GetCmdListEntryByID(int nID)
{
	if (nID < 1)
	{
		return NULL;
	}

	try
	{
		CWebletCmdData	*pCmdData = NULL;

		WebletCmdList_def::iterator x = m_CmdList.find(nID);

		if (x != m_CmdList.end())
		{
			pCmdData = (*x).second;

			return pCmdData;
		}
	}
	catch(...)
	{

	}

	return NULL;
}

CWebletCmdData * CWebletUtil::NewCmdListEntry(int nID)
{
	if (nID < 1 && nID != WEBLETCLS_NO_CMD_ID)
	{
		return NULL;
	}

	try
	{
		//* 1st find out if this CmdID already exists in the list
	
		WebletCmdList_def::iterator x = m_CmdList.find(nID);
		if (x != m_CmdList.end())
		{
			return NULL;
		}

		//* create a CWebletCmdData class

		CWebletCmdData *pNewCmdData = NEW_CMDDATA();
		if (pNewCmdData == NULL)
		{
			//* some kind of class allocation (new) error
			return NULL;
		}

		//* initialize it

		pNewCmdData->pWebletCls = this;

		pNewCmdData->nCmdID = nID;

		//* append a new node to m_CmdList 

		m_CmdList[nID] = pNewCmdData;

		//* return a pointer to the new entry

		return pNewCmdData;
	}
	catch(...)
	{

	}

	return NULL;
}


bool CWebletUtil::DeleteCmdListEntry(int nID)
{
	if (nID < 1)
	{
		return NULL;
	}

	try
	{
		WebletCmdList_def::iterator x = m_CmdList.find(nID);
		if (x != m_CmdList.end())
		{
			m_CmdList.erase(x);

			return true;
		}
	}
	catch(...)
	{

	}

	return false;
}


bool CWebletUtil::ClearCmdList()
{
	try
	{
		//* clear (delete) ONLY command entries created by this instance of CWebletUtil

		for (WebletCmdList_def::iterator x = m_CmdList.begin(); x != m_CmdList.end(); x++)
		{
			if ((*x).second->pWebletCls == this)
			{
				m_CmdList.erase(x);

				x = m_CmdList.begin();
			}
		}

		return true;
	}
	catch(...)
	{

	}

	return false;
}



int CWebletUtil::webReplyText(CWebletCmdData *pCmdData, char *pBuf, const std::string &sText)
{
	try
	{
		int nLen = sText.length();

		if (nLen < 1)
		{
			return nLen;
		}

		if (nLen >= (pCmdData->nMaxReplyLen - 1))
		{
			nLen = (pCmdData->nMaxReplyLen - 2);
		}

		pCmdData->sLastReply = sText;

		strncpy(pBuf, sText.c_str(), nLen);

		*(pBuf + nLen) = 0;
		*(pBuf + (nLen + 1)) = 0;

		return nLen;
	}
	catch(...)
	{

	}

	return -1;
}




#ifndef USE_STRING_ASSIGN_MACRO
void AssignCharPtr2String(std::string &sStr, char *pStr)
{
	try
	{
		sStr.clear();

		//ASSERT(pStr);
		if (pStr == NULL)
		{
			return;
		}

		int nLen = strlen(pStr);
		if (nLen > 0)
		{
			if (nLen < sStr.max_size())
			{
				sStr.assign(pStr, nLen);
			}
			else
			{
				sStr.assign(pStr, (sStr.max_size() - 1));
			}
		}
	}
	catch(...)
	{

	}
}

void AssignCharPtr2String2(std::string &sStr, char *pStr, int nLen)
{
	try
	{
		sStr.clear();

			//ASSERT(pStr);
		if (pStr == NULL)
		{
			return;
		}

		int nTmpLen = strlen(pStr);
		if (nLen < nTmpLen)
		{
			nTmpLen = nLen;
		}
		if (nTmpLen > 0)
		{
			if (nTmpLen < sStr.max_size())
			{
				sStr.assign(pStr, nTmpLen);
			}
			else
			{
				sStr.assign(pStr, (sStr.max_size() - 1));
			}
		}
	}
	catch(...)
	{

	}
}
#endif
