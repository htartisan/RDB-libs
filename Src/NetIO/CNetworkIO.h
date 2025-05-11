//****************************************************************************
// FILE:    CNetworkIO.h
//
// DESC:    C++ Network (TCP) input/output class 
//
// AUTHOR:  Russ Barker
//


#ifndef C_NETWORK_IO_H
#define C_NETWORK_IO_H

#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>

#include <asio.hpp>

#include "../Thread/ThreadBase.h"


#define DEFAULT_TCP_BUFFER_SIZE             4096

#define NET_STREAM_TYPE_LEN                 10

#define ASYNC_ASIO_ACCEPTOR


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
//* CTcpClient class defs
//*


// CTcpClient class

class CTcpClient
{
  protected:

    std::string                                     m_sURI;
    std::string                                     m_sPort;

    asio::io_context                                m_ioContext;

    asio::ip::tcp::socket                           m_netSocket;

    asio::ip::tcp::resolver                         *m_pNetResolver;

    asio::ip::tcp::resolver::results_type           m_netEndPoints;

    bool                                            m_bConnected;

    NetworkDataHeaderInfo_def                       *m_pBufferHeader;

    unsigned int                                    m_nHeaderSize;

    DataBytePtr_def                                 m_pDataBuffer;

    unsigned int                                    m_nBufferSize;

    unsigned int                                    m_nCurrDataLen;

    std::string                                     m_sLastError;

  public:

    CTcpClient();

    ~CTcpClient();

    void setDataType(const std::string &sType);

    std::string getDataType();

    bool isDataType(const std::string &sType);

    unsigned int getHeaderSize();
\
    bool allocBuffer(const unsigned nSize = DEFAULT_TCP_BUFFER_SIZE, bool bAllocBufHeader = true);

    bool freeBuffer();

    bool clearBuffer();

    bool setBuffer(const void* pSource, const unsigned int nLen);

    bool appendBuffer(const void *pSource, const unsigned int nLen);

    unsigned int getCurrdataLen();

    bool getBuffer(void *pTarget, const unsigned int nLen, const unsigned int nStartingAt = 0);

    DataBytePtr_def getDataPtr();

    bool setDataSize(const unsigned int nLen);

    bool setUri(const std::string &sURI);

    bool setPort(const std::string& sPort);
    bool setPort(const unsigned int nPort);

    bool open();

    bool close();

    bool isConnected();

    int read(void *pTarget = nullptr, const unsigned int nLen = 0);

    int write(const void *pSource = nullptr, const unsigned int nLen = 0);

};


//*
//* CTcpServer class defs
//*


// CServerThread clss 

class CServerThread :
    public CThreadBase
{
    asio::io_context* m_pIoContext;

public:

    CServerThread(asio::io_context* pIoContext = nullptr) :
        m_pIoContext(pIoContext)
    {

    }

    ~CServerThread()
    {
        if (m_pIoContext == nullptr)
            return;

        if (m_pIoContext->stopped() == false)
        {
            m_pIoContext->stop();
        }
    }

    void setIoContext(asio::io_context* pIoContext)
    {
        m_pIoContext = pIoContext;
    }

    virtual void threadProc(void) override
    {
        if (m_pIoContext == nullptr)
            return;

        m_pIoContext->run();
    }
};


#define MsgHeaderLen_def        sizeof(NetworkDataHeaderInfo_def)


// CNetMessageData class

struct CNetMessageData
{
    char                    *m_pData;

    unsigned int            m_nHeaderLength;

    unsigned int            m_nMaxDataSize;

    unsigned int            m_nDataLength;

    bool                    m_bUpdated;

    CNetMessageData(unsigned int nHeaderLength);

    bool allocBuffer(unsigned int nBufSize);

    void clear();

    const char* getDataPtr() const;

    char* getDataPtr();

    std::size_t getDataMax() const;

    std::size_t getMsgLength() const;

    const char* getBodyPtr() const;

    char* getBodyPtr();

    std::size_t getBodyMax() const;

    std::size_t getBodyLength() const;

    void setBodyLength(const std::size_t nLen);

    int setDataType(const char* pType, const unsigned int nLen);

    void setUpdated(bool val);

    bool isUpdated();
};


// CNetMessageHandler class

struct CNetMessageHandler
{
    CNetMessageData &m_msgData;

    CNetMessageHandler(CNetMessageData& msgData);

    bool decodeMsgHeader(const std::string &sType);

    bool encodeMsgHeader(const std::string& sType);

};


class CTcpServer;


// CTcpSession class

class CTcpSession : 
    public std::enable_shared_from_this<CTcpSession>
{
    asio::ip::tcp::socket       &m_socket;

    std::string                 &m_sMsgType;

public:

    CTcpSession
        (
            asio::ip::tcp::socket &socket,
            std::string &sMsgType
        );

    ~CTcpSession();

    bool writeMsgData(CNetMessageData& msgData);

    bool readMsgData(CNetMessageData& msgData);

private:

    bool readMsgHeader(CNetMessageData& msgData);

    bool readMsgBody(CNetMessageData& msgData);

};


// CTcpServer class

class CTcpServer
{
    unsigned int                m_port;

    unsigned int                m_bufferSize;

    eNetIoDirection             m_eIoDirection;

    std::string                 m_sDataType;

    std::string                 m_sSrvrName;

    asio::ip::tcp::endpoint     *m_pEndpoint;

    asio::ip::tcp::acceptor     *m_pAcceptor;

    asio::io_context            m_ioContext;

    CServerThread               m_srvrThread;

    std::mutex                  m_inputMutex;
    std::mutex                  m_outputMutex;

    CNetMessageData             m_inputMsg;
    CNetMessageData             m_outputMsg;

    bool                        m_bRunning;

public:

    CTcpServer(eNetIoDirection eDir, const unsigned int nPort = 0, const unsigned int nSize = 0);

    ~CTcpServer();

    void setPort(const unsigned int nPort);

    void setBufferSize(const unsigned int nSize);

    void setDataType(const std::string &sType);

    void setName(const std::string &sName);

    bool initialize(const unsigned int nBufize);

    bool acceptConnection();

    bool findTcpSession(CTcpSession *pSession);

    bool deleteTcpSession(CTcpSession *pSession);

    asio::io_context& getIoContext()
    {
        return m_ioContext;
    }

    bool start();

    bool stop();

    bool isRunning();

    void setRunning(bool bVal);

    int readInputData(void* pBuff, const unsigned int nMax);

    int writeOutputData(const void* pBuff, const unsigned int nLen);

    bool sendOutput();

    // Virtual function for input msg processing.
    // Override this function for app msg handling.

    virtual bool processInputMsg(CNetMessageData &inputMsg, CNetMessageData &outputMsg);

};


// Utility functions

bool parseServerAndPort(const std::string &sUri, std::string &sServer, std::string sPort);


};  //  namespace CNetworkIO


#endif  //  C_NETWORK_IO_H

