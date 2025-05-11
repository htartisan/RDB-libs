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
        if (x >= (unsigned int)sType.length())
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

    if (m_pDataBuffer != nullptr)
    {
        free(m_pDataBuffer);
        m_pDataBuffer = nullptr;
    }

    m_pDataBuffer = (DataBytePtr_def) calloc(1, nSize);

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
    m_pBufferHeader = nullptr;

    m_nHeaderSize = 0;

    if (m_pDataBuffer == nullptr)
        return false;

    delete m_pDataBuffer;

    m_pDataBuffer = nullptr;

    m_nBufferSize = 0;
    m_nCurrDataLen = 0;

    return true;
}


bool CTcpClient::clearBuffer()
{
    if (m_pDataBuffer == nullptr)
        return false;

    memset((m_pDataBuffer + m_nHeaderSize), 0, (m_nBufferSize - m_nHeaderSize));

    m_nCurrDataLen = 0;

    return true;
}


bool CTcpClient::setBuffer(const void* pSource, const unsigned int nLen)
{
    if (pSource == nullptr || nLen < 1)
        return false;

    if ((nLen + m_nHeaderSize) > m_nBufferSize)
        return false;

    memcpy((m_pDataBuffer + m_nHeaderSize), pSource, nLen);

    m_nCurrDataLen = nLen;

    int nFillLen = (m_nBufferSize - nLen);

    if (nFillLen > 0)
    {
        memset((m_pDataBuffer + (m_nHeaderSize + nLen)), 0, nFillLen);
    }

    return true;
}


bool CTcpClient::appendBuffer(const void *pSource, const unsigned int nLen)
{
    if (pSource == nullptr || nLen < 1)
        return false;

    if ((nLen + (m_nHeaderSize + m_nCurrDataLen))  > m_nBufferSize)
        return false;

    memcpy((m_pDataBuffer + (m_nHeaderSize + m_nCurrDataLen)), pSource, nLen);

    m_nCurrDataLen += nLen;

    return true;
}


unsigned int CTcpClient::getCurrdataLen()
{
    return m_nCurrDataLen;
}


bool CTcpClient::getBuffer(void *pTarget, const unsigned int nLen, const unsigned int nStartingAt)
{
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

    return (m_pDataBuffer + m_nHeaderSize);
}


