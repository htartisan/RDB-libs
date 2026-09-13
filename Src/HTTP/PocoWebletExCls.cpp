//**************************************************************************************************
//* FILE:		PocoWebletExCls.cpp
//*
//* DESCRIP:	PocoWebletExCls (weblet utility) class
//*				This class impliments a "weblet" (REST-ful server)
//*				using the Poco HTTP server classes
//*




#ifdef WIN32
#define _WIN32_WINNT	0x0601

#include <SDKDDKVer.h>
#include <cstring>

#define _WINSOCKAPI_ 

#include <windows.h>
#include <WinHttp.h>

//#else

#endif

#include <string>
#include <stdexcept>
#include <vector>

#include "PocoWebletExCls.h"


#ifndef _POCOWEBLETCLS_H_


//* global methods
//----------------------

PVOID NewCWebletCmdData()
{
	try
	{
		CWebletCmdData *pNew = (CWebletCmdData *) 
			malloc(sizeof(CWebletCmdData)); 

		if (pNew != nullptr) 
		{
			//pNew->Init(); 
			new (pNew) CWebletCmdData();
		}

		return pNew;
	}
	catch(...)
	{

	}

	return nullptr;
}

void DelCWebletCmdData(CWebletCmdData *pNode)
{
	try
	{
		if (pNode != nullptr) 
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

#endif


//* class method definitions

//* Internal method to handle all new HTTP messages

void CWebletExReqHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
	if (m_pCmdList == nullptr)
	{
		// no commands currently defined

		response.setStatus(HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
		return;
	}

	Poco::Mutex::ScopedLock lock(m_resourceLock);

	try
	{
		ClearStatusText();

#ifdef SUPPORT_POCO_SSL
		if (m_bSSL)
		{
			SecureStreamSocket socket = static_cast<HTTPServerRequestImpl&>(request).socket();
			if (socket.havePeerCertificate())
			{
				X509Certificate cert = socket.peerCertificate();

				UpdateStatusText("Client certificate: " + cert.subjectName());
			}
			else
			{
				UpdateStatusText("No client certificate available.");
			}
		}
#endif

		std::string sType = request.getMethod();

		if (sType == "TEARDOWN")
		{
			UpdateStatusText("Received HTTP 'TEARDOWN' operation. ");
			return;
		}

		if (sType != "GET" && sType != "POST" && sType != "PUT")
		{
			UpdateStatusText("Invalid HTTP operation. ");
			response.setStatus(HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
			throw std::runtime_error("HTTP operation type not supported");
		}

		URI url(request.getURI());

		//std::string sHost = request.getHost();
		std::string sPath = stringUtil::toLower(getHttpPath(request));
		std::string sReq = getHttpQuery(request);
		std::string sParams = getHttpParams(request);
		std::string sReply = "";
		std::string sContent = request.getContentType();

		//request.getCookies();

		int nInputLen = (int) request.getContentLength();

		int nStatus = 0;

		int nReplyLen = -1;

		WebletCmdList_def::iterator x = m_pCmdList->find(sPath);
		if (x == m_pCmdList->end())
		{
			if (m_pDefCb == nullptr)
			{
				// we did not find a registered weblet for this command
				// so return a "404" error to the calling server

				response.setStatus(HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
				UpdateStatusText("Server command not found. ");
				throw std::runtime_error("Server command not found");
			}

			nStatus =
				m_pDefCb
				(
					m_nPort,
					sPath,
					sReq,
					sParams,
					sReply,
					nReplyLen,
					nullptr
				);
		}
		else
		{
			WEBLETEXCALLBACK *pCallback = ((*x).second->m_pCb);

			CPocoWebletEx *pCls = (CPocoWebletEx *)((*x).second->m_pWebletCls);

			if (pCallback != nullptr)
			{
				nStatus =
					pCallback
					(
						m_nPort,
						sPath,
						sReq,
						sParams,
						sReply,
						nReplyLen,
						((*x).second)
					);
			}
			else if (pCls != nullptr)
			{
				int nCmd = ((*x).second->m_nCmdID);

				nStatus =
					pCls->HandleWebExEvent
					(
						m_nPort,
						nCmd,
						sReq,
						sParams,
						sReply,
						nReplyLen
					);
			}
			else
			{
				nStatus = WEBLETCLS_ERROR_NOT_CONFIGURED;
				UpdateStatusText("Server not configured. ");
				response.setStatus(HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR);
				throw std::runtime_error("Server not configured");
			}

			if ((*x).second->m_sContent != "")
			{
				sContent = ((*x).second->m_sContent);
			}
		}

		if (nStatus == WEBLETCLS_NO_ERROR)
		{
			response.setContentType(sContent);

			response.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);

			std::ostream& osOut = response.send();

			osOut.flush();
		}
		else if ((nStatus == WEBLETCLS_NO_ERROR_WITH_REPLY) || (nStatus == WEBLETCLS_NO_ERROR_WITH_EXREPLY))
		{
			std::string sOutBuf = "";

			if (m_bHTML == true)
			{
				sOutBuf += httpRespHeader();
				sOutBuf += sReply;
				sOutBuf += httpRespFooter();
			}
			else
			{
				sOutBuf = sReply;
			}

			int nOutLen = 0;

			if (nReplyLen > -1)
			{
				nOutLen = nReplyLen;
			}
			else
			{
				nOutLen = sOutBuf.length();
			}

			std::string sContent = request.getContentType();

			if (sType == "PUT")
			{
				//response.setContentType(sContent);
				response.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);

				std::ostream& osOut = 
					response.send();
				osOut.flush();
			}
			else
			{
				if ((nStatus == WEBLETCLS_NO_ERROR_WITH_REPLY) && nOutLen <= 1024)
				{
					response.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
					response.setContentType(sContent);
					response.setContentLength(nOutLen);
					response.setChunkedTransferEncoding(false);

					//response.sendBuffer(sOutBuf.c_str(), sReply.length());
					std::ostream& osOut = 
						response.send();
					osOut << sOutBuf;
					osOut.flush();
				}
				else
				{
					response.setStatus(HTTPResponse::HTTPStatus::HTTP_OK);
					response.setContentType(sContent);

					if (nOutLen <= 1024)
					{
						response.setContentLength(nOutLen);
						response.setChunkedTransferEncoding(false);
					}
					else
					{
						response.setChunkedTransferEncoding(true);
					}

					std::ostream& osOut = 
						response.send();
					osOut << sOutBuf;
					osOut.flush();
				}
			}
		}
		else
		{
			nStatus = WEBLETCLS_ERROR_UNKNOWN;
			UpdateStatusText("Server processing error. ");
			response.setStatus(HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
			throw std::runtime_error("Server processing error");
		}

		return;
	}
	catch(...)
	{

	}
}


//* Internal method to generate a new HTTP message handler

CWebletExReqHandler * CWebletExReqHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
	try
	{
		CWebletExReqHandler * pWebletReqHandler = 
			new CWebletExReqHandler(m_nPort, m_bHTML, m_pCmdList, m_sAuthInfo, m_pDefCb);

#ifdef SUPPORT_POCO_SSL
		if (pWebletReqHandler != nullptr)
		{
			pWebletReqHandler->SetSSL(m_bSSL);
		}
#endif

		return pWebletReqHandler;
	}
	catch(...)
	{

	}

	return nullptr;
}


//* CPocoWeblet class constructor and destructor

#ifdef SUPPORT_POCO_SSL
CPocoWebletEx::CPocoWebletEx(bool bSupportSSL, WEBLETEXCALLBACK *pDefCb)
#else
CPocoWebletEx::CPocoWebletEx(WEBLETEXCALLBACK *pDefCb)
#endif
{
#ifdef SUPPORT_POCO_SSL
	clsInit(bSupportSSL);
#else
	clsInit();
#endif

	m_pDefCb = pDefCb;
}

CPocoWebletEx::~CPocoWebletEx()
{
	clsDeInit();
}


//* CPocoWebletEx class initialization and de-init methods

#ifdef SUPPORT_POCO_SSL
void CPocoWebletEx::clsInit(bool bSupportSSL)
#else
void CPocoWebletEx::clsInit()
#endif
{  
	try
	{
		m_bClsInitialized = false;

		//XP_MUTEX_INIT(&m_mutex);

		m_bCfgSet = false;
		m_bStarted = false;

		m_pServer = nullptr;

		m_pDefCb = nullptr;
	
		m_nError = 0;

		m_pAppData = nullptr;

#ifdef SUPPORT_POCO_SSL
		if (m_bSupportSSL == true)
		{
			Poco::Net::initializeSSL();
		}
#endif

		m_bClsInitialized = true;
	}
	catch(...)
	{

	}
}


void CPocoWebletEx::clsDeInit()
{  
	if (m_bClsInitialized == FALSE)
	{
		return;
	}

	try
	{
		Stop();
	
#ifdef SUPPORT_POCO_SSL
		if (m_bSupportSSL == true)
		{
			Poco::Net::uninitializeSSL();
		}
#endif

		m_bClsInitialized = false;
	}
	catch(...)
	{

	}
}


//* Configure the HTTP server

int CPocoWebletEx::Configure
	(
		int nPort,
		bool bHTML,
		int nMaxQueued,
		int nMaxThreads,
		const std::string &sUser,
		const std::string &sPassword
	)
{  
	if (m_bClsInitialized == FALSE)
	{
		//* not yet initialized
		m_nError = WEBLETCLS_ERROR_NOT_INITIALIZED;
		return m_nError;
    } 

	if (nPort == 0)
	{
		//* not yet initialized
		m_nError = WEBLETCLS_ERROR_INVALID_PARAM;
		return m_nError;
    } 

	try
	{
		m_CfgData.nPort =		nPort;

		m_CfgData.bHTML =		bHTML;

		m_CfgData.nMaxQueued =	nMaxQueued;
		m_CfgData.nMaxThreads =	nMaxThreads;
	
		m_CfgData.sUser =		sUser;
		m_CfgData.sPassword =	sPassword;

		m_bCfgSet = true;

		return WEBLETCLS_NO_ERROR;
	}
	catch(...)
	{

	}

	return -1;
}


//* webInit methods...
//* These methods creates a web server instance

#ifdef SUPPORT_POCO_SSL
int CPocoWebletEx::webInit(bool bEnableSSL)
#else
int CPocoWebletEx::webInit()
#endif
{  
	if (m_bClsInitialized == FALSE)
	{
		// not yet initialized
		m_nError = WEBLETCLS_ERROR_NOT_INITIALIZED;
		return m_nError;
    } 

	if (m_bCfgSet != TRUE)
	{
		//* config data pointer has not bee
		m_nError = WEBLETCLS_ERROR_NOT_CONFIGURED;
		return m_nError;
	}

	if (m_pServer != nullptr)
	{
		// already started... must be stopped first
		m_nError = WEBLETCLS_ERROR_ALREADY_STARTED;
		return m_nError;
    } 

	HTTPServer *pWebServer = nullptr;

	try
	{
		// Create/initialize the webserver 

		int maxQueued  = (m_CfgData.nMaxQueued);
		int maxThreads = (m_CfgData.nMaxThreads);
			
		ThreadPool::defaultPool().addCapacity(maxThreads);
			
		HTTPServerParams* pParams = new HTTPServerParams;
		pParams->setMaxQueued(maxQueued);
		pParams->setMaxThreads(maxThreads);

		ServerSocket *pSvS = nullptr;

#ifdef SUPPORT_POCO_SSL
		if (bEnableSSL)
		{
			// set-up a server socket
			pSvS = (ServerSocket *) new SecureServerSocket(m_CfgData.nPort);
		}
		else
#endif
		{
			// set-up a server socket
			pSvS = (ServerSocket *) new ServerSocket(m_CfgData.nPort);
		}

		if (pSvS == nullptr)
		{
			m_nError = WEBLETCLS_ERROR_SERVER_START_FAILED;
			return m_nError;
		}

		// set-up a HTTPServer instance
		// that will handle all suborinate weblets
#ifdef SUPPORT_POCO_SSL
		CWebletExReqHandlerFactory *pReqHandlerFactory =  new 
			CWebletExReqHandlerFactory((m_CfgData.nPort), (m_CfgData.bHTML), bEnableSSL, &m_CmdList);
#else
		CWebletExReqHandlerFactory *pReqHandlerFactory =  new 
			CWebletExReqHandlerFactory((m_CfgData.nPort), (m_CfgData.bHTML), &m_CmdList);
#endif
		
		if (pReqHandlerFactory == nullptr)
		{
			m_nError = WEBLETCLS_ERROR_SERVER_START_FAILED;
			return m_nError;
		}

		pWebServer = new HTTPServer(pReqHandlerFactory, (*pSvS), pParams);

		if ( pWebServer == nullptr )
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


#ifdef SUPPORT_POCO_SSL
int CPocoWebletEx::webInit(WebletCfgData *pCfg, bool bEnableSSL)
#else
int CPocoWebletEx::webInit(WebletCfgData *pCfg)
#endif
{  
	if (m_bClsInitialized == FALSE)
	{
		//* not yet initialized
		m_nError = WEBLETCLS_ERROR_NOT_INITIALIZED;
		return m_nError;
    } 

	if (pCfg == nullptr)
	{
		m_nError = WEBLETCLS_ERROR_INVALID_PARAM;
		return m_nError;
    } 

	try
	{
		m_CfgData.nPort =			pCfg->nPort;
		m_CfgData.nMaxQueued =		pCfg->nMaxQueued;
		m_CfgData.nMaxThreads =		pCfg->nMaxThreads;
		m_CfgData.sUser =			pCfg->sUser;
		m_CfgData.sPassword =		pCfg->sPassword;

		m_bCfgSet = true;

#ifdef SUPPORT_POCO_SSL
		return webInit(bEnableSSL);
#else
		return webInit();
#endif
	}
	catch(...)
	{

	}

	return -1;
}


//* AddWebletCmdToList method...
//* This method adds a new Command with associated CmdID  

int CPocoWebletEx::AddWebletCmdToList
	(
		int nCmdID, 
		const std::string &sPath, 
		const std::string &sContent, 
		bool bRet
	)
{
	int nRet = 0;

	if (nCmdID < 0)
	{
		if (nCmdID == WEBLETCLS_NO_CMD_ID)
		{
			//* If '-1' is given as the Command ID,
			//* find the 1st avalible Command ID
			//* value... and use that.

			nCmdID = GetFreeCmdID();

			if (nCmdID < 0)
			{
				//* Error trying to get new CmdID
				return WEBLETCLS_ERROR_CMD_ID;
			}

			//* return the value of the new CmdID

			nRet = nCmdID;
		}
		else
		{
			//* Invalid CmdID
			return WEBLETCLS_ERROR_INVALID_PARAM;
		}
	}

	try
	{
		std::string sCmd = "";

		if (sPath[0] != '/')
		{
			sCmd = "/";

			sCmd.append(stringUtil::toLower(sPath));
		}
		else
		{
			sCmd = stringUtil::toLower(sPath);
		}

		CWebletCmdData	*pCmdData = nullptr;

		pCmdData = NewCmdListEntry(nCmdID, sCmd);
		if (pCmdData == nullptr)
		{
			//* Invalid cmd table entry.
			//* NOTE: If there is already
			//* an entry with the same 
			//* CmdID, NewCmdListEntry
			//* will fail (return nullptr).
			return WEBLETCLS_ERROR_INVALID_CMD_ENTRY;
		} 

		pCmdData->m_nCmdID =	nCmdID;

		pCmdData->m_sPath =		sCmd;
		pCmdData->m_sContent =	sContent;
	
		pCmdData->m_bRetType =	bRet;

		pCmdData->m_pCb =		nullptr;

		pCmdData->m_pWebletCls =	(void *) this;

		//* save a pointer to this entry in the weblet's "userdata"
		pCmdData->m_pUserData =	(void *) pCmdData;

		pCmdData->m_bCreated	=	true;
		pCmdData->m_bStarted	=	false;

		return WEBLETCLS_NO_ERROR;
	}
	catch(...)
	{

	}

	return -1;
}


//* AddWebletWithCallback method...
//* This method adds a new Command with a Callback function pointer 

int CPocoWebletEx::AddWebletWithCallback
	(
		WEBLETEXCALLBACK *pWebCallback, 
		const std::string &sPath, 
		const std::string &sContent, 
		bool bRet
	)
{
	//if (pWebCallback == nullptr || sPath == "")
	if (pWebCallback == nullptr)
	{
		return WEBLETCLS_ERROR_INVALID_PARAM;
	}

	try
	{
		std::string sCmd = "";

		if (sPath[0] != '/')
		{
			sCmd = "/";

			sCmd.append(stringUtil::toLower(sPath));
		}
		else
		{
			sCmd = stringUtil::toLower(sPath);
		}

		CWebletCmdData	*pCmdData = nullptr;

		pCmdData = NewCmdListEntry(WEBLETCLS_NO_CMD_ID, sCmd);
		if (pCmdData == nullptr)
		{
			//* Invalid cmd table entry.
			//* NOTE: If there is already
			//* an entry with the same 
			//* CmdID, NewCmdListEntry
			//* will fail (return nullptr).
			return WEBLETCLS_ERROR_INVALID_CMD_ENTRY;
		} 

		pCmdData->m_nCmdID =		WEBLETCLS_NO_CMD_ID;

		pCmdData->m_sPath =			sCmd;
		pCmdData->m_sContent =		sContent;

		pCmdData->m_bRetType =		bRet;

		pCmdData->m_pCb =			(WEBLETEXCALLBACK *) pWebCallback;

		pCmdData->m_pWebletCls =	(void *) this;

		//* save a pointer to this entry in the weblet's "userdata"
		pCmdData->m_pUserData =		(void *) pCmdData;

		pCmdData->m_bCreated	=	true;
		pCmdData->m_bStarted	=	false;

		return WEBLETCLS_NO_ERROR;
	}
	catch(...)
	{

	}

	return -1;
}


//* Start the HTTP server

int CPocoWebletEx::Start()
{  
	int nStatus = WEBLETCLS_NO_ERROR;

	try
	{
		if (m_pServer == nullptr)
		{
			webInit();

			if (m_pServer == nullptr)
			{
				return WEBLETCLS_ERROR_NOT_INITIALIZED;
			}
		} 

		if (m_CmdList.size() < 1)
		{
			nStatus = WEBLETCLS_ERROR_NO_CMDS_DEFINED;
		}
		else
		{
			//* Set up the weblet sub-URLs and assign callback functions
			//* for all entries in the command list

			CWebletCmdData	*pCmdData = nullptr;

			//for (int x = 0; x < m_nCmdListLen; x++)
			for (WebletCmdList_def::iterator x = m_CmdList.begin(); x != m_CmdList.end(); x++)
			{
				pCmdData = (*x).second;
				if (pCmdData == nullptr)
				{
					//* Invalid cmd table entry
					nStatus = WEBLETCLS_ERROR_INVALID_CMD_CONFIG;
					return nStatus;
				} 

				if (pCmdData->m_pWebletCls != (void *) this)
				{
					//* Entry was not created by this class
					//* so skip it
					continue;
				}

				pCmdData->m_bStarted = true;
			}
		}

		// Start the base server that will handle all suborinate weblets

		// start the HTTPServer
		m_pServer->start();

		m_bStarted = true;
	
		return nStatus;
	}
	catch(...)
	{

	}

	return -1;
}


//* Stop the HTTP server

int CPocoWebletEx::Stop()
{  
	if (m_bClsInitialized == FALSE)
	{
		//* not yet initialized
		m_nError = WEBLETCLS_ERROR_NOT_INITIALIZED;
		return m_nError;
    } 

	int nStatus = 0;

	try
	{
		CWebletCmdData *pCmdData = nullptr;
	
		//* Stop everything 
		for (WebletCmdList_def::iterator x = m_CmdList.begin(); x != m_CmdList.end(); x++)
		{
			pCmdData = (*x).second;
			if (pCmdData == nullptr)
			{
				//* Invalid cmd table entry
				m_nError = 3;
				break;
			} 

			if (pCmdData->m_pWebletCls != (void *) this)
			{
				//* Entry was not created by this class
				//* so skip it
				continue;
			}

			//* stop this weblet
	
			pCmdData->m_bStarted = false;
		}

		m_pServer->stop();

		m_bStarted = false;
	
		return WEBLETCLS_NO_ERROR;
	}
	catch(...)
	{

	}

	return -1;
}


// weblet callback function

//int CPocoWebletEx::ClsCallbackMethod
//	(
//		const std::string &sCmd
//		const std::string &sParams, 
//		std::string &sReply, 
//		void *pUserData
//	)
//{
//	if (pUserData == nullptr)
//	{
//		// unable to handle calls to this "callback" 
//		// without valid UserData (pointer to CmdDataEntry)
//
//		return WEBLETCLS_ERROR_INVALID_PARAM;
//	}
//
//	try
//	{
//		CWebletCmdData	*pCmdData = (CWebletCmdData *) pUserData;
//
//		CPocoWebletEx		*pCls = nullptr;
//
//		if (pCmdData != nullptr)
//		{
//			pCls = (CPocoWebletEx *) pCmdData->m_pWebletCls;
//		}
//
//		if (pCls == nullptr)
//		{
//			// invalid weblet class pointer
//
//			return WEBLETCLS_ERROR_INVALID_PARAM;
//		}
//
//		int nStatus = 0;
//
//		int	 nCmdID = pCmdData->m_nCmdID;
//
//		pCmdData->m_sLastParams = sParams;
//
//		nStatus =
//			pCls->HandleWebEvent 
//			(
//				pCmdData, 
//				pCmdData->m_nCmdID, 
//				sParams, 
//				sReply
//			);
//
//		if (nStatus != 0)
//		{
//			return WEBLETCLS_ERROR_UNKNOWN;
//		}
//	
//		if (pCmdData->m_bRetType == true)
//		{
//			return 1;  // Weblet callbacks must return a "1" for HTTP data to be sent to client
//		}
//	}
//	catch(...)
//	{
//		return WEBLETCLS_ERROR_UNKNOWN;
//	}
//
//	// else
//	return 0; 
//}



//*****************************************************************************
//* Internal utility functions


//* Clear the Command list

bool CPocoWebletEx::ClearCmdList()
{
	try
	{
		//* clear (delete) ONLY command entries created by this instance of CPocoWebletEx

		for (WebletCmdList_def::iterator x = m_CmdList.begin(); x != m_CmdList.end(); x++)
		{
			if ((*x).second->m_pWebletCls == this)
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


//* Get a free CmdID

int CPocoWebletEx::GetFreeCmdID()
{
	try
	{
		if (m_CmdList.size() < 1)
		{
			return 1;
		}

		for (int i = 1; i < 9999; i++)
		{
			CWebletCmdData	*pCmdData = GetCmdListEntryByID(i);

			if (pCmdData == nullptr)
			{
				return i;
			}
		}
	}
	catch(...)
	{

	}

	return -1;
}


//* Add a new command list entry

CWebletCmdData * CPocoWebletEx::NewCmdListEntry(int nID, const std::string &sPath)
{
	if (nID < 1 && nID != WEBLETCLS_NO_CMD_ID)
	{
		return nullptr;
	}

	try
	{
		//* 1st find out if this CmdID already exists in the list
	
		if (nID != WEBLETCLS_NO_CMD_ID)
		{
			for (WebletCmdList_def::iterator x = m_CmdList.begin(); x != m_CmdList.end(); x++)
			{
				if (((*x).second) != nullptr)
				{
					if (((*x).second)->m_nCmdID == nID)
					{
						return nullptr;
					}
				}
			}
		}

		std::string sCmd = "";

		if (sPath[0] != '/')
		{
			sCmd = "/";

			sCmd.append(sPath);
		}
		else
		{
			sCmd = sPath;
		}

		//* next find out if this path (sCmd) already exists in the list

		WebletCmdList_def::iterator x = m_CmdList.find(sCmd);
		if (x != m_CmdList.end())
		{
			return nullptr;
		}

		//* create a CWebletCmdData class

		CWebletCmdData *pNewCmdData = NEW_CMDDATA();
		if (pNewCmdData == nullptr)
		{
			//* some kind of class allocation (new) error
			return nullptr;
		}

		//* initialize it

		pNewCmdData->m_pWebletCls = this;

		pNewCmdData->m_sPath = sCmd;

		pNewCmdData->m_nCmdID = nID;

		//* append a new node to m_CmdList 

		m_CmdList[sCmd] = pNewCmdData;

		//* return a pointer to the new entry

		return pNewCmdData;
	}
	catch(...)
	{

	}

	return nullptr;
}


//* Find a Command within the command list

CWebletCmdData * CPocoWebletEx::GetCmdListEntry(const std::string &sPath)
{
	try
	{
		if (m_CmdList.size() < 1)
		{
			return nullptr;
		}

		std::string sCmd = "";

		if (sPath[0] != '/')
		{
			sCmd = "/";

			sCmd.append(sPath);
		}
		else
		{
			sCmd = sPath;
		}

		CWebletCmdData	*pCmdData = nullptr;

		WebletCmdList_def::iterator x = m_CmdList.find(sCmd);
		if (x != m_CmdList.end())
		{
			pCmdData = (*x).second;

			return pCmdData;
		}
	}
	catch(...)
	{

	}

	return nullptr;
}


//* Find a command list entry by it's index

CWebletCmdData * CPocoWebletEx::GetCmdListEntryByIdx(int nIdx)
{
	try
	{
		if (nIdx < 0 || nIdx >= (int) m_CmdList.size())
		{
			return nullptr;
		}

		int nCntr = 0;

		CWebletCmdData	*pCmdData = nullptr;

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

	return nullptr;
}


//* Find a command list entry by it's ID.
//* This will NOT work if Commands were added
//* via AddWebletWithCallback function.

CWebletCmdData * CPocoWebletEx::GetCmdListEntryByID(int nID)
{
	if (nID < 1)
	{
		return nullptr;
	}

	try
	{
		CWebletCmdData	*pCmdData = nullptr;

		for (WebletCmdList_def::iterator x = m_CmdList.begin(); x != m_CmdList.end(); x++)
		{
			if (((*x).second) != nullptr)
			{
				if (((*x).second)->m_nCmdID == nID)
				{
					pCmdData = (*x).second;

					return pCmdData;
				}
			}
		}
	}
	catch(...)
	{

	}

	return nullptr;
}


//***************************************************************************
// Public utility methods


//* Delete a command list entry by it's ID

bool CPocoWebletEx::DeleteCmdListEntry(int nID)
{
	if (nID < 1)
	{
		return nullptr;
	}

	try
	{
		for (WebletCmdList_def::iterator x = m_CmdList.begin(); x != m_CmdList.end(); x++)
		{
			if (((*x).second) != nullptr)
			{
				if (((*x).second)->m_nCmdID == nID)
				{
					m_CmdList.erase(x);

					return true;
				}
			}
		}
	}
	catch(...)
	{

	}

	return false;
}


//* Delete a command list entry by it's 'path' 

bool CPocoWebletEx::DeleteCmdListEntry(const std::string &sPath)
{
	if (sPath == "")
	{
		return nullptr;
	}

	try
	{
		for (WebletCmdList_def::iterator x = m_CmdList.begin(); x != m_CmdList.end(); x++)
		{
			if (((*x).second) != nullptr)
			{
				if (((*x).second)->m_sPath == sPath)
				{
					m_CmdList.erase(x);

					return true;
				}
			}
		}
	}
	catch(...)
	{

	}

	return false;
}




//**************************************************************
//* Assorted support functions

#ifndef USE_STRING_ASSIGN_MACRO
void AssignCharPtr2String(std::string &sStr, char *pStr)
{
	try
	{
		//ASSERT(pStr);
		if (pStr == nullptr)
		{
			sStr.clear();
			return;
		}

		int nLen = strlen(pStr);
		if (nLen > 0)
		{
			sStr.assign(pStr, nLen);
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
		//ASSERT(pStr);
		if (pStr == nullptr)
		{
			sStr.clear();
			return;
		}

		int nTmpLen = strlen(pStr);
		if (nLen < nTmpLen)
		{
			nTmpLen = nLen;
		}
		if (nTmpLen > 0)
		{
			sStr.assign(pStr, nTmpLen);
		}
	}
	catch(...)
	{

	}
}
#endif


std::string getHttpPath(HTTPServerRequest& request)
{
	std::string sOut = "";

	try
	{
		std::string sURI = request.getURI();
		if (sURI.length() < 1)
		{
			return sOut;
		}

		URI url(request.getURI());

		if (url.empty() == true)
		{
			return sOut;
		}

		std::string sPath = "";
	
		//if (url.getPathLen() > 0)
		{
			sPath = url.getPath();
		}

		sOut = sPath;
	}
	catch(...)
	{
		sOut = "";
	}

	return sOut;
}


std::string getHttpQuery(HTTPServerRequest& request)
{
	std::string sOut = "";

	try
	{
		std::string sURI = request.getURI();
		if (sURI.length() < 1)
		{
			return sOut;
		}

		//URI url(request.getURI());

		//if (url.empty() == true)
		//{
		//	return sOut;
		//}

		//* check if there are any query params

		std::size_t nPos = sURI.find("?");
		if (nPos == std::string::npos)
		{
			return sOut;
		}

		std::string sQuery = "";

		sQuery= sURI.substr(nPos);

		sOut = sQuery;
	}
	catch(...)
	{
		sOut = "";
	}

	return sOut;
}


std::string getHttpParams(HTTPServerRequest& request)
{
	std::string sOut = "";

	try
	{
		std::string sTmp = "";

		// get a stream pointer to the input data

		std::istream& isInput = request.stream();

		// get all character from the input stream
		// and store them in a string

		while ((isInput.eof() == false) && (isInput.good() == true))
		{
			sTmp = "";

			isInput >> sTmp;

			sOut.append(sTmp);

			//sOut.append(" ");
		}

		//// make sure the 1st char of the string is not ' '

		//while ((sOut.length() > 0) && (sOut[0] == ' '))
		//{
		//	std::string::iterator i = sOut.begin();
		//	if (i != sOut.end())
		//	{
		//		sOut.erase(i);
		//	}
		//}
	}
	catch(...)
	{
		sOut = "";
	}

	return sOut;
 }
