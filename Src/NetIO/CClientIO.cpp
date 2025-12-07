//****************************************************************************
// FILE:    CNetworkIO.cpp
//
// DESC:    C++ Network (TCP) input/output class 
//
// AUTHOR:  Russ Barker
//


#define _CRT_SECURE_NO_WARNINGS


#include "CClientIO.h"

#include "../String/StrUtils.h"


using namespace CNetworkIO;




// CUdpClient class


CUdpClient::CUdpClient() :
    m_pNetResolver(nullptr),
    m_bConnected(false),
    m_netSocket(m_ioContext),
    m_pBufferHeader(nullptr),
    m_nHeaderSize(0),
    m_pDataBuffer(nullptr),
    m_nBufferSize(0),
    m_nCurrDataLen(0)
{
    m_sURI = "";
    m_sPort = "";

    m_sLastError = "";
}


CUdpClient::~CUdpClient()
{
    close();

    freeBuffer();
}


void CUdpClient::setDataType(const std::string& sType)
{
    for (unsigned int x = 0; x < NET_STREAM_TYPE_LEN; x++)
    {
        if (x >= (unsigned int) sType.length())
            break;

        m_pBufferHeader->m_StreamType[x] = sType[x];
    }
}


std::string CUdpClient::getDataType()
{
    std::string sOut = "";

    sOut.assign(m_pBufferHeader->m_StreamType, NET_STREAM_TYPE_LEN);

    return sOut;
}


bool CUdpClient::isDataType(const std::string& sType)
{
    auto typeLen = sType.length();

    if (typeLen < 1)
        return false;

    if (typeLen > NET_STREAM_TYPE_LEN)
        return false;

    for (unsigned int x = 0; x < typeLen; x++)
    {
        if (m_pBufferHeader->m_StreamType[x] != sType[x])
            return false;
    }

    return true;
}


unsigned int CUdpClient::getHeaderSize()
{
    return m_nHeaderSize;
}


bool CUdpClient::allocBuffer(const unsigned nSize, bool bAllocBufHeader)
{
    if (bAllocBufHeader == true)
    {
        if (nSize < (sizeof(NetworkDataHeaderInfo_def) + 2))
            return false;
    }
    else
    {
        if (nSize < 2)
            return false;
    }

    //std::scoped_lock lock(m_ioLock);

    try
    {
        if (m_pDataBuffer != nullptr)
        {
            free(m_pDataBuffer);
            m_pDataBuffer = nullptr;
        }

        m_pDataBuffer = (DataBytePtr_def) calloc(1, nSize);
    }
    catch (...)
    {
        m_pDataBuffer = nullptr;
        return false;
    }

    if (m_pDataBuffer == nullptr)
        return false;

    m_nBufferSize = nSize;

    m_nCurrDataLen = 0;

    if (bAllocBufHeader == true)
    { 
        m_pBufferHeader = (NetworkDataHeaderInfo_def *) m_pDataBuffer;

        m_pBufferHeader->initialize();

        m_nHeaderSize = sizeof(NetworkDataHeaderInfo_def);
    }

    return true;
}


bool CUdpClient::freeBuffer()
{
    //std::scoped_lock lock(m_ioLock);

    m_pBufferHeader = nullptr;

    m_nHeaderSize = 0;

    if (m_pDataBuffer == nullptr)
        return false;

    try
    {
        delete m_pDataBuffer;
    }
    catch (...)
    {
        
    }

    m_pDataBuffer = nullptr;

    m_nBufferSize = 0;
    m_nCurrDataLen = 0;

    return true;
}


bool CUdpClient::clearBuffer()
{
    if (m_pDataBuffer == nullptr)
        return false;

    try
    {
        //std::scoped_lock lock(m_ioLock);

        memset((m_pDataBuffer + m_nHeaderSize), 0, (m_nBufferSize - m_nHeaderSize));
    }
    catch (...)
    {
        return false;
    }

    m_nCurrDataLen = 0;

    return true;
}