bool CTcpClient::setDataSize(const unsigned int nLen)
{
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

    try
    {
        m_pNetResolver = new asio::ip::tcp::resolver(m_ioContext);
    }
    catch (...)
    {
        return false;
    }

    if (m_pNetResolver == nullptr)
    {
        return false;
    }

    try
    {
        m_netEndPoints = m_pNetResolver->resolve(m_sURI.c_str(), m_sPort.c_str());
    }
    catch (...)
    {
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
    m_bConnected = m_netSocket.is_open();

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

    unsigned int nPayloadSize = (readSize - m_nHeaderSize);

    if (pTarget != nullptr && nLen > 0)
    {
        if (nPayloadSize < nLen)
            memcpy(pTarget, (m_pDataBuffer + m_nHeaderSize), nPayloadSize);
        else
            memcpy(pTarget, (m_pDataBuffer + m_nHeaderSize), nLen);
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

        asio::const_buffer send_buffer = asio::buffer(m_pDataBuffer, dataSize);

        nWriteSize = (int) asio::write(m_netSocket, send_buffer);

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


bool CNetMessageData::allocBuffer(unsigned int nBufSize)
{
    if (nBufSize < 1)
        return false;

    if (m_pData != nullptr)
    {
        free(m_pData);

        m_pData = nullptr;
    }

    m_pData = (char *) malloc(nBufSize);

    if (m_pData == nullptr)
        return false;

    memset(m_pData, 0, (nBufSize));

    m_nMaxDataSize = nBufSize;

    return true;
}


void CNetMessageData::clear()
{
    m_nDataLength = 0;

    if (m_pData == nullptr)
        return;

    memset(m_pData, 0, m_nMaxDataSize);
}


const char* CNetMessageData::getDataPtr() const
{ 
    return m_pData;
}


char* CNetMessageData::getDataPtr()
{ 
    return m_pData;
}


std::size_t CNetMessageData::getDataMax() const
{
    return (m_nMaxDataSize);
}


std::size_t CNetMessageData::getMsgLength() const
{ 
    return (m_nDataLength + m_nHeaderLength);
}


const char* CNetMessageData::getBodyPtr() const
{ 
    return (m_pData + m_nHeaderLength);
}


char* CNetMessageData::getBodyPtr()
{ 
    return (m_pData + m_nHeaderLength);
}


std::size_t CNetMessageData::getBodyMax() const
{
    return (m_nMaxDataSize - m_nHeaderLength);
}


std::size_t CNetMessageData::getBodyLength() const
{ 
    return (m_nDataLength);
}


void CNetMessageData::setBodyLength(const std::size_t nLen)
{
    m_nDataLength = (unsigned int) nLen;
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
    m_msgData.m_nDataLength = 0;

    if (m_msgData.m_pData == nullptr)
        return false;

    NetworkDataHeaderInfo_def *pMsgHeader = (NetworkDataHeaderInfo_def *) m_msgData.m_pData;
        
    if (pMsgHeader->m_headerMarker != 0xFFFF)
        return false;

    if (pMsgHeader->m_nDataLen > m_msgData.m_nMaxDataSize)
        return false;

    m_msgData.m_nDataLength = (size_t) (pMsgHeader->m_nDataLen);

    return true;
}


bool CNetMessageHandler::encodeMsgHeader(const std::string& sType)
{
    if (m_msgData.m_pData == nullptr)
        return false;

    NetworkDataHeaderInfo_def* pMsgHeader = (NetworkDataHeaderInfo_def*)m_msgData.m_pData;

    pMsgHeader->initialize(sType.c_str(), (unsigned int) m_msgData.m_nDataLength);

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
    auto pData = msgData.getDataPtr();

    if (pData == nullptr)
        return false;

    auto nLen = msgData.m_nHeaderLength;

    if (nLen < 1)
        return false;

    try
    {
#ifdef USE_ASIO_ASYNC_READ
        auto self(shared_from_this());

        asio::async_read
        (
            m_socket,
            asio::buffer(pData, nLen),
            [this, self](const asio::error_code& error, std::size_t length)
            {
                if (error)
                {
                    return false;
                }

                return true;
            }
        );
#else
        auto status = asio::read(m_socket, asio::buffer(pData, nLen));

        if (status < nLen)
        {
            return false;
        }
#endif

        CNetMessageHandler msgHandler(msgData);

        msgHandler.decodeMsgHeader(m_sMsgType);
    }
    catch (std::exception e)
    {
        std::string err = e.what();
        return false;
    }
    catch (...)
    {
        return false;
    }

    return true;
}


bool  CTcpSession::readMsgBody(CNetMessageData& msgData)
{
    auto pData = msgData.getBodyPtr();

    if (pData == nullptr)
        return false;

    auto nLen = msgData.getBodyLength();

    if (nLen < 1)
        return false;

    try
    {
#ifdef USE_ASIO_ASYNC_READ
        auto self(shared_from_this());

        asio::async_read
        (
            m_socket,
            asio::buffer(pData, nLen),
            [this, self](const asio::error_code& error, std::size_t length)
            {
                if (error)
                {
                    return false;
                }

                return true;
            }
        );
#else
        auto status = asio::read(m_socket, asio::buffer(pData, nLen));

        if (status < nLen)
        {
            return false;
        }
#endif
    }
    catch (std::exception e)
    {
        std::string err = e.what();
        return false;
    }
    catch (...)
    {
        return false;
    }

    return true;
}


bool CTcpSession::writeMsgData(CNetMessageData &msgData)
{
    auto pData = msgData.getDataPtr();

    if (pData == nullptr)
        return false;

    auto nLen = msgData.getMsgLength();

    if (nLen < 1)
        return false;

    CNetMessageHandler  msgHandler(msgData);

    msgHandler.encodeMsgHeader(m_sMsgType);

    auto nDataSize = msgData.getBodyLength();

    if (nDataSize > 0)
    {
        ;
    }

    try
    {
#ifdef USE_ASIO_ASYNC_WRIRE
        auto self(shared_from_this());

        asio::async_write
        (
            m_socket,
            asio::buffer(pData, nLen),
            [this, self](const asio::error_code& error, std::size_t length)
            {
                if (error)
                {
                    return false;
                }

                m_outputMsg.clear();

                return true;
            }
        );
#else
        auto status = asio::write(m_socket, asio::buffer(pData, nLen));

        if (status < nLen)
        {
            return false;
        }

#endif
    }
    catch (std::exception e)
    {
        std::string err = e.what();
        return false;
    }
    catch (...)
    {
        return false;
    }

    return true;
}


bool CTcpSession::readMsgData(CNetMessageData &msgData)
{

    if (readMsgHeader(msgData) == true)
    {
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


void CTcpServer::setPort(const unsigned int nPort)
{
    m_port = nPort;
}


void CTcpServer::setBufferSize(const unsigned int nSize)
{
    m_bufferSize = nSize;
}


void CTcpServer::setDataType(const std::string& sType)
{
    m_sDataType = sType;
}


void CTcpServer::setName(const std::string& sName)
{
    m_sSrvrName = sName;
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
            return false;
        break;

    case eNetIoDirection::eNetIoDirection_output:
        if (m_outputMsg.allocBuffer(nBufize) == false)
            return false;
        break;

    case eNetIoDirection::eNetIoDirection_IO:
        if (m_inputMsg.allocBuffer(nBufize) == false)
            return false;
        if (m_outputMsg.allocBuffer(nBufize) == false)
            return false;
        break;

    default:
        return false;
    }

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
                                m_inputMsg.clear();
                                pSession->readMsgData(m_inputMsg);
                            }
                            break;

                        case eNetIoDirection::eNetIoDirection_output:
                            // if needed, write output msg
                            if (m_outputMsg.isUpdated() == true)
                            { 
                                pSession->writeMsgData(m_outputMsg);
                                m_outputMsg.clear();
                            }
                            break;

                        case eNetIoDirection::eNetIoDirection_IO:
                            {
                                // read input msg
                                m_inputMsg.clear();
                                pSession->readMsgData(m_inputMsg);

                                // process input msg
                                if (processInputMsg(m_inputMsg, m_outputMsg) == false)
                                    break;
                            
                                // if needed, write output msg
                                if (m_outputMsg.isUpdated() == true)
                                {
                                    pSession->writeMsgData(m_outputMsg);
                                    m_outputMsg.clear();
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
        return -1;

    auto pData = m_inputMsg.getBodyPtr();

    if (pData == nullptr)
        return -2;

    size_t nCopyLen = (size_t)nMax;

    auto nMsgLen = m_inputMsg.getBodyLength();

    if (nMsgLen < nCopyLen)
        nCopyLen = (size_t)nMsgLen;

    memcpy(pBuff, pData, nCopyLen);

    m_inputMsg.clear();

    return (int)nCopyLen;
}


int CTcpServer::writeOutputData(const void* pBuff, const unsigned int nLen)
{
    if (pBuff == nullptr || nLen < 1)
        return -1;

    size_t nCopyLen = (size_t)nLen;

    auto nMsgLen = m_outputMsg.getBodyLength();

    if (nMsgLen < nCopyLen)
    {
        //nCopyLen = (size_t) nMsgLen;
        m_outputMsg.allocBuffer((unsigned int)nCopyLen);
    }

    auto pData = m_outputMsg.getBodyPtr();

    if (pData == nullptr)
        return false;

    memcpy(pData, pBuff, nCopyLen);

    m_outputMsg.setBodyLength(nCopyLen);

    return (int)nCopyLen;
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

bool CNetworkIO::parseServerAndPort(const std::string& sUri, std::string& sServer, std::string sPort)
{
    if (sUri == "")
    {
        return false;
    }

    size_t nPos = sUri.find(":");

    if (nPos == std::string::npos)
    {
        sServer = sUri;
        sPort = "";

        return true;
    }

    size_t nSkip = 0;

    if (sUri[nPos + 1] == '/')
    {
        nSkip = (nPos + 1);

        auto sTmp = sUri.substr(nSkip);

        nPos = sTmp.find(":");
    }

    sServer = sUri.substr(0, (nSkip + nPos));
    sPort = sUri.substr((nSkip + nPos + 1));

    return true;
}


