//****************************************************************************
// FILE:    CNetworkIO.cpp
//
// DESC:    C++ Network (TCP) input/output class 
//
// AUTHOR:  Russ Barker
//


#include "CNetworkIO.h"


using namespace CNetworkIO;


// CTcpClient class


CTcpClient::CTcpClient() :
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


CTcpClient::~CTcpClient()
{
    close();

    freeBuffer();
}


void CTcpClient::setDataType(const std::string& sType)
{
    for (unsigned int x = 0; x < NET_STREAM_TYPE_LEN; x++)
    {
        if (x >= (unsigned int) sType.length())
            break;

        m_pBufferHeader->m_StreamType[x] = sType[x];
    }
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
        m_pNetResolver = new asio::ip::tcp::resolver(m_ioContext);
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
        m_netEndPoints = m_pNetResolver->resolve(m_sURI.c_str(), m_sPort.c_str());

        if (m_netEndPoints.empty())
        {
            m_bConnected = false;

            return false;
        }
    }
    catch (...)
    {
        m_bConnected = false;

        return false;
    }

    try
    {
        asio::connect(m_netSocket, m_netEndPoints);
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


bool CTcpClient::close()
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


bool CTcpClient::isConnected()
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


int CTcpClient::read(void *pTarget, const unsigned int nLen)
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

            readSize = (int) asio::read(m_netSocket, asio::buffer(m_pDataBuffer, m_nBufferSize), error);

            m_sLastError = error.message();

            if (readSize > 0)
                m_nCurrDataLen = (unsigned int) readSize;
            else
                m_nCurrDataLen = 0;
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


int CTcpClient::write(const void *pSource, const unsigned int nLen)
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

            nWriteSize = (int) asio::write(m_netSocket, send_buffer);
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


//*
//* CTcpServer class defs
//*


// CNetMessageData clss

CNetMessageData::CNetMessageData(unsigned int nHeaderLength) :
    m_nHeaderLength(nHeaderLength),
    m_pData(nullptr),
    m_nMaxDataSize(0),
    m_nDataLength(0),
    m_bUpdated(false)
{
        
}


bool CNetMessageData::allocBuffer(unsigned int nMaxDataSize)
{
    std::scoped_lock lock(m_dataLock);

    if (nMaxDataSize < 1)
        return false;

    if (m_pData != nullptr)
    {
        free(m_pData);

        m_pData = nullptr;
    }

    m_nHeaderLength = sizeof(NetworkDataHeaderInfo_def);

    auto nBufSize = (m_nHeaderLength + nMaxDataSize);

    m_pData = (char *) malloc(nBufSize);

    if (m_pData == nullptr)
        return false;

    memset(m_pData, 0, (nBufSize));

    m_nMaxDataSize = nBufSize;

    return true;
}


bool CNetMessageData::isBufferAllocated()
{
    if (m_pData == nullptr && m_nMaxDataSize > 0)
        return false;

    return true;
}


int CNetMessageData::getMaxDataLen()
{
    if (m_nMaxDataSize == 0)
        return 0;

    if (m_nMaxDataSize < m_nHeaderLength)
        return 0;

    return (int) (m_nMaxDataSize - m_nHeaderLength);
}


int CNetMessageData::getCurDataLen()
{
    return m_nDataLength;
}


void CNetMessageData::clearAll()
{
    std::scoped_lock lock(m_dataLock);

    m_nDataLength = 0;

    if (m_pData == nullptr)
        return;

    memset(m_pData, 0, m_nMaxDataSize);
}


void CNetMessageData::clearMsgBody()
{
    std::scoped_lock lock(m_dataLock);

    m_nDataLength = 0;

    if (m_pData == nullptr)
        return;

    memset((m_pData + MsgHeaderLen_def), 0, (m_nMaxDataSize - MsgHeaderLen_def));
}


char* CNetMessageData::getDataPtr()
{ 
    m_dataLock.lock();

    return m_pData;
}


char* CNetMessageData::getBodyPtr()
{ 
    m_dataLock.lock();
    
    return (m_pData + m_nHeaderLength);
}


void CNetMessageData::releasePtr()
{
    m_dataLock.unlock();
}


void CNetMessageData::setHeaderLength(const std::size_t nLen)
{
    m_nHeaderLength = (unsigned int) nLen;
}


int CNetMessageData::getHeaderLength()
{
    return (int) m_nHeaderLength;
}


void CNetMessageData::setBodyLength(const std::size_t nLen)
{
    m_nDataLength = (unsigned int) (m_nHeaderLength + nLen);
}


int CNetMessageData::getBodyLength()
{
    return (int) (m_nDataLength - m_nHeaderLength);
}


void CNetMessageData::setUpdated(bool val)
{
    m_bUpdated = val;
}


bool CNetMessageData::isUpdated()
{
    return m_bUpdated;
}


// CNetMessageHandler clss

CNetMessageHandler::CNetMessageHandler(CNetMessageData &msgData) :
    m_msgData(msgData)
{

}


bool CNetMessageHandler::decodeMsgHeader(const std::string& sType)
{
    m_msgData.setBodyLength(0);

    if (m_msgData.isBufferAllocated() == false)
        return false;

    NetworkDataHeaderInfo_def *pHeader = (NetworkDataHeaderInfo_def *) m_msgData.getDataPtr();
        
    if (pHeader->m_headerMarker != 0xFFFF)
    {
        m_msgData.releasePtr();
        return false;
    }

    if (pHeader->m_nDataLen >= (uint32_t) m_msgData.getMaxDataLen())
    {
        m_msgData.releasePtr();
        return false;
    }

    m_msgData.setBodyLength(pHeader->m_nDataLen);

    m_msgData.releasePtr();

    return true;
}


bool CNetMessageHandler::encodeMsgHeader(const std::string& sType)
{
    if (m_msgData.isBufferAllocated() == false)
        return false;

    NetworkDataHeaderInfo_def* pMsgHeader = (NetworkDataHeaderInfo_def*) m_msgData.getDataPtr();

    pMsgHeader->initialize(sType.c_str(), (unsigned int) m_msgData.getBodyLength());

    m_msgData.releasePtr();

    return true;
}


// CTcpSession class

CTcpSession::CTcpSession
    (
        asio::ip::tcp::socket& socket,
        std::string& sMsgType
    ) :
    m_socket(socket),
    m_sMsgType(sMsgType)
{
    m_sLastError.clear();
}


CTcpSession::~CTcpSession()
{
}


//asio::ip::tcp::socket* CTcpSession::socketPtr()
//{
//    return &m_socket;
//}


bool CTcpSession::readMsgHeader(CNetMessageData &msgData)
{
    bool bReleased = false;

    msgData.clearAll();

    auto nLen = msgData.getHeaderLength();

    if (nLen < 1)
    {
        return false;
    }

    // read the message header data

    auto pData = msgData.getDataPtr();

    if (pData == nullptr)
    {
        msgData.releasePtr();
        return false;
    }

    try
    {
        auto status = asio::read(m_socket, asio::buffer(pData, nLen));

        msgData.releasePtr();

        bReleased = true;

        if (status < nLen)
        {
            return false;
        }

        CNetMessageHandler msgHandler(msgData);

        msgHandler.decodeMsgHeader(m_sMsgType);
    }
    catch (std::exception e)
    {
        if (bReleased == false)
            msgData.releasePtr();
        
        std::string err = e.what();
        
        return false;
    }
    catch (...)
    {
        if (bReleased == false)
            msgData.releasePtr();
        
        return false;
    }

    return true;
}


bool  CTcpSession::readMsgBody(CNetMessageData& msgData)
{
    bool bReleased = false;

    msgData.clearMsgBody();

    auto nLen = msgData.getBodyLength();

    if (nLen < 1)
    {
        return false;
    }

    // read the message body

    auto pData = msgData.getBodyPtr();

    if (pData == nullptr)
    {
        msgData.releasePtr();
        return false;
    }

    try
    {
        auto status = asio::read(m_socket, asio::buffer(pData, nLen));

        msgData.releasePtr();

        bReleased = true;

        if (status < nLen)
        {
            return false;
        }
        else
        {
            msgData.setUpdated(true);
        }
    }
    catch (std::exception e)
    {
        if (bReleased == false)
            msgData.releasePtr();
        
        std::string err = e.what();
        
        return false;
    }
    catch (...)
    {
        if (bReleased == false)
            msgData.releasePtr();
        
        return false;
    }

    return true;
}


bool CTcpSession::writeMsgData(CNetMessageData &msgData)
{
    std::scoped_lock lock(m_mutex);

    bool bReleased = false;

    auto nLen = msgData.getCurDataLen();

    if (nLen < 1)
    {
        return false;
    }

    // encode/format the msg header

    CNetMessageHandler  msgHandler(msgData);

    msgHandler.encodeMsgHeader(m_sMsgType);

    // write the message to the net

    auto pData = msgData.getDataPtr();

    if (pData == nullptr)
    {
        msgData.releasePtr();
        
        return false;
    }

    try
    {
        auto status = asio::write(m_socket, asio::buffer(pData, nLen));

        msgData.releasePtr();

        bReleased = true;

        if (status < nLen)
        {
            return false;
        }

        msgData.setUpdated(false);

    }
    catch (std::exception e)
    {
        if (bReleased == false)
            msgData.releasePtr();

        std::string err = e.what();
        
        return false;
    }
    catch (...)
    {
        if (bReleased == false)
            msgData.releasePtr();
        
        return false;
    }

    return true;
}


bool CTcpSession::readMsgData(CNetMessageData &msgData)
{
    std::scoped_lock lock(m_mutex);

    // read/decode the message header
    if (readMsgHeader(msgData) == true)
    {
        // read the message body

        if (readMsgBody(msgData) == false)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}



// CTcpServer class

CTcpServer::CTcpServer(eNetIoDirection eDir, const unsigned int nPort, const unsigned int nSize) :
    m_port(nPort),
    m_bufferSize(nSize),
    m_eIoDirection(eDir),
    m_pEndpoint(nullptr),
    m_pAcceptor(nullptr),
    m_inputMsg(MsgHeaderLen_def),
    m_outputMsg(MsgHeaderLen_def),
    m_bInitialized(false),
    m_bRunning(false)
{

}


CTcpServer::~CTcpServer()
{
    if (m_ioContext.stopped() == false)
    {
        m_ioContext.stop();
    }
}


bool CTcpServer::setPort(const unsigned int nPort)
{
    if (m_bRunning == true)
        return false;

    m_port = nPort;

    return true;
}


bool CTcpServer::setBufferSize(const unsigned int nSize)
{
    if (m_bInitialized == true)
        return false;

    m_bufferSize = nSize;

    return true;
}


bool CTcpServer::setDataType(const std::string& sType)
{
    if (sType == "")
        return false;

    m_sDataType = sType;

    return true;
}


bool CTcpServer::setName(const std::string& sName)
{
    if (sName == "")
        return false;

    m_sSrvrName = sName;

    return true;
}


bool CTcpServer::initialize(const unsigned int nBufize)
{
    if (nBufize > 0)
        m_bufferSize = nBufize;

    if (m_bufferSize < 1)
        return false;

    switch (m_eIoDirection)
    {
    case eNetIoDirection::eNetIoDirection_input:
        if (m_inputMsg.allocBuffer(nBufize) == false)
        {
            return false;
        }
        break;

    case eNetIoDirection::eNetIoDirection_output:
        if (m_outputMsg.allocBuffer(nBufize) == false)
        {
            return false;
        }
        break;

    case eNetIoDirection::eNetIoDirection_IO:
        if (m_inputMsg.allocBuffer(nBufize) == false)
        {
            return false;
        }
        if (m_outputMsg.allocBuffer(nBufize) == false)
        {
            return false;
        }
        break;

    default:
        return false;
    }

    m_bInitialized = true;

    return true;
}


void runSrvr(asio::io_context &ioContext)
{
    ioContext.run();
}


bool CTcpServer::acceptConnection()
{
    if (m_pAcceptor == nullptr)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(10));

        acceptConnection(); // Accept next connection

        return false;
    }

    //bool bRet = false;

    m_pAcceptor->async_accept
    (
        [this](const asio::error_code& error, asio::ip::tcp::socket socket)
        {
            if (!error)
            {
                if (socket.is_open() == false)
                    return;

                asio::socket_base::keep_alive option(true);

                socket.set_option(option);

                auto pSession = new CTcpSession(socket, m_sDataType);

                if (pSession != nullptr)
                { 
                    // handle server session

#ifdef MULTI_MSG_SESSION                    
                    //while (1)
#endif
                    { 
                        switch (m_eIoDirection)
                        {
                        case eNetIoDirection::eNetIoDirection_input:
                            {
                                // read input msg
                                //m_inputMsg.clearAll();
                                pSession->readMsgData(m_inputMsg);
                            }
                            break;

                        case eNetIoDirection::eNetIoDirection_output:
                            // if needed, write output msg
                            if (m_outputMsg.isUpdated() == true)
                            { 
                                pSession->writeMsgData(m_outputMsg);
                                //m_outputMsg.clearAll();
                            }
                            break;

                        case eNetIoDirection::eNetIoDirection_IO:
                            {
                                // read input msg
                                //m_inputMsg.clearAll();
                                pSession->readMsgData(m_inputMsg);

                                // process input msg
                                if (processInputMsg(m_inputMsg, m_outputMsg) == false)
                                    break;
                            
                                // if needed, write output msg
                                if (m_outputMsg.isUpdated() == true)
                                {
                                    pSession->writeMsgData(m_outputMsg);
                                    //m_outputMsg.clearAll();
                                }
                            }
                            break;
                        }

#ifdef MULTI_MSG_SESSION                    
                        if (socket.is_open() == false)
                        { 
                            // end of server session
                            break;
                        }
#endif
                    }

                    delete pSession;
                }
            }

            acceptConnection(); // Accept next connection
        }
    );

    return true;   //bRet;
}


