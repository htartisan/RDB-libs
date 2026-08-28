/// 
/// \file       AppUtils.h
/// 
///             App cutility function header
///

#ifndef APP_UTILS_H
#define APP_UTILS_H


#include <string>
#include <vector>

#include <libconfig/src/libconfig.h++>


bool safeConfigFileLoad(libconfig::Config *pCfg, const std::string &sFile);

bool safeConfigFileRoot(libconfig::Config *pCfg, libconfig::Setting **pRoot);

bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, std::string &sParam);
bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, bool &bParam);
bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, int &nParam);
bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, unsigned int &nParam);
bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, long long &nParam);
bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, unsigned long long &nParam);
bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, float &nParam);
bool safeConfigLookup(libconfig::Setting *pSetting, const char *pName, double &nParam);

bool isNumeric(const std::string& str);

int32_t stringToInt(const std::string &sVal);
uint32_t stringToUint(const std::string &sVal);
int64_t stringToLong(const std::string &sVal);
uint64_t stringToUlong(const std::string &sVal);
float stringToFloat(const std::string &sVal);

bool parsePluginConnection(const std::string &sPath, std::string& sPluginType, std::string& sInterface);


#endif  //  APP_UTILS_H
