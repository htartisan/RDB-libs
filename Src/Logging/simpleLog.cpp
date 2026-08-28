//*******************************************************************************
//* FILE:				simpleLog.cpp
//*
//* DESCRIPTION:		simple logging functions
//*




#include <string>
#include <iostream>


#include "simpleLog.h"





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




bool logOpen()
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

	if (m_pLoggingData->m_nLocations & LOGTO_FILE)
	{
		std::string sLogFile =
			make_log_path(0, m_pLoggingData->m_sFilename, m_pLoggingData->m_sDir);

		//* setup logger channel - file

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



bool logClose()
{
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

}

int CAppLogging::writeMsg(int nLvl, std::string sMsg)
{
	if (nLvl > m_nLogLevel)
	{
		return -1;
	}

	int nRet = 0;

	//* if screen logging enabled
	//* write it to the screen immediately

	if (m_pLoggingData->m_screen == true)
	{
		//std::cout << (sMsg);
		std::cout << (sMsg) << std::endl;

		nRet += LOGTO_SCREEN;
	}


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


