/// 
/// \file   Logging.h
/// 
///         Logging header file
///


#ifndef LOG_MESSAGE_LIST_H
#define LOG_MESSAGE_LIST_H

#include <string>
#include <vector>
#include <map>


typedef std::map<std::string, unsigned int>     LogMessageModuleList_def;


class CLogMessageModuleListMgr
{
public:

    LogMessageModuleList_def    m_moduleList;

    CLogMessageModuleListMgr()
    {
        m_moduleList.clear();
    }

    int lookup(std::string sModule)
    {
        auto x = m_moduleList.find(sModule);

        if (x != m_moduleList.end())
        {
            return (*x).second;
        }

        unsigned int nNewIdx = (unsigned int) m_moduleList.size();

        m_moduleList[sModule] = nNewIdx;

        return nNewIdx;
    }
};


struct LogMessageEntry_def
{
    unsigned int                m_nLogLevel;

    unsigned int                m_nModuleID;

    uint64_t                    m_timestamp;

    std::string                 m_message;

    LogMessageEntry_def(const unsigned int nLevel, const unsigned int nModule, const uint64_t nTimestamp, const std::string &sMsg) :
        m_nLogLevel(nLevel),
        m_nModuleID(nModule),
        m_timestamp(nTimestamp),
        m_message(sMsg)
    {

    }
};


typedef std::vector<LogMessageEntry_def>    LogMessageList_def;

class CLogMessageList
{

    unsigned int                m_maxEntries;

public:

    unsigned int                m_maxEntries;

    CLogMessageModuleListMgr    m_moduleList;

    LogMessageList_def          m_messageList;

    CLogMessageList(unsigned int nMaxEntries = 10000) :
        m_maxEntries(nMaxEntries)
    {
        m_messageList.clear();
    }

    void append(const unsigned int nLogLevel, const uint64_t nTimestamp, const std::string &sModule, const std::string sMsg)
    {
        // if m_messageList >= max entries
        while (m_messageList.size() >= m_maxEntries)
        {
            // delete oldest entry

            auto x = m_messageList.begin();

            m_messageList.erase(x);
        }

        auto nModule = m_moduleList.lookup(sModule);

        m_messageList.push_back(LogMessageEntry_def(nLogLevel, nModule, nTimestamp, sMsg));
    }

};



#endif  //  LOG_MESSAGE_LIST_H
