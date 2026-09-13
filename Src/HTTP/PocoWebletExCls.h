//**************************************************************************************************
//* FILE:		PocoWebletExCls.cpp
//*
//* DESCRIP:	PocoWebletExCls (weblet utility) class
//*				This class impliments a "weblet" (REST-ful server)
//*				using the Poco HTTP server classes
//*


#ifndef _POCOWEBLETEXCLS_H_
#define _POCOWEBLETEXCLS_H_

#ifndef _POCOWEBLETCLS_H_

//#include <WinHttp.h>

#define POCO_HAVE_ALIGNMENT

#ifdef POCO_SSL
#define SUPPORT_POCO_SSL
#endif


#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <cstring>
#ifdef SUPPORT_POCO_SSL
#include "Poco/Net/HTTPServerRequestImpl.h"
#endif
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#ifdef SUPPORT_POCO_SSL
#include "Poco/Net/SecureStreamSocket.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Net/X509Certificate.h"
#endif
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/ThreadPool.h>
#include <Poco/Mutex.h>
#include <Poco/ScopedLock.h>
#ifdef SUPPORT_POCO_SSL
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/KeyConsoleHandler.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#endif

#include <iostream>

#include "stringUtils.h"
#include "stlx.h"



#ifdef SUPPORT_POCO_SSL
using Poco::Net::SecureServerSocket;
using Poco::Net::SecureStreamSocket;
#endif
using Poco::Net::ServerSocket;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
#ifdef SUPPORT_POCO_SSL
using Poco::Net::HTTPServerRequestImpl;
#endif
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
#ifdef SUPPORT_POCO_SSL
using Poco::Net::X509Certificate;
#endif
using Poco::URI;
using Poco::ThreadPool;
//using Poco::Mutex;
using Poco::ScopedLock;
#ifdef SUPPORT_POCO_SSL
using Poco::Net::SSLManager;
using Poco::Net::Context;
using Poco::Net::KeyConsoleHandler;
using Poco::Net::PrivateKeyPassphraseHandler;
using Poco::Net::InvalidCertificateHandler;
using Poco::Net::AcceptCertificateHandler;
#endif

#endif

typedef	int WEBLETEXCALLBACK
	(
		unsigned int nPort, 
		const std::string &sCmd, 
		const std::string &sReq, 
		const std::string &sParams, 
		std::string &sReply, 
		int &nReplyLen,
		void *pUserData
	);

typedef	int WEBEXCALLBACK
	(
		unsigned int nPort, 
		int nCmdID, 
		const std::string &sReq, 
		const std::string &sParams, 
		std::string &sReply,
		int &nReplyLen
	);

#ifndef _POCOWEBLETCLS_H_

#ifdef USE_STRING_ASSIGN_MACRO
#define AssignCharPtr2String(s, c)		{int l = strlen(c); if (l > 0){s.assign(c, l);}}	
#define AssignCharPtr2String2(s, c, l)	{if (l > 0){s.assign(c, l);}}	
#else
void AssignCharPtr2String(std::string &s, char *c);
void AssignCharPtr2String2(std::string &s, char *c, int l);
#endif


std::string getHttpPath(HTTPServerRequest& request);

std::string getHttpQuery(HTTPServerRequest& request);

std::string getHttpParams(HTTPServerRequest& request);


#define NEW_CMDDATA()		(CWebletCmdData *) NewCWebletCmdData()
//#define NEW_CMDDATA()		new static CNode

#define DEL_CMDDATA(x)		DelCWebletCmdData(x)
//#define DEL_CMDDATA(x)	delete x


#define MAXCON		20					// Weblet max concurrent connections
#define MAXDYNAMIC	20000				// Dynamic page size max... strict

#define WEBLETCLS_NO_ERROR					0
#define WEBLETCLS_NO_ERROR_WITH_REPLY		1
#define WEBLETCLS_NO_ERROR_WITH_EXREPLY		2

#define WEBLETCLS_NO_CMD_ID					-1

#define WEBLETCLS_ERROR_UNKNOWN				-999

#define WEBLETCLS_ERROR_CMD_ID				-5

#define WEBLETCLS_ERROR_INVALID_PARAM		-10

#define WEBLETCLS_ERROR_INVALID_CMD_ENTRY	-15

