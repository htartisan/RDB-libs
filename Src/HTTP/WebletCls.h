//**************************************************************************************************
//* FILE:		WebletCls.h
//*
//* DESCRIP:	WebletCls (weblet utility) class header file
//*				Helps to sliplify using the "weblet" library code
//*

#include <string>
#include <cstring>
//#include <vector>

#include <stlx.h>

#include "weblet.h"

#include "utils.h"

#include "mutex.h"

//#include "CLinkedList.h"



#ifdef USE_STRING_ASSIGN_MACRO
#define AssignCharPtr2String(s, c)		{int l = strlen(c); if (l > 0){s.assign(c, l);}}	
#define AssignCharPtr2String2(s, c, l)	{if (l > 0){s.assign(c, l);}}	
#else
void AssignCharPtr2String(std::string &s, char *c);
void AssignCharPtr2String2(std::string &s, char *c, int l);
#endif

#define NEW_CMDDATA()		(CWebletCmdData *) NewCWebletCmdData()
//#define NEW_CMDDATA()		new static CNode

#define DEL_CMDDATA(x)		DelCWebletCmdData(x)
//#define DEL_CMDDATA(x)	delete x


#ifndef _WEBLETCLS_H_
#define _WEBLETCLS_H_


#define MAXCON		20					// Weblet max concurrent connections
#define MAXDYNAMIC	20000				// Dynamic page size max... strict

#define WEBLETCLS_NO_CMD_ID					-1

#define WEBLETCLS_NO_ERROR					0
#define WEBLETCLS_ERROR_INVALID_PARAM		10

#define WEBLETCLS_ERROR_INVALID_CMD_ENTRY	15

#define WEBLETCLS_ERROR_NOT_INITIALIZED		20
#define WEBLETCLS_ERROR_ALREADY_INITIALIZED 21

#define WEBLETCLS_ERROR_NOT_CONFIGURED		25

#define WEBLETCLS_ERROR_NOT_STARTED			30
#define WEBLETCLS_ERROR_ALREADY_STARTED		31
#define WEBLETCLS_ERROR_SERVER_START_FAILED	33

#define WEBLETCLS_ERROR_CMD_NOT_DEFINED		40
#define WEBLETCLS_ERROR_CMD_ALREADY_DEFINED 41
#define WEBLETCLS_ERROR_CMD_DEFINE_FAILED	43

#define WEBLETCLS_ERROR_PATH_CREATED_FAILED	45
#define WEBLETCLS_ERROR_PATH_DELETE_FAILED	46

#define WEBLETCLS_ERROR_UNKNOWN				999


void* NewCWebletCmdData();
void* DelCWebletCmdData();



typedef struct WebletCfgData_def
{
	int			nPort;
	int			nMaxConnections;
	std::string sUser;
	std::string sPassword;
    //int			nMaxReplySize;

} WebletCfgData;


class CWebletCmdData
{
public:
	int 		nCmdID;
	
	void 		*pWebletCls;
	
	WEBCALLBACK *pCb;
	
	std::string	sPath;
	std::string	sContent;
	std::string	sLastParams;
	std::string	sLastReply;
	
	void 		*pUserData;

	int			nMaxReplyLen;
	
	bool		bRetType;

	bool		bCreated;

	CWebletCmdData()
	{
		Init();
	}

	void Init()
	{
		nCmdID = 0;

		pWebletCls = NULL;

		pCb = NULL;

		pUserData = NULL;

		nMaxReplyLen = 0;

		bRetType = false;

		bCreated = false;
	}

	~CWebletCmdData()
	{
		
	}
};


typedef stlx::ptr_xmap<int, CWebletCmdData>	WebletCmdList_def;


class CWebletUtil
{
	XP_MUTEX				m_mutex;

protected:
	//static void 			*m_pWebletCls;				
	
	// webserver struct pointer
	//static WEBSERVERTYPE	*m_pServer;		
	WEBSERVERTYPE			*m_pServer;	

	//static int  			m_nError;
	int  					m_nError;

	static bool 			m_bClsInitialized;

	WebletCfgData			m_CfgData;

	bool					m_bCfgSet;
	bool					m_bStarted;

	// CmdList (stlx::ptr_xvector<CWebletCmdData>)
	static WebletCmdList_def m_CmdList;	

	static int 				m_nCmdListLen;
	

public:
	CWebletUtil();

	~CWebletUtil();

	inline void lockResource()
	{
		XP_MUTEX_LOCK(&m_mutex);
	}

	inline void unlockResource()
	{
		XP_MUTEX_UNLOCK(&m_mutex);
	}

protected:
	void clsInit();
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
			int nMaxConnections,
			char *pUser,
			char *pPassword //,
			//int maxReplySize
		);

	int webInit();
	int webInit(WebletCfgData *pCfgData);

	int AddWebletCmdToList(int nCmdID, char *pPath, char *pContent, bool bRet);
	
	int AddWebletWithCallback(WEBCALLBACK *pWebCallback, char *pPath, char *pContent);
	
	int Start();
	int Stop();

	// virtual method to called when web events are processed by the callback
	// over-ride this method with your own handler
	
	virtual int HandleWebEvent (CWebletCmdData *pCmdData, int nCmdID, WEBCALLBACKDATA *pCbData, char *pParams, char *pBuf, int *pLen)
	{
		return WEBLETCLS_NO_ERROR;
	}

	//*
	//* internal methodes... 
	//*

	// class callback handler

	static int ClsCallbackMethod(char *pParams, char *pBuf, int *pLen, void *pUserData);

	int webReplyText(CWebletCmdData * pCmdData, char * pBuf, const std::string &sText);

protected:

	//static WebletCmdData *	GetCmdListEntryByIdx(void *pCls, int nIdx);
	CWebletCmdData *			GetCmdListEntryByIdx(int nIdx);

	//static WebletCmdData *	GetCmdListEntryByID(void *pCls, int nID);
	CWebletCmdData *			GetCmdListEntryByID(int nID);

	//static WebletCmdData *	NewCmdListEntry(void *pCls, int nID);
	CWebletCmdData *			NewCmdListEntry(int nID);

	//static bool				DeleteCmdListEntry(void *pCls, int nID);
	bool						DeleteCmdListEntry(int nID);

	//static bool				ClearCmdList(void *pCls, );
	bool						ClearCmdList();

};


#endif  // _WEBLETCLS_H_

