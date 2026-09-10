/// 
/// \file       CPlaylistMgr.h
/// 
///             CPlaylistMgr class header file
///


#ifndef CPLAYLISTMGR_H
#define CPLAYLISTMGR_H

#include "CFolderPathMgr.h"


enum ePlaylistType
{
    ePlaylistType_unknown = 0,
    ePlaylistType_m3u,
    ePlaylistType_m3u8,
};


class CPlaylistMgr
{
    std::string     m_sFilePath;

    std::string     m_sBaseFolderPath;

    ePlaylistType   m_eType;

    bool isDirectory(const std::string& sPath)
    {
        std::filesystem::path testPath = sPath;

        if (std::filesystem::is_directory(testPath)) 
        {
            return true;
        }

        return false;        
    }

    PathList_def parseM3U(const std::string& sPath) 
    {
        PathList_def fileList;

        std::ifstream file(sPath);
    
        if (file.is_open() == false) 
        {
            return fileList;
        }
        
        std::string line;

        while (std::getline(file, line)) 
        {
            // Trim leading/trailing carriage returns or whitespace if necessary
            if (!line.empty() && line.back() == '\r') 
            {
                line.pop_back();
            }
            
            // Skip empty lines and metadata/comment lines starting with '#'
            if (line.empty() == false && line[0] != '#') 
            {
                PathInfoEntry_def pathEntry;
                
                pathEntry.m_sPath = line;
                pathEntry.m_ePathEntryType = ePathEntryType_file;

                fileList.push_back(pathEntry);
            }
        }
        
        file.close();

        return fileList;
    }

    ePlaylistType getPlaylistType(const std::string& sPath)
    {
        if (sPath.empty())
        {
            return ePlaylistType_unknown;
        }

        std::filesystem::path filePath = sPath;

        std::filesystem::path ext = filePath.extension();

        // 2. Convert to string to use or print
        std::string extStr = ext.string();

        if (extStr == ".m3u")
            return ePlaylistType_m3u;

        if (extStr == ".m3u8")
            return ePlaylistType_m3u8;

        return ePlaylistType_unknown;
    }

public:

    CPlaylistMgr()
    {
        m_sFilePath = "";
        m_sBaseFolderPath = "";
        m_eType = ePlaylistType_unknown;
    }

    CPlaylistMgr(const std::string &sFile, const std::string &sFolder = "") :
        m_sFilePath(sFile),
        m_sBaseFolderPath(sFolder)
    {
        m_eType = ePlaylistType_unknown;
    }

    ~CPlaylistMgr()
    {}

    bool isSupportedFileType(const std::string& sPath = "")
    {
        ePlaylistType type = ePlaylistType_unknown;

        if (sPath.empty())
        {
            if (m_sFilePath.empty())
            {
                return false;
            }

            type = getPlaylistType(m_sFilePath);
        }
        else
        {
            type = getPlaylistType(sPath);
        }
           
        if (type == ePlaylistType_m3u)
            return true;
        
        return false;
    }

    bool setFilePath(const std::string& sPath)
    {
        if (sPath.empty())
            return false;

        if (std::filesystem::exists(sPath) == false)
            return false;

        if (isSupportedFileType(sPath) == false)
            return false;
    
        m_sFilePath = sPath;

        return true;   
    }

    bool setBaseFolder(const std::string& sPath)
    {
        if (sPath.empty())
            return false;

        if (std::filesystem::exists(sPath) == false)
            return false;

        if (isSupportedFileType(sPath) == false)
            return false;

        if (isDirectory(sPath) == false)
    
        m_sBaseFolderPath = sPath;
            return false;

        return true;   
    }

    PathList_def getPathList()
    {
        PathList_def pathList;

        if (m_sFilePath.empty())
            return pathList;
        
        switch (m_eType)
        {
        case ePlaylistType_m3u:
            pathList = parseM3U(m_sFilePath);
            break;

        default:
            break;
        }

        return pathList;
    }
    
};


#endif  //  CPLAYLISTMGR_H