#define WEBLETCLS_ERROR_NOT_INITIALIZED		-20
#define WEBLETCLS_ERROR_ALREADY_INITIALIZED -21

#define WEBLETCLS_ERROR_NOT_CONFIGURED		-25

#define WEBLETCLS_ERROR_NOT_STARTED			-30
#define WEBLETCLS_ERROR_ALREADY_STARTED		-31
#define WEBLETCLS_ERROR_SERVER_START_FAILED	-33

#define WEBLETCLS_ERROR_CMD_NOT_DEFINED		-40
#define WEBLETCLS_ERROR_CMD_ALREADY_DEFINED -41
#define WEBLETCLS_ERROR_CMD_DEFINE_FAILED	-43

#define WEBLETCLS_ERROR_PATH_CREATE_FAILED	-45
#define WEBLETCLS_ERROR_PATH_DELETE_FAILED	-46

#define WEBLETCLS_ERROR_NO_CMDS_DEFINED		-50
#define WEBLETCLS_ERROR_INVALID_CMD_CONFIG	-51


class CPocoWeblet;


PVOID NewCWebletCmdData();
PVOID DelCWebletCmdData();


typedef struct WebletCfgData_def
{
	unsigned int	nPort;

	bool			bHTML;

	int				nMaxQueued;
	int				nMaxThreads;
	
	std::string		sUser;
	std::string		sPassword;

} WebletCfgData;


class CWebletCmdData
{
public:
	void 				*m_pWebletCls;
	
	int 				m_nCmdID;

	//unsigned int		m_nPort;

	std::string			m_sPath;		//* ie: Cmd
	std::string			m_sContent;

	std::string			m_sLastParams;
	std::string			m_sLastReply;
	
	void 				*m_pUserData;

	WEBLETEXCALLBACK	*m_pCb;
	
	int					m_nMaxReplyLen;
	
	bool				m_bRetType;

	bool				m_bCreated;
	bool				m_bStarted;

	CWebletCmdData()
	{
		Init();
	}

	void Init()
	{
		m_pWebletCls = NULL;

		m_nCmdID = 0;

		//m_nPort = 0;

		m_pCb = NULL;

		m_pUserData = NULL;

		m_sContent = "";

		m_nMaxReplyLen = 0;

		m_bRetType = false;

		m_bCreated = false;
		m_bStarted = false;
	}

	~CWebletCmdData()
	{
		
	}
};


//typedef stlx::ptr_xmap<int, CWebletCmdData>	WebletCmdList_def;
//typedef stlx::ptr_xvector<CWebletCmdData>	WebletCmdList_def;
typedef stlx::ptr_xmap<std::string, CWebletCmdData>	WebletCmdList_def;

#endif

class CWebletExReqHandler: public HTTPRequestHandler
{
private:

	Poco::Mutex			m_resourceLock;

	bool				m_bHTML;

#ifdef SUPPORT_POCO_SSL
	bool				m_bSSL;
#endif

	unsigned int		m_nPort;

	WebletCmdList_def	*m_pCmdList;

	std::string			m_sAuthInfo;

	WEBLETEXCALLBACK	*m_pDefCb;

	std::string			m_sResponse;

	std::string			m_sStatus;

	std::string			m_sHtmlTitle;

	std::string			m_sHtmlHeader;

	void ClearStatusText()
	{
		m_sStatus = "";
	}
	
	void UpdateStatusText(std::string sText)
	{
		if (m_sStatus.length() > 0)
		{
			m_sStatus += "\n";
		}

		m_sStatus += sText;
	}

public:

	CWebletExReqHandler()
	{
		m_nPort = 0;

		m_bHTML = false;

#ifdef SUPPORT_POCO_SSL
		m_bSSL = false;
#endif
		m_pCmdList = NULL;

		m_sAuthInfo = "";

		m_sResponse = "";

		m_sStatus = "";

		m_pDefCb = NULL;

		m_sHtmlTitle = "POCO based Weblet class";

		m_sHtmlHeader = "";

		ClearStatusText();
	}