bool CTcpServer::start()
{
    if (m_bRunning == true)
        return false;

    try
    {
        if (m_pAcceptor != nullptr)
            delete m_pAcceptor;
    }
    catch (...)
    {
    }

    try 
    {
        m_pEndpoint = new asio::ip::tcp::endpoint(asio::ip::tcp::v4(), m_port);

        if (m_pEndpoint == nullptr)
        { 
            return false;
        }

        m_pAcceptor = new 
            asio::ip::tcp::acceptor
            (
                m_ioContext,
                *m_pEndpoint
            );

        if (m_pAcceptor == nullptr)
        { 
            delete m_pEndpoint;

            m_pEndpoint = nullptr;

            return false;
        }

        acceptConnection();

        m_srvrThread.setIoContext(&m_ioContext);

        m_srvrThread.setName(m_sSrvrName + "_server");

        if (m_srvrThread.createThread() == false)
        {
            delete m_pAcceptor;

            m_pAcceptor = nullptr;

            delete m_pEndpoint;

            m_pEndpoint = nullptr;

            return false;
        }
    }
    catch (...) 
    {
        if (m_pAcceptor != nullptr)
        {
            delete m_pAcceptor;

            m_pAcceptor = nullptr;

            delete m_pEndpoint;

            m_pEndpoint = nullptr;
        }

        return false;
    }

    m_bRunning = true;

    return true;
}


