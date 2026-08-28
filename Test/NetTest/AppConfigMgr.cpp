/// 
/// \file       AppConfigMgr.cpp
/// 
///             CAppConfigMgr class function definitions
///


#define _CRT_SECURE_NO_WARNINGS


#include <iostream>
#include <iomanip>
#include <cstdlib>

#include <RDB-libs/Src/String/StrUtils.h>


#include <RDB-libs/Src/Logging/Logging.h>

#include "AppConfigMgr.h"

#include "AppGlobalsCls.h"

#include "AppUtils.h"


#ifdef ALWAYS_DISPLAY_STATUS_OUTPUT
#define LogParsingStatusMsg     LogInfo
#else
#define LogParsingStatusMsg     LogDebug
#endif

#ifndef DefaultNetOutputPath
#define DefaultNetOutputPath    ("127.0.0.1")
#endif

#ifndef DefaultNetOutputPort
#define DefaultNetOutputPort    ("39900")
#endif

#ifndef DefaultAudioFile
#define DefaultAudioFile        ("media-output.raw")
#endif


/// \brief (extern) Declaration for app 'globals' class
extern AppGlobalsCls    g_globals;



// CAppConfigMgr function defs 

///
/// \brief CAppConfigMgr constructor
///
CAppConfigMgr::CAppConfigMgr() :
    //m_frameRate(0),
    //m_blockSize(0),
    m_bUsePumpThreadInterval(false),
    m_nLogLevel(LogLevel_useDefault)
{
    for (int c = 0; c < MAX_APP_CONFIG_FILES; c++)
        m_sConfigFile[c].clear();

    m_sConfigDir.clear();

    m_sNetPath.clear();
    m_sNetPort.clear();

    m_sTestType.clear();

    m_sAudioFile.clear();
};


///
/// \brief CAppConfigMgr destructor
///
CAppConfigMgr::~CAppConfigMgr()
{

};

/// \brief CAppConfigMgr::getConfigFilePath
/// Get a full path to the config file
/// \param nCfgFileIdx Index of selected config file
/// \return 
std::string CAppConfigMgr::getConfigFilePath(const unsigned int nCfgFileIdx)
{
    if (nCfgFileIdx >= MAX_APP_CONFIG_FILES)
    {
        LogError("invalid config file index");
        return "";
    }

#ifdef WINDOWS

    char cFilePathSeperator = '\\';

#else

    char cFilePathSeperator = '/';

#endif

    // set config file dir...

    std::string sConfigFilePath = m_sConfigDir;

    if (sConfigFilePath.length() < 1)
    {
        sConfigFilePath = ".";
    }

    int lastChar = (int) (sConfigFilePath.length() - 1);

    if (sConfigFilePath[lastChar] != cFilePathSeperator)
    {
        sConfigFilePath.push_back(cFilePathSeperator);
    }

    // set full config file path...

    sConfigFilePath.append(m_sConfigFile[nCfgFileIdx]);

    std::filesystem::path filePath = m_sConfigFile[nCfgFileIdx];

    if (filePath.extension().string() == "")
    {
        sConfigFilePath.append(".cfg");
    }

    return sConfigFilePath;
}


bool CAppConfigMgr::processCommandLine(std::vector<std::string>& argsList)
{
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

        parser.option("configDir d", ".");
        parser.option("configFile c", (m_sAppName + ".cfg"));

        parser.option("netPath n", "");
        parser.option("netPort p", "");

        parser.option("audioFile a", "");

        parser.parse(argsList);

        if (parser.found("help") == true)
        {
#ifdef DEBUG
            parser.print();
#endif
            return false;
        }

        m_sLogFile = parser.value("logFile");

        m_sLogDir = parser.value("logDir");

        if (parser.found("logLevel") == true)
        {
            m_nLogLevel = stringToInt(parser.value("logLevel"));
        }
        else
        {
            m_nLogLevel = LogLevel_useDefault;
        }

        m_sConfigDir = parser.value("configDir");
        m_sConfigFile[0] = parser.value("configFile");

        m_sNetPath = parser.value("netPath");

        if (m_sNetPath == "")
            m_sNetPath = DefaultNetOutputPath;

        m_sNetPort = parser.value("netPort");

        if (m_sNetPort == "")
            m_sNetPort = DefaultNetOutputPort;

        if (m_sAudioFile == "")
            m_sAudioFile = DefaultAudioFile;
    }
    catch (const std::exception& e)
    {
        LogDebug("problem parsing command line args - {} ", e.what());
        return false;
    }
    catch (...)
    {
        LogDebug("unknown exceptio parsing command line args ");
        return false;
    }

    return true;
}



/// \brief CAppConfigMgr::parseCommandLine
/// Parse the application command line
/// \param argc Number of command line params
/// \param argv Pointer to array of command line params
/// \return true of false (success is parsing)
bool CAppConfigMgr::parseCommandLine(int argc, char *argv[])
{
    std::vector<std::string> argsList;

    LogDebug("Command line args: ");
    for (auto a = 0; a < argc; a++)
    {
        std::string sTmp = argv[a];
        argsList.push_back(sTmp);
        LogDebug("  {}", *argv[a]);
    }

    return processCommandLine(argsList);
};