	CWebletExReqHandler
		(
			unsigned int nPort,
			bool bHTML, 
			WebletCmdList_def *pCmdList, 
			const std::string &sAuth = "", 
			WEBLETEXCALLBACK *pDefCb = NULL
		)
	{
		m_nPort = nPort;

		m_bHTML = bHTML;

#ifdef SUPPORT_POCO_SSL
		m_bSSL = false;
#endif

		m_pCmdList = pCmdList;

		m_sAuthInfo = sAuth;

		m_sResponse = "";

		m_pDefCb = pDefCb;

		ClearStatusText();
	}

#ifdef SUPPORT_POCO_SSL
	void SetSSL(bool bVal)
	{
		m_bSSL = bVal;
	}
#endif

	std::string httpRespHeader()
	{
		std::string sOut = "";

		sOut += "<html>";
		sOut += "<head><title>";
		sOut += m_sHtmlTitle;
		sOut += "</title></head>";
		if (m_sHtmlHeader.length() > 0)
		{
			sOut += m_sHtmlHeader;
		}
		sOut += "<body>\n";

		return sOut;
	}

	std::string httpRespFooter()
	{
		std::string sOut = "";

		sOut += "</p></body>";
		sOut += "</html>\n";

		return sOut;
	}

	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

	//virtual int handleCmd(int nCmdID, const std::string &sReq, std::string &sReply)
	//{
	//	return WEBLETCLS_ERROR_NOT_CONFIGURED;
	//}

	void SetHtmlTitle(std::string sTxt)
	{
		m_sHtmlTitle = sTxt;
	}

	void SetHtmlHeader(std::string sTxt)
	{
		m_sHtmlHeader = sTxt;
	}

	std::string GetStatus()
	{
		return m_sStatus;
	}
};


class CWebletExReqHandlerFactory: public HTTPRequestHandlerFactory
{
private:

	unsigned int		m_nPort;

	bool				m_bHTML;

#ifdef SUPPORT_POCO_SSL
	bool				m_bSSL;
#endif

	WebletCmdList_def	*m_pCmdList;

	std::string			m_sAuthInfo;

	WEBLETEXCALLBACK	*m_pDefCb;

public:

	CWebletExReqHandlerFactory()
	{
		m_nPort = 0;

		m_bHTML = false;

#ifdef SUPPORT_POCO_SSL
		m_bSSL = false;
#endif

		m_pCmdList = NULL;

		m_sAuthInfo = "";

		m_pDefCb = NULL;
	}

	CWebletExReqHandlerFactory
		(
			unsigned int nPort,
			bool bHTML,
#ifdef SUPPORT_POCO_SSL
			bool bSSL,
#endif
			WebletCmdList_def *pCmdList, 
			const std::string &sAuth = "", 
			WEBLETEXCALLBACK *pDefCb = NULL
		) 
	{
		m_nPort = nPort;

		m_bHTML = bHTML;

#ifdef SUPPORT_POCO_SSL
		m_bSSL = bSSL;
#endif

		m_pCmdList = pCmdList;

		m_sAuthInfo = sAuth;

		m_pDefCb = pDefCb;
	}

	CWebletExReqHandler* createRequestHandler(const HTTPServerRequest& request);
	
};



class CPocoWebletEx
{
	//XP_MUTEX				m_mutex;

protected:
	
	// webserver struct pointer
	HTTPServer				*m_pServer;	

#ifdef SUPPORT_POCO_SSL
	bool					m_bSupportSSL;
#endif

	WebletCfgData			m_CfgData;

	int  					m_nError;

	bool 					m_bClsInitialized;

	bool					m_bCfgSet;
	bool					m_bStarted;

	WebletCmdList_def		m_CmdList;	

	int 					m_nCmdListLen;
	
	WEBLETEXCALLBACK		*m_pDefCb;
	
public:

#ifdef SUPPORT_POCO_SSL
	CPocoWebletEx(bool bSupportSSL = false, WEBLETEXCALLBACK *pDefCb = NULL);
#else
	CPocoWebletEx(WEBLETEXCALLBACK *pDefCb = NULL);
#endif

	~CPocoWebletEx();

	//inline void lockResource()
	//{
	//	XP_MUTEX_LOCK(&m_mutex);
	//}

	//inline void unlockResource()
	//{
	//	XP_MUTEX_UNLOCK(&m_mutex);
	//}

protected:

#ifdef SUPPORT_POCO_SSL
	void clsInit(bool bSupportSSL = false);
#else
	void clsInit();
#endif	
	