bool CTcpServer::stop()
{
    if (m_bRunning == false)
        return true;

    try
    {
        m_ioContext.stop();

        if (m_srvrThread.isActive() == true)
        {
            m_srvrThread.stopThread();
        }
    }
    catch (...)
    {
        return false;
    }

    try
    {
        if (m_pAcceptor != nullptr)
            delete m_pAcceptor;
    }
    catch (...)
    {
    }

    m_bRunning = false;

    return true;
}


bool CTcpServer::isRunning()
{
    return m_bRunning;
}


void CTcpServer::setRunning(bool bVal)
{
    m_bRunning = false;
}


int CTcpServer::readInputData(void* pBuff, const unsigned int nMax)
{
    if (pBuff == nullptr || nMax < 1)
    { 
        return -1;
    }

    size_t nCopyLen = (size_t)nMax;

    auto nMsgLen = m_inputMsg.getBodyLength();

    if (nMsgLen < nCopyLen)
    {
        nCopyLen = (size_t)nMsgLen;
    }

    auto pData = m_inputMsg.getBodyPtr();

    if (pData == nullptr)
    { 
        m_outputMsg.releasePtr();

        return -2;
    }

    memcpy(pBuff, pData, nCopyLen);

    m_inputMsg.releasePtr();

    return (int)nCopyLen;
}


