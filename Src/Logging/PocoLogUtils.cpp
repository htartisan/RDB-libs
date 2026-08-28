//*******************************************************************************
//* FILE:				PocoLogUtils.cpp
//*
//* DESCRIPTION:		LoggerCls utility functions
//*




#include <string>
#include <iostream>


#include "PocoLogUtils.h"


#ifndef _USE_LOGGING_DLL_


#ifdef _USE_POCO_
using Poco::Logger;
using Poco::PatternFormatter;
using Poco::FormattingChannel;
using Poco::ConsoleChannel;
using Poco::SplitterChannel;
using Poco::FileChannel;
using Poco::Message;
#endif

#define THREAD_SAFE(x)	


// this definition seems to be missing from the Msoft SDK
#ifndef ATTACH_PARENT_CONSOLE
#define ATTACH_PARENT_CONSOLE -1
#endif


#ifndef THREAD_DELAY
//#define THREAD_DELAY(x)		XpSleep(x)
#ifdef _USE_POCO_
#define THREAD_DELAY(x)		Thread::sleep(x)
#else
#define THREAD_DELAY(x)		std::this_thread::sleep_for(x)
#endif
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



CAppLogging::CLoggingData::CLoggingData() :
	m_screen(false),
#ifdef WIN32
	m_sysevent(false),
#endif
	m_rolloverInterval(RO_INTERVAL_DEFAULT),
	m_numFileBackups(NUM_RO_FILES_DEFAULT),
	m_bTerminate(false),
	m_bRotate(false)
{
	m_sFilename = "";
	m_sDir = "";

	m_file = NULL;

	m_console = NULL;
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
		const std::string &folder,
		int backups,
		bool forceRot,
		int rolloverInterval
	) throw(std::runtime_error) :
	m_pLoggingData(NULL) //,
	//m_bForceLogRot(forceRot)
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

	m_pLoggingData->m_numFileBackups = backups;

	m_pLoggingData->m_initType = CAppLogging::CLoggingData::LOG_ROTATION_INTERVAL;

#ifdef DEBUG_LOGGING_LIB
	m_pLoggingData->m_rolloverInterval = RO_INTERVAL_DEFAULT;
#else
	if (rolloverInterval == 0)
	{
		m_pLoggingData->m_rolloverInterval = RO_INTERVAL_DEFAULT;
	}
	else
	{
		m_pLoggingData->m_rolloverInterval = rolloverInterval;
	}
#endif

	m_pLoggingData->m_sFilename = filename;

	m_pLoggingData->m_sDir = folder;

	m_pLoggingData->m_nLocations = destinations;

	//m_pLoggingData->m_bRotate = false;

#ifdef _USE_SYS_LOGGING_
	SysEventCls		sysEvntCls;

	m_sSysEvntLibVer = sysEvntCls.GetVersion();
#endif

	if (forceRot == true)
	{
		m_bForceLogRot = true;
		//rotate();
	}
}


CAppLogging::CAppLogging
	(
		const std::string &module,
		int destinations,
		const std::string &filename,
		const std::string &folder,
		int backups,
		bool forceRot,
		std::string &rolloverInterval
	) throw(std::runtime_error) :
	m_pLoggingData(NULL) //,
	//m_bForceLogRot(forceRot)
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

	m_pLoggingData->m_numFileBackups = backups;

	m_pLoggingData->m_initType = CAppLogging::CLoggingData::LOG_ROTATION_STRING;

#ifdef DEBUG_LOGGING_LIB
	m_pLoggingData->m_rolloverType = "3 minutes";
#else
	m_pLoggingData->m_rolloverType = rolloverInterval;
#endif

	m_pLoggingData->m_sFilename = filename;

	m_pLoggingData->m_sDir = folder;

	m_pLoggingData->m_nLocations = destinations;

	//m_pLoggingData->m_bRotate = false;

#ifdef _USE_SYS_LOGGING_
	SysEventCls		sysEvntCls;

	m_sSysEvntLibVer = sysEvntCls.GetVersion();
#endif

	if (forceRot == true)
	{
		m_bForceLogRot = true;
		//rotate();
	}
}


