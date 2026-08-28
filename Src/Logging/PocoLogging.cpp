//*******************************************************************************
//* FILE:				PocoLogging.cpp
//*
//* DESCRIPTION:		Poco logging defs
//*




#include <string>
#include <iostream>


#include "PocoLogging.h"


#ifndef _USE_LOGGING_DLL_


#define THREAD_SAFE(x)	


// this definition seems to be missing from the Msoft SDK
#ifndef ATTACH_PARENT_CONSOLE
#define ATTACH_PARENT_CONSOLE -1
#endif


#ifndef THREAD_DELAY
//#define THREAD_DELAY(x)		XpSleep(x)
#define THREAD_DELAY(x)		std::this_thread::sleep_for(x)
#endif



#ifdef WIN32
#ifdef _USE_SYS_LOGGING_
void OutputDebugStr(std::string sText)
{
	OutputDebugStringA((LPCSTR)sText.c_str());
}
#endif
#endif

extern CAppLogging				*g_Log;


static std::string make_log_path(int which, const std::string &filename, const std::string &dir = "") throw()
{
	std::string sOut = "";

	if (dir != "")
	{
		sOut.append(dir);

		int x = (int) dir.length();

		if (dir[x - 1] != '\\')
		{
			sOut.append("\\");
		}
	}

	if (which > 0)
	{
		//sOut.append(fileUtil::stripSuffix(filename) + "_" + stringUtil::tos(which) + "." + fileUtil::getSuffix(filename));
		sOut.append(filename + "." + stringUtil::tos(which - 1));
	}
	else
	{
		sOut.append(filename);
	}

	return sOut;
}


std::string formatMsg(const std::string &sLevel, const std::string &sMsg)
{
	std::string sOut = "";
	
	if (sLevel.length() > 0)
	{
		sOut += sLevel;
		sOut += ":";
	}
	else
	{
		sOut += " ";
	}
	
	int nPadLen = (MSG_PREFIX_LEN - sLevel.length()) - 1;
	
	if (nPadLen > 0)
	{
		sOut.append(' ', nPadLen);
	}
	
	sOut += sMsg;
	
	return sOut;
}	


CAppLogging::CLoggingData::CLoggingData() :
#ifdef WIN32
	m_screen(false),
	m_sysevent(false),
#else
	m_screen(false)
#endif
{
	m_sFilename = "";
	m_sDir = "";

	m_file = NULL;

	//m_console = NULL;
}


CAppLogging::CLoggingData::~CLoggingData()
{
	if (m_file != NULL)
	{
		((Logger *)m_file)->close();

		m_file = NULL;
	}

	if (m_console != NULL)
	{
		((Logger *)m_console)->close();

		m_console = NULL;
	}
}






//*******************************************************************************
//* CAppLogging
//*
//* Constructor.
//*		destinations - flags indicating where to log to (file or console or both)
//*		filename	- File to use for file logging
//*

CAppLogging::CAppLogging
	(
		const std::string &module,
		int destinations,
		const std::string &filename,
		const std::string &folder
	) throw(std::runtime_error) :
	m_pLoggingData(NULL) 
{
	m_pLoggingData = new CLoggingData;

	if (m_pLoggingData == NULL)
	{
		throw std::runtime_error("CAppLogging could not allocate data buffer");
	}

	m_nLogLevel = 1;

#ifdef _USE_SYS_LOGGING_
	m_sSysEvntLibVer = "<NOT LOADED>";
#endif

	m_pLoggingData->m_sFilename = filename;

	m_pLoggingData->m_sDir = folder;

	m_pLoggingData->m_nLocations = destinations;

#ifdef _USE_SYS_LOGGING_
	SysEventCls		sysEvntCls;

	m_sSysEvntLibVer = sysEvntCls.GetVersion();
#endif

}