	void clsDeInit();

public:

	// NOTE:  This should only be called once
	// for all instances of this class
	void globalInit()
	{
		m_nCmdListLen = 0;
	}

	// NOTE:  This should only be called once
	// for all instances of this class
	void globalDeInit()
	{
		globalClearCmdList();

		m_nCmdListLen = 0;
	}

	void globalClearCmdList()
	{
		m_CmdList.clear();
	}

	bool IsInitialized()
	{
		return m_bClsInitialized;
	}

	bool IsConfigured()
	{
		return m_bCfgSet;
	}

	bool IsStarted()
	{
		return m_bStarted;
	}

	int Configure
		(
			int nPort,
			bool bHTML = false,
			int nMaxQueued = 100,
			int nMaxThreads = 16,
			const std::string &sUser = "",
			const std::string &sPassword = ""
		);

	int Configure
		(
			int nPort,
			const std::string &sUser,
			const std::string &sPassword
		)
	{
		return Configure(nPort, false, 100, 16, sUser, sPassword);
	};

	int Configure
		(
			int nPort,
			bool bHTML,
			const std::string &sUser,
			const std::string &sPassword
		)
	{
		return Configure(nPort, bHTML, 100, 16, sUser, sPassword);
	};

#ifdef SUPPORT_POCO_SSL
	int webInit(bool bEnableSSL = false);
	int webInit(WebletCfgData *pCfgData, bool bEnableSSL = false);
#else
	int webInit();
	int webInit(WebletCfgData *pCfgData);
#endif

	int SetDefaultCallback(WEBLETEXCALLBACK *pWebCallback, const std::string &sContent = "")
	{
		m_pDefCb = pWebCallback;
	
		if (m_pDefCb == NULL)
		{
			return -1;
		}

		return WEBLETCLS_NO_ERROR;
	};
	
	int AddWebletCmdToList
		(
			int nCmdID,								//* Unique CMD ID
			const std::string &sPath,				//* Command 'path'
			const std::string &sContent = "",		//* HTTP content type
			bool bRet = false						//* command handler return type
		);
	
	int AddWebletWithCallback
		(
			WEBLETEXCALLBACK *pWebCallback,			//* Ponter to Callback function
			const std::string &sPath,				//* Command 'path'
			const std::string &sContent = "",		//* HTTP content type
			bool bRet = false						//* Callback return type
		);											
	
	//* If return type = 'true' ...
	//* 'HandleWebEvent' method or Callback MUST return
	//* 'WEBLETCLS_NO_ERROR_WITH_REPLY' in order for 
	//* 'sReply' to be sent to client application.

	
	int Start();
	int Stop();

	// virtual method to called when web events are processed by the callback
	// over-ride this method with your own handler
	
	virtual int HandleWebExEvent 
		(
			const unsigned int nPort,
			const int nCmdID,
			const std::string sRew,
			const std::string sParams,
			std::string &sReply,
			int &nReplyLen
		)
	{
		return WEBLETCLS_NO_ERROR;
	}

	//*
	//* internal methodes... 
	//*

	//int ClsCallbackMethod(const std::string &sParams, std::string &sReply, void *pUserData);

	std::string GetParam(const std::string &sContent, const std::string &sParams, const std::string &sName);

protected:

	bool						ClearCmdList();

	int							GetFreeCmdID();

	CWebletCmdData *			NewCmdListEntry(int nID, const std::string &sPath);

	CWebletCmdData *			GetCmdListEntry(const std::string &sPath);

	CWebletCmdData *			GetCmdListEntryByIdx(int nIdx);

	CWebletCmdData *			GetCmdListEntryByID(int nID);

public:

	bool						DeleteCmdListEntry(int nID);
	bool						DeleteCmdListEntry(const std::string &sPath);

	std::string					GetStatus()
	{
		std::string sOut = "";

		try
		{
			if (m_pServer != NULL)
			{
				sOut = ((CWebletExReqHandler *) m_pServer)->GetStatus();
			}
		}
		catch(...)
		{

		}

		return sOut;
	}

};



#endif // _POCOWEBLETEXCLS_H_
