/// 
/// \file       AppUtils.cpp
/// 
///             App utility function definitions
///


#define _CRT_SECURE_NO_WARNINGS


//#include <iostream>
//#include <iomanip>
#include <cstdlib>

#include <RDB-libs/Src/String/StrUtils.h>

#include <RDB-libs/Src/Logging/Logging.h>

#include "AppUtils.h"


//
// Utility functiuons
//

// safe function for loading libconfig file

bool safeConfigFileLoad(libconfig::Config *pCfg, const std::string &sFile)
{
    if (pCfg == nullptr || sFile == "")
        return false;

    // Read the file. If there is an error, report it and exit.
    try
    {
        pCfg->readFile(sFile);
    }
    catch (const libconfig::ParseException& pex)
    {
        LogError("Config file parse error at {}:{} - {} ", pex.getFile(), pex.getLine(), pex.getError());
        return false;
    }
    catch (...)
    {
        LogError("Error while reading config file: {}", sFile);
        return false;
    }

    return true;
}


// safe function for getting libconfig file root pointer

bool safeConfigFileRoot(libconfig::Config *pCfg, libconfig::Setting **pSetting)
{
    if (pCfg == nullptr || pSetting == nullptr)
        return false;

    *pSetting = nullptr;

    libconfig::Setting *pRoot = nullptr;

    try
    {
        pRoot = &(pCfg->getRoot());
    }
    catch (...)
    {
        LogError("Error while getting config file root");
        return false;
    }

    if (pRoot == nullptr)
    {
        LogError("Invalid config file root ptr");
        return false;
    }    

    *pSetting = pRoot;

    return true;
}


// safe functions for looking up libconfig params

bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, std::string &sParam)
{
    if (pSetting == nullptr || pName == nullptr)
        return false;

    bool bRet = false;

    try
    {
        bRet = pSetting->lookupValue(pName, sParam);
    }
    catch (...)
    {
        return false;
    }

    return bRet;
} 

bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, bool &bParam)
{
    if (pSetting == nullptr || pName == nullptr)
        return false;

    bool bRet = false;

    try
    {
        bRet = pSetting->lookupValue(pName, bParam);
    }
    catch (...)
    {
        return false;
    }

    return bRet;
} 


bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, int &nParam)
{
    if (pSetting == nullptr || pName == nullptr)
        return false;

    bool bRet = false;

    try
    {
        bRet = pSetting->lookupValue(pName, nParam);
    }
    catch (...)
    {
        return false;
    }

    return bRet;
} 


bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, unsigned int &nParam)
{
    if (pSetting == nullptr || pName == nullptr)
        return false;

    bool bRet = false;

    try
    {
        bRet = pSetting->lookupValue(pName, nParam);
    }
    catch (...)
    {
        return false;
    }

    return bRet;
} 


bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, long long &nParam)
{
    if (pSetting == nullptr || pName == nullptr)
        return false;

    bool bRet = false;

    try
    {
        bRet = pSetting->lookupValue(pName, nParam);
    }
    catch (...)
    {
        return false;
    }

    return bRet;
} 


bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, unsigned long long &nParam)
{
    if (pSetting == nullptr || pName == nullptr)
        return false;

    bool bRet = false;

    try
    {
        bRet = pSetting->lookupValue(pName, nParam);
    }
    catch (...)
    {
        return false;
    }

    return bRet;
} 


bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, float &nParam)
{
    if (pSetting == nullptr || pName == nullptr)
        return false;

    bool bRet = false;

    try
    {
        bRet = pSetting->lookupValue(pName, nParam);
    }
    catch (...)
    {
        return false;
    }

    return bRet;
} 


bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, double &nParam)
{
    if (pSetting == nullptr || pName == nullptr)
        return false;

    bool bRet = false;

    try
    {
        bRet = pSetting->lookupValue(pName, nParam);
    }
    catch (...)
    {
        return false;
    }

    return bRet;
} 


/// \brief Tests if a std::string is numeric
/// \param str 
/// \return true or flass
bool isNumeric(const std::string& str)
{
    std::string::const_iterator it = str.begin();

    while (it != str.end() && std::isdigit(*it)) 
        ++it;

    return (!str.empty() && it == str.end());
};


/// \brief Converts a std::string value to int32
/// \param str 
/// \return int32 value
int32_t stringToInt(const std::string &sVal)
{
    if (sVal == "")
        return 0;

    int32_t nRet = 0;

    try
    {
        if (sVal.substr(0, 2) == "0x")
        {
            LogDebug("Converting HEX string ({}) to int32", sVal);
            nRet = std::stol(sVal.substr(2).c_str(), nullptr, 16);
        }
        else
        {
            nRet = std::stol(sVal);
        }
    }
    catch (const std::invalid_argument& ia) 
    {
        LogError("Invalid argument ({}) while converting ({}) to uint ", ia.what(), sVal);
        return 0;
    } 
    catch (const std::out_of_range& oor) 
    {
        LogError("Out of range ({}) while converting ({}) to uint ", oor.what(), sVal);
        return 0;
    }
    catch(...)
    {
        LogError("Unknown error while converting ({}) to uint", sVal);
        return 0;
    }                

    return nRet;
}