CAppLogging::CAppLogging
	(
		const std::string &module,
		int destinations,
		const std::string &filename,
		const std::string &folder
	) throw(std::runtime_error) :
	m_pLoggingData(NULL) 
{
	m_pLoggingData = new CLoggingData;

	if (m_pLoggingData == NULL)
	{
		throw std::runtime_error("CAppLogging could not allocate data buffer");
	}

	m_nLogLevel = 1;

#ifdef _USE_SYS_LOGGING_
	m_sSysEvntLibVer = "<NOT LOADED>";
#endif

#ifdef DEBUG_LOGGING_LIB
	m_pLoggingData->m_rolloverType = "3 minutes";
#else
	m_pLoggingData->m_rolloverType = rolloverInterval;
#endif

	m_pLoggingData->m_sFilename = filename;

	m_pLoggingData->m_sDir = folder;

	m_pLoggingData->m_nLocations = destinations;

#ifdef _USE_SYS_LOGGING_
	SysEventCls		sysEvntCls;

	m_sSysEvntLibVer = sysEvntCls.GetVersion();
#endif

}


//void CAppLogging::rotate(bool bCloseAndReopen)
//{
//	if (m_pLoggingData == NULL)
//	{
//		std::cout << "loggingLib: WARNING - Log rotation called, but logging data buffer = NULL " << std::endl;
//		return;
//	}

//	bool bStatus = false;

//	if (m_pLoggingData->m_file != NULL)
//	{

//	}
//}


void CAppLogging::open()
{
	if (m_pLoggingData == NULL)
	{
		throw std::runtime_error("CAppLogging data not initialized");
		//return;
	}

	bool bStatus = false;

	if (m_pLoggingData->m_nLocations & LOGTO_SCREEN)
	{
		m_pLoggingData->m_screen = true;
	}

//	if (m_pLoggingData->m_nLocations & LOGTO_CONSOLE)
//	{
//		//* setup logger channel - console


//	}

#ifdef WIN32
	if (m_pLoggingData->m_nLocations & LOGTO_SYSEVENT)
	{
#ifdef _USE_SYS_LOGGING_
		std::string eventLogName("Shepherd " + m_sModuleName);

		m_pLoggingData->m_systemLog.OpenLog(eventLogName);

		m_pLoggingData->m_sysevent = true;
#endif
	}
#endif

	if (m_pLoggingData->m_nLocations & LOGTO_FILE)
	{
		std::string sLogFile =
			make_log_path(0, m_pLoggingData->m_sFilename, m_pLoggingData->m_sDir);


		
		if (m_pLoggingData->m_file == NULL)
		{
			throw std::runtime_error("CAppLogging could not open log file");
		}
		else
		{

		}


		pCFile->open();
	}
}

CAppLogging::~CAppLogging() throw()
{
	if (m_pLoggingData != NULL)
	{
		close();

#ifdef WIN32
#ifdef _USE_SYS_LOGGING_
		if (m_pLoggingData->m_systemLog.GetHandle() != NULL)
		{
			m_pLoggingData->m_systemLog.CloseLog();
		}
#endif
#endif
		delete m_pLoggingData;

		m_pLoggingData = NULL;
	}
}


void CAppLogging::close()
{
	if (m_pLoggingData == NULL)
	{
		return;
	}

	if (m_pLoggingData->m_file != NULL)
	{
		((Logger *)m_pLoggingData->m_file)->close();

		m_pLoggingData->m_file = NULL;
	}

//	if (m_pLoggingData->m_console != NULL)
//	{
//		((Logger *)m_pLoggingData->m_console)->close();
//
//		m_pLoggingData->m_console = NULL;
//	}

	if (m_pLoggingData->m_sysevent == true)
	{
#ifdef _USE_SYS_LOGGING_
		m_pLoggingData->m_systemLog.CloseLog();

		m_pLoggingData->m_sysevent = false;
#endif
	}
}