bool CUdpClient::setBuffer(const void* pSource, const unsigned int nLen)
{
    if (pSource == nullptr || nLen < 1)
        return false;

    if ((nLen + m_nHeaderSize) > m_nBufferSize)
        return false;

    try
    {
        //std::scoped_lock lock(m_ioLock);

        memcpy((m_pDataBuffer + m_nHeaderSize), pSource, nLen);

        m_nCurrDataLen = nLen;

        int nFillLen = (m_nBufferSize - nLen);

        if (nFillLen > 0)
        {
            memset((m_pDataBuffer + (m_nHeaderSize + nLen)), 0, nFillLen);
        }
    }
    catch (...)
    {
        return false;
    }

    return true;
}


bool CUdpClient::appendBuffer(const void *pSource, const unsigned int nLen)
{
    if (pSource == nullptr || nLen < 1)
        return false;

    if ((nLen + (m_nHeaderSize + m_nCurrDataLen)) > m_nBufferSize)
        return false;

    //std::scoped_lock lock(m_ioLock);

    try
    {
        memcpy((m_pDataBuffer + (m_nHeaderSize + m_nCurrDataLen)), pSource, nLen);
    }
    catch (...)
    {
        return false;
    }

    m_nCurrDataLen += nLen;

    return true;
}


unsigned int CUdpClient::getCurrdataLen()
{
    return m_nCurrDataLen;
}


bool CUdpClient::getBuffer(void *pTarget, const unsigned int nLen, const unsigned int nStartingAt)
{
    //std::scoped_lock lock(m_ioLock);

    if (pTarget == nullptr || nLen < 1)
        return false;

    if (nLen > (m_nBufferSize - m_nHeaderSize))
        return false;

    memcpy(pTarget, (m_pDataBuffer + (m_nHeaderSize + nStartingAt)), nLen);

    return true;
}


DataBytePtr_def CUdpClient::getDataPtr()
{
    if (m_pDataBuffer == nullptr)
        return nullptr;

    //m_ioLock.lock();

    return (m_pDataBuffer + m_nHeaderSize);
}


void CUdpClient::releasePtr()
{
    //m_ioLock.unlock();
}


bool CUdpClient::setDataSize(const unsigned int nLen)
{
    //std::scoped_lock lock(m_ioLock);

    if (nLen > (m_nBufferSize - m_nHeaderSize))
        return false;

    m_nCurrDataLen = (m_nHeaderSize + nLen);

    return true;
}


bool CUdpClient::setUri(const std::string &sURI)
{
    if (sURI == "")
        return false;

    m_sURI = sURI;

    return true;
}


bool CUdpClient::setPort(const std::string &sPort)
{
    if (sPort == "")
        return false;

    m_sPort = sPort;

    return true;
}


bool CUdpClient::setPort(const unsigned int nPort)
{
    if (nPort < 1)
        return false;

    m_sPort = std::to_string(nPort);

    return true;
}


bool CUdpClient::open()
{
    if (m_sURI == "")
        return false;

    if (m_sPort == "")
        return false;

#if 0
    if (m_bConnected == true || m_pNetResolver != nullptr)
    {
        try
        {
            m_netSocket.close();
        }
        catch (...)
        { }

        if (m_pNetResolver != nullptr)
        {
            delete m_pNetResolver;

            m_pNetResolver = nullptr;
        }

        m_bConnected = false;
    }
#else
    if (m_bConnected == true)
    {
        return true;
    }
#endif

    try
    {
        m_pNetResolver = new asio::ip::udp::resolver(m_ioContext);
    }
    catch (...)
    {
        m_bConnected = false;

        return false;
    }

    if (m_pNetResolver == nullptr)
    {
        m_bConnected = false;

        return false;
    }

    try
    {
        m_netEndPoint = *m_pNetResolver->resolve(m_sURI.c_str(), m_sPort.c_str()).begin();
    }
    catch (...)
    {
        m_bConnected = false;

        return false;
    }

    try
    {
        m_netSocket.open(asio::ip::udp::v4());
    }
    catch (...)
    {
        delete m_pNetResolver;

        m_pNetResolver = nullptr;

        m_bConnected = false;

        return false;
    }

    m_bConnected = true;

    return true;
}