/// \brief Converts a std::string value to uint32
/// \param str 
/// \return uint32 value
uint32_t stringToUint(const std::string &sVal)
{
    if (sVal == "")
        return 0;

    uint32_t nRet = 0;

    try
    {
        if (sVal.substr(0, 2) == "0x")
        {
            LogDebug("Converting HEX string ({}) to uint32", sVal);
            nRet = std::stoul(sVal.substr(2).c_str(), nullptr, 16);
        }
        else
        {
            nRet = std::stoul(sVal);
        }
    }
    catch (const std::invalid_argument& ia) 
    {
        LogError("Invalid argument ({}) while converting ({}) to uint ", ia.what(), sVal);
        return 0;
    } 
    catch (const std::out_of_range& oor) 
    {
        LogError("Out of range ({}) while converting ({}) to uint ", oor.what(), sVal);
        return 0;
    }
    catch(...)
    {
        LogError("Unknown error while converting ({}) to uint", sVal);
        return 0;
    }                

    return nRet;
}


/// \brief Converts a std::string value to int64
/// \param str 
/// \return int64 value
int64_t stringToLong(const std::string &sVal)
{
    if (sVal == "")
        return 0;

    int64_t nRet = 0;

    try
    {
        if (sVal.substr(0, 2) == "0x")
        {
            LogDebug("Converting HEX string ({}) to int64", sVal);
            nRet = std::stoll(sVal.substr(2).c_str(), nullptr, 16);
        }
        else
        {
            nRet = std::stoll(sVal);
        }
    }
    catch (const std::invalid_argument& ia) 
    {
        LogError("Invalid argument ({}) while converting ({}) to uint ", ia.what(), sVal);
        return 0;
    } 
    catch (const std::out_of_range& oor) 
    {
        LogError("Out of range ({}) while converting ({}) to uint ", oor.what(), sVal);
        return 0;
    }
    catch(...)
    {
        LogError("Unknown error while converting ({}) to uint", sVal);
        return 0;
    }                

    return nRet;
}


/// \brief Converts a std::string value to uint64
/// \param str 
/// \return uint64 value
uint64_t stringToUlong(const std::string &sVal)
{
    if (sVal == "")
        return 0;

    uint64_t nRet = 0;

    try
    {
        if (sVal.substr(0, 2) == "0x")
        {
            LogDebug("Converting HEX string ({}) to uint64", sVal);
            nRet = std::stoull(sVal.substr(2).c_str(), nullptr, 16);
        }
        else
        {
            nRet = std::stoull(sVal);
        }
    }
    catch (const std::invalid_argument& ia) 
    {
        LogError("Invalid argument ({}) while converting ({}) to uint ", ia.what(), sVal);
        return 0;
    } 
    catch (const std::out_of_range& oor) 
    {
        LogError("Out of range ({}) while converting ({}) to uint ", oor.what(), sVal);
        return 0;
    }
    catch(...)
    {
        LogError("Unknown error while converting ({}) to uint", sVal);
        return 0;
    }                

    return nRet;
}


/// \brief Converts a std::string value to float
/// \param str 
/// \return uint32 value
float stringToFloat(const std::string &sVal)
{
    if (sVal == "")
        return 0;

    float nRet = 0;

    try
    {
        nRet = std::stof(sVal);
    }
    catch (const std::invalid_argument& ia) 
    {
        LogError("Invalid argument ({}) while converting ({}) to float ", ia.what(), sVal);
        return 0;
    } 
    catch (const std::out_of_range& oor) 
    {
        LogError("Out of range ({}) while converting ({}) to float ", oor.what(), sVal);
        return 0;
    }
    catch(...)
    {
        LogError("Unknown error while converting ({}) to float", sVal);
        return 0;
    }                

    return nRet;
}


/// \brief Parse a plugin 'connection' string
/// \param sPath Input std::string variable containing plugin 'connection' info 
/// \param sPluginType Returned std::string 'type' of the plugin
/// \param sInterface Returned std::string 'interface name' for the plugin connection
/// \return true or false (success in parsing the input string)
bool parsePluginConnection(const std::string &sPath, std::string& sPluginType, std::string& sInterface)
{
    if (sPath.empty())
    {
        return false;
    }

    std::string sType = "";
    std::string sXface = "";

    auto firstColon = sPath.find(':', 0);

    if (firstColon == std::string::npos)
    {
        return false;
    }

    sType = sPath.substr(0, firstColon);

    if (sType.empty())
    {
        return false;
    }

    sXface = sPath.substr((firstColon + 1));

    if (sXface.empty())
    {
        return false;
    }

    sPluginType = sType;
    sInterface = sXface;

    return true;
};

