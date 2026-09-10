/// 
/// \file       CFolderPathMgr.h
/// 
///             CFolderPathMgr class header file
///


#ifndef CFOLDERPATHMGR_H
#define CFOLDERPATHMGR_H

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>


#ifndef PATHLIST_DEF
#define PATHLIST_DEF

enum ePathEntryType
{
    ePathEntryType_unknown = 0,
    ePathEntryType_file,
    ePathEntryType_dir,
    
};

struct PathInfoEntry_def
{
    std::string    m_sPath;

    ePathEntryType m_ePathEntryType;

    PathInfoEntry_def()
    {
        m_sPath = "";
        m_ePathEntryType = ePathEntryType_unknown;
    }
};

typedef std::vector<PathInfoEntry_def>    PathList_def;

#endif


#ifndef PATHSEARCHTYPE_DEF
#define PATHSEARCHTYPE_DEF

enum ePathSearchType
{
    ePathSearchType_unknown = 0,
    ePathSearchType_file,
    ePathSearchType_dir,
    ePathSearchType_ext,
    ePathSearchType_name,
    ePathSearchType_all,
};

#endif


class CFolderPathMgr
{
private:

    std::string    m_sPath;

    enum eProcessedEntryType
    {
        eProcessedEntryType_none = 0,
        eProcessedEntryType_dir,
        eProcessedEntryType_file,
    };

    eProcessedEntryType processPathEntry
        (
            const std::filesystem::directory_entry &entry, 
            ePathSearchType eSearchType,
            PathList_def &pathList,
            const std::string sSearchString = ""
        )
    {
        PathInfoEntry_def    pathInfoEntry;

        auto sPath = entry.path().string();

        if (entry.is_directory() == true)
        {
            if (eSearchType == ePathSearchType_dir || eSearchType == ePathSearchType_all)
            {
                pathInfoEntry.m_sPath = sPath;
                pathInfoEntry.m_ePathEntryType = ePathEntryType_dir;

                pathList.push_back(pathInfoEntry);

                return eProcessedEntryType_dir;
            }
        }

        if (entry.is_regular_file() == true)
        {
            if (eSearchType == ePathSearchType_file || eSearchType == ePathSearchType_all)
            {
                pathInfoEntry.m_sPath = sPath;
                pathInfoEntry.m_ePathEntryType = ePathEntryType_file;

                pathList.push_back(pathInfoEntry);

                return eProcessedEntryType_file;
            }

            if (eSearchType == ePathSearchType_ext)
            {
                if (entry.path().extension() == sSearchString)
                {
                    pathInfoEntry.m_sPath = sPath;
                    pathInfoEntry.m_ePathEntryType = ePathEntryType_file;

                    pathList.push_back(pathInfoEntry);

                    return eProcessedEntryType_file;
                }

                return eProcessedEntryType_none;
            }

            if (eSearchType == ePathSearchType_name)
            {
                if (sSearchString == "")
                {
                    return eProcessedEntryType_none;
                }

                if (entry.path().filename() == sSearchString)
                {
                    pathInfoEntry.m_sPath = sPath;
                    pathInfoEntry.m_ePathEntryType = ePathEntryType_file;

                    pathList.push_back(pathInfoEntry);

                    return eProcessedEntryType_file;
                }

                return eProcessedEntryType_none;
            }
        }

        return eProcessedEntryType_none;
    }

public:
    CFolderPathMgr()
    {
        m_sPath = "";
    }

    CFolderPathMgr(const std::string &sPath) :
        m_sPath(sPath)
    {

    }

    ~CFolderPathMgr()
    {}

    bool setPath(const std::string &sPath)
    {
        if (sPath.empty() == true)
        {
            return false;
        }
        
        if (std::filesystem::exists(sPath) == false)
        {
            return false;
        }

        m_sPath = sPath;

        return true;
    }

    PathList_def
        getFolderList
        (
            ePathSearchType eSearchType,
            bool bRecursive,
            const std::string& sSearchString = ""
        )
    {
        PathList_def    pathList;

        if (std::filesystem::exists(m_sPath) == false)
        {
            return pathList;
        }

        if (bRecursive == true)
        {
            // Use recursive_directory_iterator to loop through everything
            for (const auto& entry : std::filesystem::recursive_directory_iterator(m_sPath))
            {
                auto retValue = processPathEntry(entry, eSearchType, pathList, sSearchString);
                if (retValue == eProcessedEntryType_none)
                {
                    break;   
                }
            }
        }
        else
        {
            for (const auto& entry : std::filesystem::directory_iterator(m_sPath))
            {
                auto retValue = processPathEntry(entry, eSearchType, pathList, sSearchString);
                if (retValue == eProcessedEntryType_none)
                {
                    break;   
                }
            }
        }

        return pathList;
    }

};



#endif // CFOLDERPATHMGR_H