bool CUdpClient::close()
{
    try
    {
        m_netSocket.close();
    }
    catch (...)
    {
    }

    if (m_pNetResolver != nullptr)
    {
        delete m_pNetResolver;

        m_pNetResolver = nullptr;
    }

    m_bConnected = false;

    return true;
}


bool CUdpClient::isConnected()
{
    if (m_bConnected == false)
        return false;

    try
    { 
        m_bConnected = m_netSocket.is_open();
    }
    catch (...)
    {
        return false;
    }

    return m_bConnected;
}


int CUdpClient::read(void *pTarget, const unsigned int nLen)
{
    if (m_pDataBuffer == nullptr && (pTarget == nullptr || nLen < 1))
        return -1;

    if (nLen > m_nBufferSize)
        return -2;

    clearBuffer();

    int readSize = 0;

    int nPayloadSize = 0;

    {
        //std::scoped_lock lock(m_ioLock);

        try
        {
            asio::error_code error;

            error.clear();

            auto recv_buffer = asio::buffer(m_pDataBuffer, m_nBufferSize);

            //readSize = (int) asio::read(m_netSocket, recv_buffer, error);
            readSize = (int) 
                m_netSocket.receive_from
                (
                    recv_buffer, 
                    m_netEndPoint, 
                    (asio::socket_base::message_flags) 0,
                    error
                );

            if (error)
            {
                m_sLastError = error.message();

                return -4;
            }
            else
            {
                m_sLastError = "";
            }

            if (readSize > 0)
            { 
                m_nCurrDataLen = (unsigned int) readSize;
            }
            else
            { 
                m_nCurrDataLen = 0;
            }
        }
        catch (...)
        {
            return -5;
        }

        if (readSize < 1)
        {
            return readSize;
        }

        nPayloadSize = (readSize - m_nHeaderSize);

        if (pTarget != nullptr && nLen > 0)
        {
            if (nPayloadSize < (int) nLen)
                memcpy(pTarget, (m_pDataBuffer + m_nHeaderSize), nPayloadSize);
            else
                memcpy(pTarget, (m_pDataBuffer + m_nHeaderSize), nLen);
        }
    }

    return nPayloadSize;
}


int CUdpClient::write(const void *pSource, const unsigned int nLen)
{
    if (m_pDataBuffer == nullptr && (pSource == nullptr || nLen < 1))
        return -1;

    if ((pSource != nullptr && nLen < 1) || (pSource == nullptr && nLen > 0))
        return -2;

    int nWriteSize = 0;

    try
    {
        void *pData = nullptr;

        std::size_t dataSize = 0;

        asio::error_code error;

        error.clear();

        {
            //std::scoped_lock lock(m_ioLock);

            if (pSource != nullptr)
            { 
                pData = (void *) pSource;

                dataSize = (std::size_t) nLen;
            }
            else
            { 
                pData = (void *) m_pDataBuffer;

                dataSize = (std::size_t) (m_nHeaderSize + m_nCurrDataLen);

                if (m_nHeaderSize == sizeof(NetworkDataHeaderInfo_def))
                {
                    m_pBufferHeader->m_nDataLen = (uint32_t) dataSize;
                }
            }

            asio::const_buffer send_buffer = asio::buffer(pData, dataSize);

            //nWriteSize = (int) asio::write(m_netSocket, send_buffer);
            nWriteSize = (int) 
                m_netSocket.send_to
                (
                    send_buffer, 
                    m_netEndPoint,
                    (asio::socket_base::message_flags) 0,
                    error
                );

            if (error)
            {
                m_sLastError = error.message();

                return -4;
            }
            else
            {
                m_sLastError = "";
            }
        }

        if (nWriteSize > 0)
        {
            m_sLastError = "";
        }
        else
        {
            m_sLastError = "write failed";
        }

        m_nCurrDataLen = 0;
    }
    catch (...)
    {
        return -5;
    }

    return nWriteSize;
}


