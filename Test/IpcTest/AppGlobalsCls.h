/// 
/// \file       AppGlobalsCls.h
/// 
///             Global app definitions and include files
///


#ifndef GLOBAL_APP_DEFS_H
#define GLOBAL_APP_DEFS_H


#include <vector>
#include <string>
#include <memory>
#include <filesystem>

#include <Logging/Logging.h>
#include <argspp-lib/src/args.h>

#include "AppUtils.h"


/// @class  AppGlobalsCls
/// A struct contains global variables and functions
struct AppGlobalsCls
{
    std::string                     m_sAppName;
    std::string                     m_sAppVersion;

    std::string                     m_sCWD;

    CLogger                         m_logger;

    std::string                     m_sLogFile;
    std::string                     m_sLogDir;

    int                             m_nLogLevel;

    std::string                     m_sTestInputFile;
    std::string                     m_sTestOutputFile;

    int                             m_nSecurityFlag;    

    bool                            m_bGenerateTestData;

    bool                            m_bSetIpcStartOnLaunch;    

    // app exit control variabled

    AppGlobalsCls()
    {
        m_sAppName = "";
        m_sAppVersion = "";
        m_sCWD = "";
        m_sLogFile = "";
        m_sLogDir = "";
        m_nLogLevel = LogLevel_useDefault;
        m_sTestInputFile = "";
        m_sTestOutputFile = "";
        m_nSecurityFlag = 0777;         // default to full access for everyone
        m_bSetIpcStartOnLaunch = false;
        m_bGenerateTestData = false;
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

        m_sCWD = getCWD();

        return true;
    }

    bool parseCommandLine(int argc, char* argv[])
    {
        std::vector<std::string> argsList;

        LogDebug("Command line args: ");
        for (auto a = 0; a < argc; a++)
        {
            std::string sTmp = argv[a];
            argsList.push_back(sTmp);
            LogDebug("  {}", *argv[a]);
        }

        std::string sUsageText = "usage: ";

        try
        {
            sUsageText.append(m_sAppName);
            sUsageText.append(" section... ");

            args::ArgParser parser(sUsageText, "1.0");

            parser.flag("help h");

            parser.option("logFile o", (m_sAppName + "_log.txt"));
            parser.option("logDir f", ".");
            parser.option("logLevel l", "");

            parser.option("testInputFile i", ".");
            parser.option("testOutputFile o", ".");

            parser.option("securityAccess a", "");

            parser.flag("startOnLaunch s");
            
            parser.parse(argsList);

            m_sLogFile = parser.value("logFile");
        
            m_sLogDir = parser.value("logDir");

            if (parser.found("logLevel") == true)
            {
                m_nLogLevel =   atoi(parser.value("logLevel").c_str());
            }
            else
            {
                m_nLogLevel =   LogLevel_useDefault;
            }

            if (parser.found("testInputFile") == true)
            {
                m_sTestInputFile = parser.value("testInputFile");
            }

            if (parser.found("testOutputFile") == true)
            {
                m_sTestOutputFile = parser.value("testOutputFile");
            }

            if (parser.found("securityAccess") == true)
            {
                std::string sSecFlag = parser.value("securityAccess");
                if (sSecFlag.empty() == false)
                {
                    // if the string starts with 0, parse as octal, otherwise parse as decimal
                    if (sSecFlag[0] == '0')
                    {
                        m_nSecurityFlag = std::stoi(sSecFlag, nullptr, 8);  // parse as octal
                    }
                    else
                    {
                        m_nSecurityFlag = std::stoi(sSecFlag, nullptr, 10);  // parse as decimal
                    }
                }
                else
                {
                    m_nSecurityFlag = 0777;  // default to full access for everyone
                }
            }

            if (parser.found("startOnLaunch") == true)
            {
                m_bSetIpcStartOnLaunch = true;
            }

            if (parser.found("help") == true)
            {
    #ifdef DEBUG
                parser.print();
    #endif
                return false;
            }
        }
        catch(const std::exception& e)
        {
            LogDebug("problem parsing command line args - {} ", e.what());
            return false;
        }
        catch(...)
        {
            LogDebug("unknown exceptio parsing command line args ");
            return false;
        }
        
        return true;
    }

    // do this after parsing command line
    bool setLoggerConfig(const eLogLevel logLevel = LogLevel_useDefault)
    {
        if (m_nLogLevel == LogLevel_useDefault)
        {
            if (logLevel != LogLevel_useDefault)
            {
                m_nLogLevel = logLevel;
            }
            else
            {
                m_nLogLevel = LogLevel_info;
            }
        }

        m_logger.setLogFile
        (
            m_sLogFile,
            m_sLogDir
        );

        m_logger.Init(m_sAppName, (eLogLevel)m_nLogLevel);

        return true;
    }


};


#endif  // GLOBAL_APP_DEFS_H
