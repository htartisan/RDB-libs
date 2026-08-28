/// 
/// \file       AppConfigMgr.h
/// 
///             CAppConfigMgr class header file
///
///             Loads/manages the global app configuration file entries.
///


#ifndef APP_CONFIG_MGR_H
#define APP_CONFIG_MGR_H


#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <string>
#include <vector>
#include <memory>
#include <variant>

#include <libconfig/src/libconfig.h++>

#include <argspp-lib/src/args.h>


#include <ME-Common/Src/CmdPlugin/CmdPluginDefs.h>


#define MAX_APP_CONFIG_FILES        5



// App configuration mgr

class CAppConfigMgr
{
    std::string                         m_sAppName;
    std::string                         m_sAppUsage;

public:

    std::string                         m_sConfigDir;
    std::string                         m_sConfigFile[MAX_APP_CONFIG_FILES];

    std::string                         m_sLogFile;
    std::string                         m_sLogDir;

    int                                 m_nLogLevel;

    std::string                         m_sNetPath;
    std::string                         m_sNetPort;

    std::string                         m_sTestType;

    std::string                         m_sAudioFile;

    bool                                m_bUsePumpThreadInterval;


    // CAppConfigMgr constructor

    CAppConfigMgr();

    ~CAppConfigMgr();

    void setAppName(const std::string& sName)
    {
        m_sAppName = sName;
    }

    std::string getConfigFilePath(const unsigned int nCfgFileIdx = 0);

    bool processCommandLine(std::vector<std::string> &argsList);

    bool parseCommandLine(int argc, char *argv[]);
    bool parseCommandLine(std::string &sCmdLineParams);

    bool parseConfigFile(const unsigned int nCfgFileIdx = 0);


protected:

    // support functions

    bool parseGolbalSection(const libconfig::Setting* root);

};



#endif  //  APP_CONFIG_MGR_H