// CTcpClient class


CTcpClient::CTcpClient() :
    m_pNetResolver(nullptr),
    m_bConnected(false),
    m_netSocket(m_ioContext),
    m_pBufferHeader(nullptr),
    m_nHeaderSize(0),
    m_pDataBuffer(nullptr),
    m_nBufferSize(0),
    m_nCurrDataLen(0),
    //m_bTcpNoDelay(TCP_NO_DELAY_DEFAULT)
    m_bMultiIoSession(false),
    m_heartBeatInterval(0)
{
    m_sURI = "";
    m_sPort = "";

    m_sLastError = "";
}


CTcpClient::~CTcpClient()
{
    close();

    freeBuffer();
}


void CTcpClient::setStreamType()
{
    for (unsigned int x = 0; x < NET_STREAM_TYPE_LEN; x++)
    {
        if (x >= (unsigned int) m_sStreamType.length())
            break;

        m_pBufferHeader->m_StreamType[x] = m_sStreamType[x];
    }
}


void CTcpClient::setDataType(const std::string& sType)
{
    m_sStreamType = sType;

    setStreamType();
}


std::string CTcpClient::getDataType()
{
    std::string sOut = "";

    sOut.assign(m_pBufferHeader->m_StreamType, NET_STREAM_TYPE_LEN);

    return sOut;
}


bool CTcpClient::isDataType(const std::string& sType)
{
    auto typeLen = sType.length();

    if (typeLen < 1)
        return false;

    if (typeLen > NET_STREAM_TYPE_LEN)
        return false;

    for (unsigned int x = 0; x < typeLen; x++)
    {
        if (m_pBufferHeader->m_StreamType[x] != sType[x])
            return false;
    }

    return true;
}


unsigned int CTcpClient::getHeaderSize()
{
    return m_nHeaderSize;
}


bool CTcpClient::allocBuffer(const unsigned nSize, bool bAllocBufHeader)
{
    if (bAllocBufHeader == true)
    {
        if (nSize < (sizeof(NetworkDataHeaderInfo_def) + 2))
            return false;
    }
    else
    {
        if (nSize < 2)
            return false;
    }

    //std::scoped_lock lock(m_ioLock);

    try
    {
        if (m_pDataBuffer != nullptr)
        {
            free(m_pDataBuffer);
            m_pDataBuffer = nullptr;
        }

        m_pDataBuffer = (DataBytePtr_def) calloc(1, nSize);
    }
    catch (...)
    {
        m_pDataBuffer = nullptr;
        return false;
    }

    if (m_pDataBuffer == nullptr)
        return false;

    m_nBufferSize = nSize;

    m_nCurrDataLen = 0;

    if (bAllocBufHeader == true)
    { 
        m_pBufferHeader = (NetworkDataHeaderInfo_def *) m_pDataBuffer;

        m_pBufferHeader->initialize();

        m_nHeaderSize = sizeof(NetworkDataHeaderInfo_def);
    }

    return true;
}


bool CTcpClient::freeBuffer()
{
    //std::scoped_lock lock(m_ioLock);

    m_pBufferHeader = nullptr;

    m_nHeaderSize = 0;

    if (m_pDataBuffer == nullptr)
        return false;

    try
    {
        delete m_pDataBuffer;
    }
    catch (...)
    {
        
    }

    m_pDataBuffer = nullptr;

    m_nBufferSize = 0;
    m_nCurrDataLen = 0;

    return true;
}


bool CTcpClient::clearBuffer()
{
    if (m_pDataBuffer == nullptr)
        return false;

    try
    {
        //std::scoped_lock lock(m_ioLock);

        memset((m_pDataBuffer + m_nHeaderSize), 0, (m_nBufferSize - m_nHeaderSize));
    }
    catch (...)
    {
        return false;
    }

    m_nCurrDataLen = 0;

    return true;
}


