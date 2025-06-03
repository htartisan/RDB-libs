///****************************************************************************
/// FILE:    CPluginFileMgr.h
///
/// DEESC:   Plugin (lib) file inst manager class def
///
/// AUTHOR:  Russ Barker
///

#ifndef PLUGIN_FILE_MGR_H
#define PLUGIN_FILE_MGR_H


#include "../Error/CError.h"

#if __cplusplus < 201703L
COMPILE_ERROR("ERRORL: C++17 not supported")
#endif

#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include <fstream> 
#include <iostream> 


#ifndef FileList_def
#define FileList_def    std::vector<std::string>
#endif


class CPluginFileMgr
{
  protected:

    std::string     m_sDirPath;
    std::string     m_sPluginFileExt;

    FileList_def    m_pluginFileList;

  public:

      CPluginFileMgr(const std::string &sDir = "", const std::string &sExt = "") :
        m_sDirPath(sDir),
        m_sPluginFileExt(sExt)
    {
        m_pluginFileList.clear();
    }

    ~CPluginFileMgr()
    {
        clear();
    }

    void clear()
    {
        m_sDirPath.clear();
        m_sPluginFileExt.clear();
        m_pluginFileList.clear();
    }

    bool setPluginDir(std::string &sDir)
    {
        if (sDir.empty())
            return false;

        m_sDirPath = sDir;

        return true;
    }

    int findPluginFiles(const std::string &sExt = "")
    {
        if (sExt != "")
            m_sPluginFileExt = sExt;

        if (m_sDirPath.empty() || m_sPluginFileExt.empty())
            return -1;

        std::filesystem::path dirPath = m_sDirPath;

        if (std::filesystem::exists(dirPath) == false)
        {
            std::string sCWD = (std::filesystem::current_path().string() + "\\");

            auto sTmp = (sCWD + m_sDirPath);

            LogError("findPluginFiles called with invalid path {}", sTmp);

            return -2;
        }

        for (const auto & entry : std::filesystem::directory_iterator(dirPath)) 
        {
            if (std::filesystem::is_directory(entry) == false) 
            {
                std::string sCurrFilePath = entry.path().string();

                std::string sCurrFileExt = entry.path().extension().string();

                if (sCurrFileExt == ("." + m_sPluginFileExt))
                {
                    m_pluginFileList.push_back(sCurrFilePath);
                }
            }
        }

        return (int) m_pluginFileList.size();
    }

    int getPluginCount()
    {
        return (int) m_pluginFileList.size();
    }

    std::string getPluginFile(const unsigned int nFileNum)
    {
        if (nFileNum >= (int) m_pluginFileList.size())
            return "";

        return m_pluginFileList[nFileNum];
    }

};


#endif  //  PLUGIN_FILE_MGR_H
