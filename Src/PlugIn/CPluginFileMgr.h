//****************************************************************************
// FILE:    CPluginFileMgr.h
//
// DEESC:   Plugin (lib) file manager class def
//
// AUTHOR:  Russ Barker
//

#ifndef PLUGIN_FILE_MGR_H
#define PLUGIN_FILE_MGR_H


#include "../Error/CError.h"

#if __cplusplus < 201703L
    COMPILE_ERROR("ERRORL: C++17 not supported")
#endif

#include "CPluginLoader.h"

#include <filesystem>


#ifndef FileList_def
#define std::vector<std::string>     FileList_def;
#endif


class CPluginFileMgr
{

    std::string     m_sDirPath;
    std::string     m_sPluginFileExt;

    FileList_def    m_pluginFileList;

  public:

    CPluginFileMgr(std::string &sDir = "", std::string &sExt = "") :
        m_sDirPath(sDir),
        m_sPluginFileExt(sExt)
    {
        m_pluginFileList.clear();
    }

    bool setPluginDir(std::string &sDir)
    {
        if (sDir.empty())
            return false;

        m_sDirPath = sDir;

        return true;
    }

    int findPluginFiles(std::string &sExt = "")
    {
        if (sExt.empty() != false)
            m_sPluginFileExt = sExt;

        if (m_sDirPath.empty() || m_sPluginFileExt.empty())
            return -1;

        std::filesystem::path dirPath = m_sDirPath;

        for (const auto & entry : std::filesystem::directory_iterator(dirPath)) 
        {
            if (std::filesystem::is_directory(entry) == false) 
            {
                std::string sCurrFilePath = entry.path();

                std::string sCurrFileExt = entry.extension().string();

                if (sCurrFileExt == m_sPluginFileExt)
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


#endif  /  PLUGIN_FILE_MGR_H