bool CTcpClient::setBuffer(const void* pSource, const unsigned int nLen)
{
    if (pSource == nullptr || nLen < 1)
        return false;

    if ((nLen + m_nHeaderSize) > m_nBufferSize)
        return false;

    try
    {
        //std::scoped_lock lock(m_ioLock);

        memcpy((m_pDataBuffer + m_nHeaderSize), pSource, nLen);

        m_nCurrDataLen = nLen;

        int nFillLen = (m_nBufferSize - nLen);

        if (nFillLen > 0)
        {
            memset((m_pDataBuffer + (m_nHeaderSize + nLen)), 0, nFillLen);
        }
    }
    catch (...)
    {
        return false;
    }

    return true;
}


bool CTcpClient::appendBuffer(const void *pSource, const unsigned int nLen)
{
    if (pSource == nullptr || nLen < 1)
        return false;

    if ((nLen + (m_nHeaderSize + m_nCurrDataLen)) > m_nBufferSize)
        return false;

    //std::scoped_lock lock(m_ioLock);

    try
    {
        memcpy((m_pDataBuffer + (m_nHeaderSize + m_nCurrDataLen)), pSource, nLen);
    }
    catch (...)
    {
        return false;
    }

    m_nCurrDataLen += nLen;

    return true;
}


unsigned int CTcpClient::getCurrdataLen()
{
    return m_nCurrDataLen;
}


bool CTcpClient::getBuffer(void *pTarget, const unsigned int nLen, const unsigned int nStartingAt)
{
    //std::scoped_lock lock(m_ioLock);

    if (pTarget == nullptr || nLen < 1)
        return false;

    if (nLen > (m_nBufferSize - m_nHeaderSize))
        return false;

    memcpy(pTarget, (m_pDataBuffer + (m_nHeaderSize + nStartingAt)), nLen);

    return true;
}


DataBytePtr_def CTcpClient::getDataPtr()
{
    if (m_pDataBuffer == nullptr)
        return nullptr;

    //m_ioLock.lock();

    return (m_pDataBuffer + m_nHeaderSize);
}


void CTcpClient::releasePtr()
{
    //m_ioLock.unlock();
}


bool CTcpClient::setDataSize(const unsigned int nLen)
{
    //std::scoped_lock lock(m_ioLock);

    if (nLen > (m_nBufferSize - m_nHeaderSize))
        return false;

    m_nCurrDataLen = (m_nHeaderSize + nLen);

    return true;
}


bool CTcpClient::setUri(const std::string &sURI)
{
    if (sURI == "")
        return false;

    m_sURI = sURI;

    return true;
}


bool CTcpClient::setPort(const std::string &sPort)
{
    if (sPort == "")
        return false;

    m_sPort = sPort;

    return true;
}


bool CTcpClient::setPort(const unsigned int nPort)
{
    if (nPort < 1)
        return false;

    m_sPort = std::to_string(nPort);

    return true;
}


bool CTcpClient::open()
{
    if (m_sURI == "")
        return false;

    if (m_sPort == "")
        return false;

    //if (m_bTcpNoDelay == true)
    //{
        //asio::socket_base::keep_alive keepAlive(true);

        //m_netSocket.set_option(keepAlive);

        //asio::ip::tcp::no_delay noDelay(true);

        //m_netSocket.set_option(noDelay);
    //}

    if (m_bConnected == true)
    {
        return true;
    }

    try
    {
        m_pNetResolver = new asio::ip::tcp::resolver(m_ioContext);
    }
    catch (...)
    {
        m_sLastError = "unknown exception creating resolver";

        m_bConnected = false;

        return false;
    }

    if (m_pNetResolver == nullptr)
    {
        m_bConnected = false;

        return false;
    }

    try
    {
        m_netEndPoints = m_pNetResolver->resolve(m_sURI.c_str(), m_sPort.c_str());

        if (m_netEndPoints.empty())
        {
            m_bConnected = false;

            return false;
        }
    }
    catch (...)
    {
        m_sLastError = "unknown exception during 'resolve'";

        m_bConnected = false;

        return false;
    }

    try
    {
        // open TCP socket

        asio::error_code error;

        error.clear();

        asio::connect(m_netSocket, m_netEndPoints, error);

        if (error)
        {
            m_sLastError = error.message();

            close();

            m_bConnected = false;

            return false;
        }
        else
        {
            m_sLastError = "";
        }

        // set flag to report aborted socket ops

        asio::socket_base::enable_connection_aborted option(true);

        m_netSocket.set_option(option);

        m_sLastError.clear();
    }
    catch (...)
    {
        delete m_pNetResolver;

        m_pNetResolver = nullptr;

        m_sLastError = "unknown exception during 'connect'";

        m_bConnected = false;

        return false;
    }

    m_bConnected = true;

    LogDebugInfoMsg("TCP connection opened");

    return true;
}