int CAppLogging::writeMsg(int nLvl, const std::string &sMsg)
{
	if (m_pLoggingData == NULL)
	{
		return -1;
	}

	if (nLvl > m_nLogLevel)
	{
		return -1;
	}

	THREAD_SAFE(m_mutex);

	int nRet = 0;

	std::string sOut = "";

	//* Cue Msg to be written to the console & logfile
	
	switch (nLvl)
	{
	case LOG_LEVEL_ERROR:
		sOut = formatMsg("ERROR", sMsg);
		break;

	case LOG_LEVEL_WARNING:
		sOut = formatMsg("WARNING", sMsg);
		break;

	case LOG_LEVEL_INFO:
		sOut = formatMsg("INFO", sMsg);
		break;

	case LOG_LEVEL_DEBUG:
		sOut = formatMsg("DEBUG", sMsg);
		break;

	default:
		sOut = formatMsg("TRACE", sMsg);
		break;
	}

	//* if screen logging enabled
	//* write it to the screen immediately

	if (m_pLoggingData->m_screen == true)
	{
		std::cout << (sOut) << std::endl;

		nRet += LOGTO_SCREEN;
	}

#ifdef WIN32
	//* if System Event logging enabled
	//* write it to the SysEvent log

	if (m_pLoggingData->m_sysevent == true)
	{
#ifdef _USE_SYS_LOGGING_
		switch (nLvl)
		{
		case LOG_LEVEL_ERROR:
			m_pLoggingData->m_systemLog.LogEvent(EVENTLOG_ERROR_TYPE, (DWORD)0, (sMsg.c_str()), NULL);
			break;

		case LOG_LEVEL_WARNING:
			m_pLoggingData->m_systemLog.LogEvent(EVENTLOG_WARNING_TYPE, (DWORD)0, (sMsg.c_str()), NULL);
			break;

		case LOG_LEVEL_INFO:
			m_pLoggingData->m_systemLog.LogEvent(EVENTLOG_INFORMATION_TYPE, (DWORD)0, (sMsg.c_str()), NULL);
			break;
		}

		nRet += LOGTO_SYSEVENT;
#endif
	}
#endif

	//* Cue Msg to be written to the console & logfile
	
	
//	if (m_pLoggingData->m_console != NULL)
//	{
//
//		nRet += LOGTO_CONSOLE;
//	}

	if (m_pLoggingData->m_file != NULL)
	{
		m_pLoggingData->m_file->write(sOut);

		nRet += LOGTO_FILE;
	}

	return nRet;
};



int CAppLogging::writeMsg(int nLvl, const std::string &sPrefix, const std::string &sMsg)
{
	if (m_pLoggingData == NULL)
	{
		return -1;
	}

	if (nLvl > m_nLogLevel)
	{
		return -1;
	}

	THREAD_SAFE(m_mutex);

	int nRet = 0;

	std::string sOut = "";

	sOut = formatMsg(sPrefix, sMsg);

	writeMsg(nLvl, sOut);

	return 0;
};


#endif


//* Logging util functions

int SetLoggingDestinations
	(
		bool bScreen,
		//bool bConsole,
#ifdef WIN32
		bool bSysEvemntLog,
#endif
		bool bFile
	)
{
	return
		(
#ifdef _USE_LOGGING_DLL_
		((g_Options.m_screenLogging == true) ? (loggerCls::LOGTO_SCREEN) : 0) |
		//((g_Options.m_bConsoleLogging == true) ? (loggerCls::LOGTO_CONSOLE) : 0) |
#ifdef WIN32
		((g_Options.m_bSysEventLogging == true) ? (loggerCls::LOGTO_SYSEVENT) : 0) |
#endif
		((g_Options.m_bFileLogging == true) ? (loggerCls::LOGTO_FILE) : 0)
#else
		((bScreen == true) ? (CAppLogging::LOGTO_SCREEN) : 0) |
		//((bConsole == true) ? (CAppLogging::LOGTO_CONSOLE) : 0) |
#ifdef WIN32
#ifdef _USE_SYS_LOGGING_
		((bSysEvemntLog == true) ? (CAppLogging::LOGTO_SYSEVENT) : 0) |
#endif
#endif
		((bFile == true) ? (CAppLogging::LOGTO_FILE) : 0)
#endif
		);
}


bool CheckLogLevel(int nLogLevel)
{
	if (g_Log == NULL)
	{
		return false;
	}

	if (g_Log->m_nLogLevel >= nLogLevel)
	{
		return true;
	}
	
	return false;
}

void LogWrite(int nLogLevel, std::string sMsg)
{
	if (g_Log == NULL)
	{
		return;
	}

	if (nLogLevel > g_Log->m_nLogLevel)
	{
		return;
	}

	g_Log->writeMsg(nLogLevel, sMsg);
}


void LogWrite(int nLogLevel, std::string sPrefix, std::string sMsg)
{
	if (g_Log == NULL)
	{
		return;
	}

	if (nLogLevel > g_Log->m_nLogLevel)
	{
		return;
	}

	g_Log->writeMsg(nLogLevel, sPrefix, sMsg);
}