int CTcpServer::writeOutputData(const void* pBuff, const unsigned int nLen)
{
    if (pBuff == nullptr || nLen < 1)
    { 
        return -1;
    }

    // make sure buffer has been allocated

    if (m_outputMsg.isBufferAllocated() == false)
    {
        m_outputMsg.allocBuffer((unsigned int) nLen);
    }

    size_t nCopyLen = (size_t) nLen;

    auto nMaxLen = m_outputMsg.getMaxDataLen();

    // make sure buffer it big enough

    if (nMaxLen < nCopyLen)
    {
        m_outputMsg.allocBuffer((unsigned int) nCopyLen);
    }

    auto pData = m_outputMsg.getBodyPtr();

    if (pData == nullptr)
    { 
        m_outputMsg.releasePtr();

        return false;
    }

    memcpy(pData, pBuff, nCopyLen);

    m_outputMsg.releasePtr();

    m_outputMsg.setBodyLength(nCopyLen);

    return (int) nCopyLen;
}


bool CTcpServer::sendOutput()
{
    m_outputMsg.setUpdated(true);

    return true;
}


bool CTcpServer::processInputMsg(CNetMessageData& inputMsg, CNetMessageData& outputMsg)
{
    // This is a placeholder for an input message processing function
    // for server I/O processing.  Override this function for your 
    // processing logic.

    return true;
}


//*
//* utility functions
//*

bool CNetworkIO::parseServerAndPort(const std::string& sUri, std::string& sServer, std::string &sPort, std::string &sProtocol)
{
    if (sUri == "")
    {
        return false;
    }

    size_t nSkip = 0;

    size_t nColonPos = 0;

    size_t nPtotocolPos = sUri.find("://");

    if (nPtotocolPos != std::string::npos)
    {
        // server uri contains protocol ("://")

        nSkip = (nPtotocolPos + 3);
    }

    nColonPos = sUri.rfind(":");

    if (nColonPos == std::string::npos)
    {
        // there is no ":" in the uri
        // return the uri chars and a blank port string 

        sServer = sUri.substr(nSkip);
        sPort = "";
        
        if (nSkip > 3)
            sProtocol = sUri.substr(0, (nSkip - 3));
        else
            sProtocol;

        return true;
    }

    // return the uri string chars and the port string chars

    sServer = sUri.substr(nSkip, (nColonPos - nSkip));

    if (nColonPos > nSkip + 1)
        sPort = sUri.substr((nColonPos + 1));
    else
        sPort = "";

    if (nSkip > 3)
        sProtocol = sUri.substr(0, (nSkip - 3));
    else
        sProtocol = "";

    return true;
}


