//****************************************************************************
// FILE:    CServerIO.h
//
// DESC:    C++ Network server input/output class 
//
// AUTHOR:  Russ Barker
//


#define _CRT_SECURE_NO_WARNINGS


#ifndef NET_SERVER_IO_H
#define NET_SERVER_IO_H


#if defined(WINDOWS)
#include <SDKDDKVer.h>
#endif

#include <asio.hpp>

#if defined(WINDOWS)
#include <windows.h>
#endif

#include "CNetworkIO.h"


namespace CNetworkIO
{ 


//*
//* CUdpServer class defs
//*


class CUdpServer;


typedef void (*ProcessMsgProc_def)(CUdpServer *, CNetMessageData &, CNetMessageData &);


class CUdpProcessingContext
{
    asio::io_context        &m_ioContext;

    CNetMessageData         &m_inputMsg;
    CNetMessageData         &m_outputMsg;

    CUdpServer              *m_pUdpServer;

    ProcessMsgProc_def      m_pProcessMsgProc;

    bool                    m_bExit;

public:

    CUdpProcessingContext
        (
            CUdpServer       *pSrvr,
            asio::io_context &ioContext,
            CNetMessageData  &inputMsg,
            CNetMessageData  &outputMsg
        ) :
        m_ioContext(ioContext),
        m_inputMsg(inputMsg),
        m_outputMsg(outputMsg),
        m_pUdpServer(pSrvr),
        m_bExit(false)
    {

    }

    ~CUdpProcessingContext()
    {
        m_bExit = true;
    }

    bool setProcessMsgProc(ProcessMsgProc_def pProc)
    {
        if (pProc == nullptr)
        {
            return false;
        }

        m_pProcessMsgProc = pProc;

        return true;
    }

    void run();

    bool stopped()
    {
        return m_bExit;
    }

    void stop()
    {
        m_bExit = true;
    }
};


// CUdpSession class

class CUdpSession :
    public std::enable_shared_from_this<CUdpSession>
{
    asio::ip::udp::socket       &m_socket;

    asio::ip::udp::endpoint     &m_endpoint;

    std::string                 &m_sMsgType;

    std::string                 m_sLastError;

    std::mutex                  m_mutex;

public:

    CUdpSession
    (
        asio::ip::udp::socket &socket,
        asio::ip::udp::endpoint &endpoint,
        std::string &sMsgType
    );

    ~CUdpSession();

    asio::ip::udp::socket* getSocket()
    {
        return &m_socket;
    }

    bool writeMsgData(CNetMessageData& msgData);

    bool readMsgData(CNetMessageData& msgData);

private:

    bool readMsgHeader(CNetMessageData& msgData);

    bool readMsgBody(CNetMessageData& msgData);

};


// CUdpServer class

class CUdpServer
{
    unsigned int                            m_port;

    unsigned int                            m_bufferSize;

    eNetIoDirection                         m_eIoDirection;

    std::string                             m_sDataType;

    std::string                             m_sSrvrName;

    asio::io_context                        m_ioContext;

    asio::ip::udp::endpoint                 *m_pEndpoint;

    asio::ip::udp::socket                   *m_pSocket;

    CUdpSession                             *m_pSession;

    CUdpProcessingContext                   *m_pProcessingContext;

    CServerThread<CUdpProcessingContext>   m_srvrThread;

    std::mutex                              m_inputMutex;
    std::mutex                              m_outputMutex;

    CNetMessageData                         m_inputMsg;
    CNetMessageData                         m_outputMsg;

    bool                                    m_bInitialized;
    bool                                    m_bRunning;

public:

    CUdpServer(eNetIoDirection eDir, const unsigned int nPort = 0, const unsigned int nSize = 0);

    ~CUdpServer();

    bool setPort(const unsigned int nPort);

    bool setBufferSize(const unsigned int nSize);

    bool setDataType(const std::string &sType);

    bool setName(const std::string &sName);

    bool initialize(const unsigned int nBufize = 0);

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

    bool processInputMsg(CNetMessageData& inputMsg, CNetMessageData& outputMsg);

    // Virtual function for input msg processing.
    // Override this function for app msg handling.

    static void ioMsgHandler(CUdpServer *pSrvr, CNetMessageData &inputMsg, CNetMessageData &outputMsg);

};


class CTcpServer;


// CTcpSession class

class CTcpSession : 
    public std::enable_shared_from_this<CTcpSession>
{
    asio::ip::tcp::socket       &m_socket;

    std::string                 &m_sMsgType;

    std::string                 m_sLastError;

    std::mutex                  m_mutex;

public:

    CTcpSession
        (
            asio::ip::tcp::socket &socket,
            std::string &sMsgType
        );

    ~CTcpSession();

    bool writeMsgData(CNetMessageData& msgData);

    bool sendMsgData(CNetMessageData& msgData);

    bool readMsgData(CNetMessageData& msgData);

    std::string getLastError()
    {
        return m_sLastError;
    }

private:

    bool readMsgHeader(CNetMessageData& msgData);

    bool readMsgBody(CNetMessageData& msgData);

};


// CTcpServer class

class CTcpServer
{
    unsigned int                            m_port;

    unsigned int                            m_bufferSize;

    eNetIoDirection                         m_eIoDirection;

    std::string                             m_sDataType;

    std::string                             m_sSrvrName;

    asio::ip::tcp::endpoint                 *m_pEndpoint;

    asio::ip::tcp::acceptor                 *m_pAcceptor;

    asio::io_context                        m_ioContext;

    CServerThread<asio::io_context>         m_srvrThread;

    std::mutex                              m_inputMutex;
    std::mutex                              m_outputMutex;

    CNetMessageData                         m_inputMsg;
    CNetMessageData                         m_outputMsg;
    CNetMessageData                         m_ctrlMsg;

    unsigned long                           m_heartBeatInterval;

    std::chrono::system_clock::time_point   m_heartBeatTimestamp;

    bool                                    m_bInitialized;
    bool                                    m_bRunning;
    bool                                    m_bTcpNoDelay;
    bool                                    m_bMultiMsgSession;
    bool                                    m_bExitSession;

    unsigned int                            m_activeSessions;

public:

    CTcpServer(eNetIoDirection eDir, const unsigned int nPort = 0, const unsigned int nSize = 0);

    ~CTcpServer();

    bool setPort(const unsigned int nPort);

    bool setBufferSize(const unsigned int nSize);

    bool setDataType(const std::string &sType);

    bool setName(const std::string &sName);

    void setMultiIoSession(bool val)
    {
        m_bMultiMsgSession = val;

        if (val == true)
        {
            m_bTcpNoDelay = true;
        }
    }

    void setTcpNoDelay(bool val)
    {
        m_bTcpNoDelay = val;
    }

    void setHeartBeatInterval(unsigned long nInt)
    {
        m_heartBeatInterval = nInt;
    }

    bool initialize(const unsigned int nBufize = 0);

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

    std::string getRemoteAddress(asio::ip::tcp::socket &socket)
    {
        auto endPoint = socket.remote_endpoint();

        auto remoteAddr = endPoint.address();

        return remoteAddr.to_string();
    }

    int readInputData(void* pBuff, const unsigned int nMax);

    int writeOutputData(const void* pBuff, const unsigned int nLen);

    bool sendOutput();

    unsigned int numActiveSessions()
    {
        return m_activeSessions;
    }

    // Virtual function for input msg processing.
    // Override this function for app msg handling.

    bool processInputMsg(CNetMessageData& inputMsg, CNetMessageData& outputMsg);

};


};  //  namespace CNetworkIO


#endif  //  C_NETWORK_IO_H