void CAppLogging::rotate(bool bCloseAndReopen)
{
	if (m_pLoggingData == NULL)
	{
		std::cout << "loggingLib: WARNING - Log rotation called, but logging data buffer = NULL " << std::endl;
		return;
	}

	bool bStatus = false;

	if (m_pLoggingData->m_file != NULL)
	{
		//* if the file is already open, close the log

		if (bCloseAndReopen == true)
		{
			((Logger *)m_pLoggingData->m_file)->close();
		}

		//* perform the file "rotate" operation

		for (int x = m_pLoggingData->m_numFileBackups; x > 0; --x)
		{
			std::string sTmpName =
				make_log_path(x, m_pLoggingData->m_sFilename, m_pLoggingData->m_sDir);

			//* check if this file name already exisits... if so, delete it

			if (fileUtil::fileExists(sTmpName) == true)
			{
				fileUtil::deleteFile(sTmpName);
			}

			std::string sTmpName2 =
				make_log_path((x - 1), m_pLoggingData->m_sFilename, m_pLoggingData->m_sDir);

			//* rename the log file

			if (fileUtil::fileExists(sTmpName2) == true)
			{
				fileUtil::renameFile(sTmpName2, sTmpName);
			}
		}

		//* reopen the log file

		if (bCloseAndReopen == true)
		{
			((Logger *)m_pLoggingData->m_file)->open();
		}
	}
}


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

	std::string sLoggerName;

	if (m_pLoggingData->m_nLocations & LOGTO_CONSOLE)
	{
		//* setup logger channel - console

		ConsoleChannel *pCConsole = new ConsoleChannel;

		//FormattingChannel* pFCConsole = new FormattingChannel;

		//pFCConsole->setChannel(new ConsoleChannel);
		//pFCConsole->open();

		//* create two Logger object

		sLoggerName = "ConsoleLogger";

		Poco::Logger &conLogger =
			Poco::Logger::create
			(
				(const std::string&) sLoggerName, 
				(Poco::Channel*) pCConsole, 
				(int) Message::PRIO_INFORMATION
			);
		
		m_pLoggingData->m_console = &conLogger;
		
		if (m_pLoggingData->m_console == NULL)
		{
			throw std::runtime_error("CAppLogging could not open console");
		}
		else
		{
			((Logger *) m_pLoggingData->m_console)->setLevel(Message::PRIO_TRACE);
		}

		pCConsole->open();
	}

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

		//* setup logger channel - file

#if 1
		FileChannel* pCFile = new FileChannel(sLogFile);

		if (m_pLoggingData->m_numFileBackups > 0)
		{
			switch (m_pLoggingData->m_initType)
			{
			case CAppLogging::CLoggingData::LOG_ROTATION_INTERVAL:
			{
				std::string sType = (stringUtil::tos(m_pLoggingData->m_rolloverInterval) + " seconds");
				pCFile->setProperty("rotation", sType);
			}
			break;

			case CAppLogging::CLoggingData::LOG_ROTATION_STRING:
			{
				pCFile->setProperty("rotation", (m_pLoggingData->m_rolloverType));
			}
			break;

			default:
			{
				throw std::runtime_error("CAppLogging initialization type error");
			}
			break;
			}

			pCFile->setProperty("purgeCount", stringUtil::tos(m_pLoggingData->m_numFileBackups));
		}

		//FormattingChannel* pFCFile = new FormattingChannel;

		//pFCFile->setChannel(pCFile);

#else
		SplitterChannel *pLogFileSplitter = new SplitterChannel();

		if (m_pLoggingData->m_numFileBackups > 0)
		{
			pLogFileSplitter->setProperty("rotation", "100");

			pLogFileSplitter->addChannel(new FileChannel(sLogFile));

			for (int x = 0; x < m_pLoggingData->m_numFileBackups; x++)
			{
				std::string sBULogFile =
					make_log_path((x + 1), m_pLoggingData->m_sFilename, m_pLoggingData->m_sDir);

				pLogFileSplitter->addChannel(new FileChannel(sBULogFile));
			}

			FormattingChannel* pFCFile = new FormattingChannel(NULL, pLogFileSplitter);
		}
#endif
		sLoggerName = "FileLogger";

		//* create two Logger object

		Poco::Logger &fileLogger =
			Poco::Logger::create
			(
				(const std::string&) sLoggerName,
				(Poco::Channel*) pCFile,
				(int) Message::PRIO_WARNING
			);
		
		m_pLoggingData->m_file = &fileLogger;
		
		if (m_pLoggingData->m_file == NULL)
		{
			throw std::runtime_error("CAppLogging could not open log file");
		}
		else
		{
			((Logger *) m_pLoggingData->m_file)->setLevel(Message::PRIO_TRACE);
		}

		if (m_pLoggingData->m_numFileBackups > 0 && m_bForceLogRot == true)
		{
			rotate(false);
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

	if (m_pLoggingData->m_console != NULL)
	{
		((Logger *)m_pLoggingData->m_console)->close();

		m_pLoggingData->m_console = NULL;
	}

	if (m_pLoggingData->m_sysevent == true)
	{
#ifdef _USE_SYS_LOGGING_
		m_pLoggingData->m_systemLog.CloseLog();

		m_pLoggingData->m_sysevent = false;
#endif
	}
}

