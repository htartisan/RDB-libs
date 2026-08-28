//*******************************************************************************
//* FILE:				PocoLogUtils.h
//*
//* DESCRIPTION:		LoggerCls utility  efs
//*



#ifndef _LOGUTILS_H_
#define _LOGUTILS_H_



#ifdef _USE_LOGGING_DLL_

#include "loggerCls.h"

extern loggerCls				*g_Log;

#else

#ifdef _USE_POCO_

#include "Poco/Logger.h"
#include "Poco/PatternFormatter.h"
#include "Poco/FormattingChannel.h"
#include "Poco/SplitterChannel.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/FileChannel.h"
#include "Poco/Message.h"

#else
	
#include <mutex>
#include <thread>
#include <chrono>

#endif

#ifdef _USE_SYS_LOGGING_
#include "SysEventCls.h"
#endif


#include "../stl/stringUtils.h"
#include "../file/FileUtils.h"




#define TIME_INTERVAL_SET(x)	(x * 60 * 60)
#define RO_INTERVAL_DEFAULT		TIME_INTERVAL_SET(24)
#define NUM_RO_FILES_DEFAULT	5

#define LOG_LEVEL_ERROR			0
#define LOG_LEVEL_WARNING		1
#define LOG_LEVEL_INFO			2
#define LOG_LEVEL_DEBUG			3

#define MSG_PREFIX_LEN			12


class CAppLogging
{
public:

	struct CLoggingData
	{
	public:
		CLoggingData();

		~CLoggingData();

		bool			m_screen;

#ifdef WIN32
		bool			m_sysevent;

#ifdef _USE_SYS_LOGGING_
		SysEventCls		m_systemLog;
#endif
#endif

		void			*m_console;

		void			*m_file;

		enum eRotationType
		{
			LOG_ROTATION_TYPE_ERROR,
			LOG_ROTATION_INTERVAL,
			LOG_ROTATION_STRING

		};

		eRotationType	m_initType;

		int				m_rolloverInterval;

		std::string		m_rolloverType;

		int				m_numFileBackups;

		int				m_nLocations;

		bool			m_bTerminate;

		bool			m_bRotate;

		std::string		m_sFilename;
		std::string		m_sDir;
	};

	typedef enum
	{
		LOGTO_SCREEN = 1,
		LOGTO_CONSOLE = 2,
		LOGTO_FILE = 4,
#ifdef WIN32
		LOGTO_SYSEVENT = 8
#endif
	} destinations_t;

	CAppLogging
		(
			const std::string &module,
			int destinations,
			const std::string &filename,
			const std::string &folder,
			int backups = 0,				//NUM_RO_FILES_DEFAULT,
			bool forceRot = false,
			int rolloverInterval = RO_INTERVAL_DEFAULT
		) throw(std::runtime_error);

	CAppLogging
		(
			const std::string &module,
			int destinations,
			const std::string &filename,
			const std::string &folder,
			int backups,
			bool forceRot,
			std::string &rolloverInterval
		) throw(std::runtime_error);

	~CAppLogging() throw();

private:

#ifdef _USE_POCO_
	Poco::Mutex		m_mutex;
#else
	std::mutex		m_mutex;
#endif

#ifdef _USE_SYS_LOGGING_
	std::string		m_sSysEvntLibVer;
#endif

	CLoggingData	*m_pLoggingData;

public:

	//* public data variables

	int				m_nLogLevel;

	bool			m_bForceLogRot;

	//* public member functions

	void rotate(bool bCloseAndReopen = true);

	//void updateFile(const std::string &sPath);

	void open();

	void close();

	int writeMsg(int nLvl, std::string sMsg);

	int writeMsg(int nLvl, std::string sPrefix, std::string sMsg);

};



#define ELOG(x)		{ if (g_Log) g_Log->writeMsg(LOG_LEVEL_ERROR, x); }
#define WLOG(x)		{ if (g_Log && (g_Log->m_nLogLevel > 0)) g_Log->writeMsg(LOG_LEVEL_WARNING, x); }
#define LOG(x)		{ if (g_Log && (g_Log->m_nLogLevel > 1)) g_Log->writeMsg(LOG_LEVEL_INFO, x); }
#define DLOG(x)		{ if (g_Log && (g_Log->m_nLogLevel > 2)) g_Log->writeMsg(LOG_LEVEL_DEBUG, x); }
#define TLOG(x)		{ if (g_Log && (g_Log->m_nLogLevel > 3)) g_Log->writeMsg(LOG_LEVEL_TRACE, x); }
#define T2LOG(x)	{ if (g_Log && (g_Log->m_nLogLevel > 3)) g_Log->writeMsg((LOG_LEVEL_TRACE_2), x); }
#define T3LOG(x)	{ if (g_Log && (g_Log->m_nLogLevel > 3)) g_Log->writeMsg((LOG_LEVEL_TRACE_3), x); }

#define LOGMSG(n, x)	{ if (g_Log) g_Log->writeMsg(n, x); }

#define UnkErrLOG(m) {std::ostringstream Err; Err << m << "ERROR - Unknown exception at file: " << __FILE__ << ", line: " << stringUtil::tos(__LINE__); ELOG(Err.str());}

#endif


int SetLoggingDestinations
	(
		bool bConsole,
#ifdef WIN32
		bool bSysEvemntLog,
#endif
		bool bFile
	);

bool CheckLogLevel(int nLogLevel);

void LogWrite(int nLogLevel, std::string sMsg);
void LogWrite(int nLogLevel, std::string sPrefix, std::string sMsg);


#endif  // _LOGUTILS_H_
