//*******************************************************************************
//* FILE:				simpleLog.h
//*
//* DESCRIPTION:		simple log functions  
//*



#ifndef _SIMPLELOG_H_
#define _SIMPLELOG_H_



	

#define LOG_LEVEL_ERROR			0
#define LOG_LEVEL_WARNING		1
#define LOG_LEVEL_INFO			2
#define LOG_LEVEL_DEBUG			3

#define MSG_PREFIX_LEN			12


typedef enum 
{
	LOG_ROTATION_TYPE_ERROR,
	LOG_ROTATION_INTERVAL,
	LOG_ROTATION_STRING

} eRotationType_def;


tyepdef struct
{
		bool			m_screen;

		void			*m_file;

		std::string		m_sFilename;
		std::string		m_sDir;

	
} loggingData_def;


extern loggingData_def	g_loggingData;


bool logOpen();

void logClose();

void logWrite(const int nLogLevel, const std::string &sMsg);
void logWrite(const int nLogLevel, const std::string &sPrefix, const std::string &sMsg);

#define ELOG(x)		{ if (g_Log) g_Log->writeMsg(LOG_LEVEL_ERROR, x); }
#define WLOG(x)		{ if (g_Log && (g_Log->m_nLogLevel > 0)) g_Log->writeMsg(LOG_LEVEL_WARNING, x); }
#define LOG(x)		{ if (g_Log && (g_Log->m_nLogLevel > 1)) g_Log->writeMsg(LOG_LEVEL_INFO, x); }
#define DLOG(x)		{ if (g_Log && (g_Log->m_nLogLevel > 2)) g_Log->writeMsg(LOG_LEVEL_DEBUG, x); }
#define TLOG(x)		{ if (g_Log && (g_Log->m_nLogLevel > 3)) g_Log->writeMsg(LOG_LEVEL_TRACE, x); }
#define T2LOG(x)	{ if (g_Log && (g_Log->m_nLogLevel > 3)) g_Log->writeMsg((LOG_LEVEL_TRACE_2), x); }
#define T3LOG(x)	{ if (g_Log && (g_Log->m_nLogLevel > 3)) g_Log->writeMsg((LOG_LEVEL_TRACE_3), x); }

#define LOGMSG(n, x)	{ if (g_Log) g_Log->writeMsg(n, x); }

#define UnkErrLOG(m) {std::ostringstream Err; Err << m << "ERROR - Unknown exception at file: " << __FILE__ << ", line: " << stringUtil::tos(__LINE__); ELOG(Err.str());}


