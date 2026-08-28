/// 
/// \file       AppGlobalsCls.h
/// 
///             Global app definitions and include files
///


#ifndef GLOBAL_APP_DEFS_H
#define GLOBAL_APP_DEFS_H


#ifdef WINDOWS
#include "Windows.h"
#endif


#include "Include/SampleType.h"

#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <filesystem>


#include "AppConfigMgr.h"



/// @class  AppGlobalsCls
/// A struct contains global variables and functions
struct AppGlobalsCls
{
    std::string                     m_sAppName;
    std::string                     m_sAppVersion;

    std::string                     m_sCWD;

    CLogger                         m_logger;

    CAppConfigMgr                   m_AppConfigMgr;

    unsigned int                    m_nConfigFileIdx;

    // app exit control variabled

    std::mutex                      m_appExitMutex;

    std::condition_variable         m_appExitSignal;

    bool                            m_bExitApp;



    AppGlobalsCls()
    {
        m_bExitApp = false;

        m_sAppName.clear();
        m_sAppVersion.clear();

        m_sCWD = std::filesystem::current_path().string();
    }

    // do this on app startup
    bool appInit
        (
            const std::string& sName, 
            const std::string& sVersion = ""
        )
    {
        m_sAppName = sName;
        m_sAppVersion = sVersion;

        m_AppConfigMgr.setAppName(sName);

        return true;
    }

    bool parseCommandLine(int argc, char* argv[])
    {
        return m_AppConfigMgr.parseCommandLine(argc, argv);
    }

    // do this after parsing command line
    bool setLoggerConfig(const eLogLevel logLevel = LogLevel_useDefault)
    {
        if (m_AppConfigMgr.m_nLogLevel == LogLevel_useDefault)
        {
            if (logLevel != LogLevel_useDefault)
            {
                m_AppConfigMgr.m_nLogLevel = logLevel;
            }
            else
            {
                m_AppConfigMgr.m_nLogLevel = LogLevel_info;
            }
        }

        m_logger.setLogFile
        (
            m_AppConfigMgr.m_sLogFile,
            m_AppConfigMgr.m_sLogDir
        );

        m_logger.Init(m_sAppName, (eLogLevel)m_AppConfigMgr.m_nLogLevel);

        return true;
    }

    bool loadAppConfig(const unsigned int nCfgIdx)
    {
        return m_AppConfigMgr.parseConfigFile(nCfgIdx);
    }


};



#endif  // GLOBAL_APP_DEFS_H
