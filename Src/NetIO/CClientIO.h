//****************************************************************************
// FILE:    CClientIO.h
//
// DESC:    C++ Network client input/output class 
//
// AUTHOR:  Russ Barker
//


#define _CRT_SECURE_NO_WARNINGS


#ifndef NET_CLIENT_IO_H
#define NET_CLIENT_IO_H


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
//* CUdpClient class defs
//*

// CUdpClient class

class CUdpClient
{
  protected:

    std::string                                     m_sURI;
    std::string                                     m_sPort;

    asio::io_context                                m_ioContext;

    asio::ip::udp::socket                           m_netSocket;

    asio::ip::udp::resolver                         *m_pNetResolver;

    asio::ip::udp::endpoint                         m_netEndPoint;

    bool                                            m_bConnected;

    NetworkDataHeaderInfo_def                       *m_pBufferHeader;

    unsigned int                                    m_nHeaderSize;

    DataBytePtr_def                                 m_pDataBuffer;

    unsigned int                                    m_nBufferSize;

    unsigned int                                    m_nCurrDataLen;

    std::string                                     m_sLastError;

    std::mutex                                      m_ioLock;

  public:

    CUdpClient();

    ~CUdpClient();

    void setDataType(const std::string &sType);

    std::string getDataType();

    bool isDataType(const std::string &sType);

    unsigned int getHeaderSize();

    bool allocBuffer(const unsigned nSize = DEFAULT_TCP_BUFFER_SIZE, bool bAllocBufHeader = true);

    bool freeBuffer();

    bool clearBuffer();

    bool setBuffer(const void* pSource, const unsigned int nLen);

    bool appendBuffer(const void *pSource, const unsigned int nLen);

    unsigned int getCurrdataLen();

    bool getBuffer(void *pTarget, const unsigned int nLen, const unsigned int nStartingAt = 0);

    DataBytePtr_def getDataPtr();

    void releasePtr();

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
//* CTcpClient class defs
//*

// CTcpClient class

class CTcpClient
{
  protected:

    std::string                                 m_sURI;
    std::string                                 m_sPort;

    asio::io_context                            m_ioContext;

    asio::ip::tcp::socket                       m_netSocket;

    asio::ip::tcp::resolver                     *m_pNetResolver;

    asio::ip::tcp::resolver::results_type       m_netEndPoints;

    bool                                        m_bConnected;

    NetworkDataHeaderInfo_def                   *m_pBufferHeader;

    unsigned int                                m_nHeaderSize;

    std::string                                 m_sStreamType;

    DataBytePtr_def                             m_pDataBuffer;

    unsigned int                                m_nBufferSize;

    unsigned int                                m_nCurrDataLen;

    std::string                                 m_sLastError;

    std::mutex                                  m_ioLock;

    //bool                                      m_bTcpNoDelay;
    bool                                        m_bMultiIoSession;

    unsigned long                               m_heartBeatInterval;

    std::chrono::system_clock::time_point       m_heartBeatTimestamp;

    void setStreamType();

public:

    CTcpClient();

    ~CTcpClient();

    void setMultiIoSession(bool val)
    {
        m_bMultiIoSession = val;
    }

    void setHeartBeatInterval(unsigned long nInt)
    {
        m_heartBeatInterval = nInt;
    }

    void setDataType(const std::string &sType);

    std::string getDataType();

    bool isDataType(const std::string &sType);

    unsigned int getHeaderSize();

    bool allocBuffer(const unsigned nSize = DEFAULT_TCP_BUFFER_SIZE, bool bAllocBufHeader = true);

    bool freeBuffer();

    bool clearBuffer();

    bool setBuffer(const void* pSource, const unsigned int nLen);

    bool appendBuffer(const void *pSource, const unsigned int nLen);

    unsigned int getCurrdataLen();

    bool getBuffer(void *pTarget, const unsigned int nLen, const unsigned int nStartingAt = 0);

    DataBytePtr_def getDataPtr();

    void releasePtr();

    bool setDataSize(const unsigned int nLen);

    bool setUri(const std::string &sURI);

    bool setPort(const std::string& sPort);
    bool setPort(const unsigned int nPort);

    //void setTcpNoDelay(bool val)
    //{
    //    m_bTcpNoDelay = val;
    //}

    bool open();

    bool close();

    bool isConnected();

    std::string getRemoteAddress();

    std::string getLastError()
    {
        return m_sLastError;
    }

    int read(void *pTarget = nullptr, const unsigned int nLen = 0);

    int write(const void *pSource = nullptr, const unsigned int nLen = 0);
};


};  //  namespace CNetworkIO


#endif  //  NET_CLIENT_IO_H