int CAppLogging::writeMsg(int nLvl, std::string sMsg)
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

	//* if screen logging enabled
	//* write it to the screen immediately

	if (m_pLoggingData->m_screen == true)
	{
		//std::cout << (sMsg);
		std::cout << (sMsg) << std::endl;

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

	Poco::Logger *pLogChannel = NULL;

	switch (nLvl)
	{
	case LOG_LEVEL_ERROR:
		if (m_pLoggingData->m_console != NULL)
		{
			pLogChannel = (Poco::Logger *) m_pLoggingData->m_console;

			pLogChannel->error(sMsg);

			nRet += LOGTO_CONSOLE;
		}

		if (m_pLoggingData->m_file != NULL)
		{
			pLogChannel = (Poco::Logger *) m_pLoggingData->m_file;

			pLogChannel->error(sMsg);

			nRet += LOGTO_FILE;
		}

		break;

	case LOG_LEVEL_WARNING:
		if (m_pLoggingData->m_console != NULL)
		{
			pLogChannel = (Poco::Logger *) m_pLoggingData->m_console;

			pLogChannel->warning(sMsg);

			nRet += LOGTO_CONSOLE;
		}

		if (m_pLoggingData->m_file != NULL)
		{
			pLogChannel = (Poco::Logger *) m_pLoggingData->m_file;

			pLogChannel->warning(sMsg);

			nRet += LOGTO_FILE;
		}

		break;

	case LOG_LEVEL_INFO:
		if (m_pLoggingData->m_console != NULL)
		{
			pLogChannel = (Poco::Logger *) m_pLoggingData->m_console;

			pLogChannel->information(sMsg);

			nRet += LOGTO_CONSOLE;
		}

		if (m_pLoggingData->m_file != NULL)
		{
			pLogChannel = (Poco::Logger *) m_pLoggingData->m_file;

			pLogChannel->information(sMsg);

			nRet += LOGTO_FILE;
		}

		break;

	case LOG_LEVEL_DEBUG:
		if (m_pLoggingData->m_console != NULL)
		{
			pLogChannel = (Poco::Logger *) m_pLoggingData->m_console;

			pLogChannel->debug(sMsg);

			nRet += LOGTO_CONSOLE;
		}

		if (m_pLoggingData->m_file != NULL)
		{
			pLogChannel = (Poco::Logger *) m_pLoggingData->m_file;

			pLogChannel->debug(sMsg);

			nRet += LOGTO_FILE;
		}

		break;

	default:
		if (m_pLoggingData->m_console != NULL)
		{
			pLogChannel = (Poco::Logger *) m_pLoggingData->m_console;

			pLogChannel->trace(sMsg);

			nRet += LOGTO_CONSOLE;
		}

		if (m_pLoggingData->m_file != NULL)
		{
			pLogChannel = (Poco::Logger *) m_pLoggingData->m_file;

			pLogChannel->trace(sMsg);

			nRet += LOGTO_FILE;
		}

		break;
	}

	return 0;
};



int CAppLogging::writeMsg(int nLvl, std::string sPrefix, std::string sMsg)
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

	std::string sExtMsg = "";

	if (sPrefix != "")
	{
		sExtMsg = (sPrefix + " [" + stringUtil::tos(nLvl) + "]:");

		while (sExtMsg.length() < MSG_PREFIX_LEN)
		{
			sExtMsg.append(" ");
		}
	}

	sExtMsg.append(sMsg);

	writeMsg(nLvl, sExtMsg);

	return 0;
};


#endif


//* Logging util functions

int SetLoggingDestinations
	(
		//bool bScreen,
		bool bConsole,
#ifdef WIN32
		bool bSysEvemntLog,
#endif
		bool bFile
	)
{
	return
		(
#ifdef _USE_LOGGING_DLL_
		//((g_Options.m_screenLogging == true) ? (loggerCls::LOGTO_SCREEN) : 0) |
		((g_Options.m_bConsoleLogging == true) ? (loggerCls::LOGTO_CONSOLE) : 0) |
#ifdef WIN32
		((g_Options.m_bSysEventLogging == true) ? (loggerCls::LOGTO_SYSEVENT) : 0) |
#endif
		((g_Options.m_bFileLogging == true) ? (loggerCls::LOGTO_FILE) : 0)
#else
		//((bScreen == true) ? (CAppLogging::LOGTO_SCREEN) : 0) |
		((bConsole == true) ? (CAppLogging::LOGTO_CONSOLE) : 0) |
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

