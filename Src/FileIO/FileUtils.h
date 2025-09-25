/// 
/// \file       FileUtils.h
/// 
///             File utility function definitions
///


#ifndef _UTILITY_FUNCTION_DEFS_
#define _UTILITY_FUNCTION_DEFS_


#include <filesystem>

#include <string>



//std::string removeLeadingSpaces(std::string &sStr);

//std::string removeTrailingSpaces(std::string& sStr);

//std::string getFileDir(std::string& sPath);

//std::string getFileExt(std::string &sPath);

//std::string getFileName(std::string& sPath);

//int getNumericStringAt(const std::string& sText, const unsigned int pos);



/// Utility functions

inline std::string removeLeadingSpaces(std::string& sStr)
{
    std::string sOut = "";

    bool bFirstCharFound = false;

    // skip leading spaces
    for (auto x = sStr.begin(); x != sStr.end(); x++)
    {
        if (bFirstCharFound == false && (*x) == ' ')
        {
            continue;
        }

        bFirstCharFound = true;

        sOut.push_back((*x));
    }

    return sOut;
}


inline std::string removeTrailingSpaces(std::string& sStr)
{
    std::string sOut = "";

    int nLen = (int)sStr.size();

    // find string len - num trailing spaces
    for (auto x = (sStr.end() - 1); x != sStr.begin(); x--)
    {
        if ((*x) != ' ')
        {
            break;
        }

        nLen--;
    }

    sOut.append(sStr.substr(0, nLen));

    return sOut;

}


inline std::string getFileDir(std::string& sPath)
{
    std::filesystem::path filePath(sPath);

    return (filePath.parent_path().string());
}


inline std::string getFileExt(std::string& sPath)
{
    std::filesystem::path filePath(sPath);

    return (filePath.extension().string());
}


inline std::string getFileName(std::string& sPath)
{
    std::filesystem::path filePath(sPath);

    return (filePath.stem().string());
}


inline int getNumericStringAt(const std::string& sText, const unsigned int pos)
{
    auto len = sText.length();

    if (pos >= len)
        return -1;

    std::string sTemp = sText.substr(pos);

    sTemp = removeLeadingSpaces(sTemp);

    sTemp = removeTrailingSpaces(sTemp);

    if (!StrUtils::isNumeric(sTemp))
        return -1;

    return std::stoi(sTemp);
}


#endif

