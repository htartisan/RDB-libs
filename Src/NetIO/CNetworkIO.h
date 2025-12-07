//****************************************************************************
// FILE:    CNetworkIO.h
//
// DESC:    C++ Network input/output base classes 
//
// AUTHOR:  Russ Barker
//


#define _CRT_SECURE_NO_WARNINGS


#ifndef C_NETWORK_IO_H
#define C_NETWORK_IO_H


#include <string>
#include <vector>
#include <chrono>
#include <mutex>

#include "../Thread/ThreadBase.h"

#include "../Logging/Logging.h"


#ifdef DEBUG
#define LogDebugInfoMsg         LogInfo
#else
#define LogDebugInfoMsg         LogDebug
#endif


#define DEFAULT_TCP_BUFFER_SIZE             4096

#define NET_STREAM_TYPE_LEN                 10

#define TCP_NO_DELAY_DEFAULT                true

#ifdef TCP_MULTI_SESSION
#define TCP_SRVR_SESSION_LOOP_DEFAULT       true
#else
#define TCP_SRVR_SESSION_LOOP_DEFAULT       false
#endif


//#define ASYNC_ASIO_ACCEPTOR

//#define USE_ASIO_ASYNC_READ
//#define USE_ASIO_ASYNC_WRIRE


// Utility functions


bool parseServerAndPort(const std::string& sUri, std::string& sServer, std::string& sPort, std::string& sProtocol);


typedef struct UriParamData_tag
{
    std::string     m_sName;
    std::string     m_sValue;

    UriParamData_tag(const std::string &sName = "", const std::string& sValue = "")
    {
        m_sName = sName;
        m_sValue = sValue;
    }

} UriParamData_def;

typedef std::vector<UriParamData_def>      UriParamList_def;


bool parseUriParams(const std::string& sPrmsStr, UriParamList_def& prmList);


namespace CNetworkIO
{ 


enum eNetIoDirection
{
    eNetIoDirection_unknown = 0,
    eNetIoDirection_input,
    eNetIoDirection_output,
    eNetIoDirection_IO
};


#pragma pack(push, 1)

struct NetworkDataHeaderInfo_def
{
    uint16_t            m_headerMarker;

    char                m_StreamType[NET_STREAM_TYPE_LEN];

    uint32_t            m_nDataLen;

    eNetIoDirection     m_eIoDirection;

    void initialize(const char *pType = nullptr, const unsigned int nLen = 0)
    {
        m_headerMarker = 0xFFFF;

        memset(m_StreamType, 0, NET_STREAM_TYPE_LEN);

        if (pType != nullptr)
        {
            for (auto x = 0; x < NET_STREAM_TYPE_LEN; x++)
            {
                if (*(pType + x) == 0)
                    break;

                m_StreamType[x] = *(pType + x);
            }
        }

        m_nDataLen = nLen;
    }
};

#pragma pack(pop)


typedef int8_t                                      DataByte_def;

typedef DataByte_def *                              DataBytePtr_def;


//*
//* General Server class defs
//*


// CServerThread clss 

template <typename T>
class CServerThread :
    public CThreadBase
{
    T* m_pContext;

public:

    CServerThread(T* pContext = nullptr) :
        m_pContext(pContext)
    {

    }

    ~CServerThread()
    {
        if (m_pContext == nullptr)
            return;

        if (m_pContext->stopped() == false)
        {
            m_pContext->stop();
        }
    }

    void setContext(T* pContext)
    {
        m_pContext = pContext;
    }

    virtual void threadProc(void) override
    {
        if (m_pContext == nullptr)
            return;

        m_pContext->run();
    }
};


#define MsgHeaderLen_def        sizeof(NetworkDataHeaderInfo_def)


// CNetMessageData class

class CNetMessageData
{
    char                            *m_pData;

    unsigned int                    m_nHeaderLength;

    unsigned int                    m_nMaxDataSize;

    unsigned int                    m_nDataLength;

    bool                            m_bUpdated;

    std::mutex                      m_dataLock;

public:

    CNetMessageData(unsigned int nHeaderLength);

    bool allocBuffer(unsigned int nMaxDataSize);

    bool isBufferAllocated();

    int getMaxDataLen();

    int getCurDataLen();

    void clearAll();

    void clearMsgBody();

    char* getDataPtr();

    char* getBodyPtr();

    void releasePtr();

    void setHeaderLength(const std::size_t nLen);

    int getHeaderLength();

    void setBodyLength(const std::size_t nLen);

    int getBodyLength();

    int setDataType(const char* pType, const unsigned int nLen);

    bool setMsgData(const void* pData, const unsigned int nLen);

    bool compareMsgData(const char* pData, const unsigned int nLen);

    void setUpdated(bool val);

    bool isUpdated();

    bool decodeMsgHeader(const std::string& sType);

    bool encodeMsgHeader(const std::string& sType);

};



};  //  namespace CNetworkIO


#endif  //  C_NETWORK_IO_H