bool CTcpClient::close()
{
    LogDebugInfoMsg("sending 'exit' msg to server");

    write("exit", 4);

    //std::this_thread::sleep_for(std::chrono::milliseconds(50));

    try
    {
        m_netSocket.shutdown(asio::ip::tcp::socket::shutdown_send);
    }
    catch (...)
    {
    }

    try
    {
        m_netSocket.close();
    }
    catch (...)
    {
    }

    if (m_pNetResolver != nullptr)
    {
        delete m_pNetResolver;

        m_pNetResolver = nullptr;
    }

    m_bConnected = false;

    LogDebugInfoMsg("TCP connection closed");

    return true;
}


bool CTcpClient::isConnected()
{
    if (m_bConnected == false)
    { 
        return false;
    }

    try
    { 
        m_bConnected = m_netSocket.is_open();

        if (m_bConnected == false || getRemoteAddress() == "")
        {
            m_netSocket.close();

            m_bConnected = false;
        }

        asio::error_code error;

        error.clear();

        auto bytesAvail = m_netSocket.available(error);

        if (error)
        {
            m_sLastError = error.message();

            m_bConnected = false;

            return false;
        }
    }
    catch (...)
    {
        return false;
    }

    return m_bConnected;
}


std::string CTcpClient::getRemoteAddress()
{
    if (m_bConnected == false)
    {
        return "";
    }

    try
    { 
        asio::error_code error;

        error.clear();

        auto endPoint = m_netSocket.remote_endpoint(error);

        if (error)
        {
            m_sLastError = error.message();

            m_bConnected = false;

            return "";
        }

        auto remoteAddr = endPoint.address();

        return remoteAddr.to_string();
    }
    catch (...)
    {
        m_sLastError = "unknown exception getting remote endpoint";

        m_bConnected = false;
    }

    return "";
}