/// \brief CAppConfigMgr::parseCommandLine
/// Parse the application command line
/// \param sCmdLineParams std::string of command line params
/// \return true of false (success is parsing)
bool CAppConfigMgr::parseCommandLine(std::string &sCmdLineParams)
{
    std::vector<std::string> argsList;

    LogDebug("Command line args: ");

    unsigned int nStart = 0;

    while (1)
    {
        std::string sTmp = sCmdLineParams.substr(nStart);

        auto nPos = sTmp.find(' ');

        if (nPos == std::string::npos)
        {
            argsList.push_back(sTmp);

            LogDebug("  {}", sTmp);

            break;
        }

        sTmp = sCmdLineParams.substr(nStart, nPos);

        argsList.push_back(sTmp);

        LogDebug("  {}", sTmp);

        nStart += (unsigned int) (nPos + 1);
    }

    return processCommandLine(argsList);
};


/// \brief CAppConfigMgr::parseConfigFile
/// Parse the application config file
/// \param nCfgFileIdx Index of the config file to parse
/// \return true or false (success)
bool CAppConfigMgr::parseConfigFile(const unsigned int nCfgFileIdx)
{
    if (nCfgFileIdx >= MAX_APP_CONFIG_FILES)
    {
        LogError("invalid config file index");
        return false;
    }

    if (m_sConfigFile[nCfgFileIdx].empty())
    {
        LogError("config file not set");
        return false;
    }

    const std::string sConfigFilePath = 
        getConfigFilePath(nCfgFileIdx);

    // parse the app config file

    libconfig::Config cfg;

    // Read the file. If there is an error, report it and exit.
    try
    {
        cfg.readFile(sConfigFilePath);
    }
    catch (const libconfig::ParseException& pex)
    {
        LogError("Parse error at {}:{} - {} ", pex.getFile(), pex.getLine(), pex.getError());
        return false;
    }
    catch (...)
    {
        LogError("I/O error while reading config file: {}", sConfigFilePath);
        return false;
    }

    bool bStatus;

    const libconfig::Setting *root = nullptr;

    try
    {
        root = &(cfg.getRoot());
    }
    catch (...)
    {
        LogError("I/O error while reading config file: {}", sConfigFilePath);
        return false;
    }

    bStatus = 
        parseGolbalSection(root);


    return true;
};


/// @brief CAppConfigMgr::parseGolbalSection
/// Prase app config file 'globalSettings' section
/// @param root Pointer to settings 'root'
/// @return true or false (success)
bool CAppConfigMgr::parseGolbalSection(const libconfig::Setting* root)
{
    if (root == nullptr)
    {
        LogError("invalid root ptr");
        return false;
    }

    bool retValue = true;

    LogParsingStatusMsg("parsing globalSettings section... ");

    bool bStatus = false;

    libconfig::Setting* globalSettings = nullptr;

    // Find all global settings entries
    try
    {
        try
        {
            globalSettings = &(root->lookup("globalSettings"));
        }
        catch (...)
        {
            globalSettings = nullptr;
        }

        if (globalSettings != nullptr)
        {
            std::string sTmp = "false";

            bStatus = globalSettings->lookupValue("usePumpThreadInterval", sTmp);

            if (bStatus == false)
            {
                LogParsingStatusMsg("global usePumpThreadInterval not set.  (default = false)");
                m_bUsePumpThreadInterval = false;
            }
            else
            {
                if (StrUtils::toLower(sTmp) == "true")
                    m_bUsePumpThreadInterval = true;
                else
                    m_bUsePumpThreadInterval = false;

                LogParsingStatusMsg("Set global usePumpThreadInterval={}", m_bUsePumpThreadInterval);
            }
        }
        else
        {
            LogParsingStatusMsg("no globalSettings section");
        }
    }
    catch (...)
    {
        LogWarning("problem occuired while parsing globalSettings section ");
        retValue = false;
    }

    libconfig::Setting* directories = nullptr;

    // Find all global settings directory enties
    try
    {
        try
        {
            directories = &(globalSettings->lookup("directories"));
        }
        catch (...)
        {
            directories = nullptr;
        }

        if (directories != nullptr)
        {
#if 0
            bStatus = directories->lookupValue("audioPlugins", m_sAudioPluginDir);

            if (bStatus == false)
            {
                LogParsingStatusMsg("Invalid global audioPlugins dir");
            }
            else
            {
                LogParsingStatusMsg("Set global audioPlugins dir={}", m_sAudioPluginDir);
            }

            bStatus = directories->lookupValue("cmdPlugins", m_sCmdPluginDir);

            if (bStatus == false)
            {
                LogParsingStatusMsg("Invalid global cmdPlugins dir");
            }
            else
            {
                LogParsingStatusMsg("Set global cmdPlugins dir={}", m_sCmdPluginDir);
            }
            bStatus = directories->lookupValue("initFiles", m_sIniDir);

            if (bStatus == false)
            {
                LogParsingStatusMsg("Invalid global initFiles dir");
            }
            else
            {
                LogParsingStatusMsg("Set global initFiles dir={}", m_sIniDir);
            }
#endif
        }
        else
        {
            LogParsingStatusMsg("no globalSettings : directories section");
        }
    }
    catch (...)
    {
        LogWarning("problem occuired while parsing globalSettings : directories section ");
        retValue = false;
    }


    return retValue;
}