int CTcpClient::read(void *pTarget, const unsigned int nLen)
{
    if (m_pDataBuffer == nullptr && (pTarget == nullptr || nLen < 1))
    { 
        return -1;
    }

    int8_t *pTargetBuf = nullptr;

    unsigned int nBytesToRead = 0;

    if (pTarget != nullptr)
    {
        pTargetBuf = (int8_t *) pTarget;

        nBytesToRead = nLen;
    }
    else
    {
        pTargetBuf = (int8_t*) m_pDataBuffer;

        nBytesToRead = m_nBufferSize;
    }

    if (nBytesToRead < 1)
    {
        return -2;
    }

    clearBuffer();

    int readSize = 0;

    int nPayloadSize = 0;

    {
        //std::scoped_lock lock(m_ioLock);

        try
        {
            if (m_netSocket.is_open() == false)
            {
                m_bConnected = false;

                return -20;
            }

            asio::error_code error;

            error.clear();

            auto bytesAvail = m_netSocket.available(error);

            if (error)
            {
                m_sLastError = error.message();

                m_bConnected = false;

                return -20;
            }

            auto minMsgSize = 8;

            if (bytesAvail < minMsgSize)
            {
                // check for msg timeout
                if (m_heartBeatInterval > 0)
                {
                    auto now = std::chrono::system_clock::now();

                    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_heartBeatTimestamp);

                    if (dur.count() > m_heartBeatInterval)
                    {
                        // signal timeout

                        m_bConnected = false;

                        return -10;
                    }
                }

                return 0;
            }

            error.clear();

            readSize = (int) asio::read(m_netSocket, asio::buffer(m_pDataBuffer, bytesAvail), error);

            if (error)
            {
                m_sLastError = error.message();

                m_bConnected = false;

                return -20;
            }

            m_sLastError.clear();

            if (readSize > 0)
            { 
                if (readSize >= 4)
                {
                    std::string sTmp = (char *) (m_pDataBuffer + m_nHeaderSize);

                    if (StrUtils::strCompare("exit", sTmp, 4) == 0)
                    {
                        LogDebugInfoMsg("server 'exit' msg received");

                        m_nCurrDataLen = 0;

                        m_bConnected = false;

                        return -30;
                    }
                }

                m_nCurrDataLen = (unsigned int) readSize;
            }
            else
            { 
                m_nCurrDataLen = 0;
            }
        }
        catch (...)
        {
            m_netSocket.close();

            m_bConnected = false;

            return -5;
        }

        if (readSize < 1)
        {
            return readSize;
        }

        nPayloadSize = (readSize - m_nHeaderSize);

        if (pTarget != nullptr && nLen > 0)
        {
            if (nPayloadSize < (int) nLen)
            { 
                memcpy(pTarget, (m_pDataBuffer + m_nHeaderSize), nPayloadSize);
            }
            else
            { 
                memcpy(pTarget, (m_pDataBuffer + m_nHeaderSize), nLen);
            }
        }
    }

    return nPayloadSize;
}


int CTcpClient::write(const void *pSource, const unsigned int nLen)
{
    if (m_pDataBuffer == nullptr && (pSource == nullptr || nLen < 1))
        return -1;

    if ((pSource != nullptr && nLen < 1) || (pSource == nullptr && nLen > 0))
        return -2;

    if (pSource != nullptr && nLen > m_nBufferSize)
        return -3;

    if (pSource == nullptr && m_nCurrDataLen < 1)
        return -4;

    int nWriteSize = 0;

    try
    {
        std::size_t dataSize = 0;

        {
            //std::scoped_lock lock(m_ioLock);

            if (pSource != nullptr)
            { 
                dataSize = (std::size_t) (m_nHeaderSize + nLen);

                if (m_nHeaderSize == sizeof(NetworkDataHeaderInfo_def))
                {
                    m_pBufferHeader = (NetworkDataHeaderInfo_def *) m_pDataBuffer;

                    m_pBufferHeader->initialize();

                    setStreamType();

                    m_pBufferHeader->m_nDataLen = nLen;
                }

                memcpy((((int8_t *) m_pDataBuffer) + m_nHeaderSize), pSource, nLen);

                m_nCurrDataLen = nLen;
            }
            else
            { 
                dataSize = (std::size_t) (m_nHeaderSize + m_nCurrDataLen);

                if (m_nHeaderSize == sizeof(NetworkDataHeaderInfo_def))
                {
                    m_pBufferHeader = (NetworkDataHeaderInfo_def*) m_pDataBuffer;

                    m_pBufferHeader->m_nDataLen = (uint32_t) m_nCurrDataLen;
                }
            }

            asio::error_code error;

            error.clear();

            asio::const_buffer send_buffer = asio::buffer(m_pDataBuffer, dataSize);

            nWriteSize = (int) asio::write(m_netSocket, send_buffer, error);

            if (error)
            {
                m_sLastError = error.message();

                return -20;
            }

            m_sLastError.clear();
        }

        m_sLastError.clear();

        m_nCurrDataLen = 0;
    }
    catch (...)
    {
        m_netSocket.close();

        m_bConnected = false;

        return -5;
    }

    return nWriteSize;
}


