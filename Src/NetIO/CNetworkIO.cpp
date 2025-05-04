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
    m_nPort(0),
    m_pNetEndPoint(nullptr),
    m_bConnected(false),
    m_netSocket(m_ioContext),
    m_pBufferHeader(nullptr),
    m_nHeaderSize(0),
    m_pDataBuffer(nullptr),
    m_nBufferSize(0),
    m_nCurrDataLen(0)
{
    m_sURI = "";

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


bool CTcpClient::setPort(const unsigned int nPort)
{
    if (nPort < 1)
        return false;

    m_nPort = nPort;

    return true;
}


bool CTcpClient::open()
{
    if (m_sURI == "")
        return false;

    if (m_nPort < 1)
        return false;

    if (m_bConnected == true || m_pNetEndPoint != nullptr)
    {
        try
        {
            m_netSocket.close();
        }
        catch (...)
        { }

        if (m_pNetEndPoint != nullptr)
        {
            delete m_pNetEndPoint;

            m_pNetEndPoint = nullptr;
        }

        m_bConnected = false;
    }

    m_pNetEndPoint = new asio::ip::tcp::endpoint(asio::ip::address::from_string(m_sURI), m_nPort);

    if (m_pNetEndPoint == nullptr)
    {
        return false;
    }

    try
    {
        m_netSocket.connect(*m_pNetEndPoint);
    }
    catch (...)
    {
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

    if (m_pNetEndPoint != nullptr)
    {
        delete m_pNetEndPoint;

        m_pNetEndPoint = nullptr;
    }

    m_bConnected = false;

    return true;
}


bool CTcpClient::isConnected()
{
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



// CTcpServer class

CNetMessage::CNetMessage() :
    m_pData(nullptr),
    m_nMaxDataSize(0),
    m_nDatLength(0)
{
        
}


bool CNetMessage::allocBuffer(unsigned int nBufSize)
{
    if (nBufSize < 1)
        return false;

    m_pData = (char *) malloc(nBufSize);

    if (m_pData == nullptr)
        return false;

    memset(m_pData, 0, (nBufSize));

    m_nMaxDataSize = nBufSize;

    return true;
}


void CNetMessage::clear()
{
    m_nDatLength = 0;

    if (m_pData == nullptr)
        return;

    //memset((m_pData + MsgHeaderLen_def), 0, (m_nMaxDataSize - MsgHeaderLen_def));
    memset(m_pData, 0, m_nMaxDataSize);
}


const char* CNetMessage::getDataPtr() const
{ 
    return m_pData;
}


char* CNetMessage::getDataPtr()
{ 
    return m_pData;
}


std::size_t CNetMessage::getDataMax() const
{
    return (m_nMaxDataSize);
}


std::size_t CNetMessage::getMsgLength() const
{ 
    return (m_nDatLength + MsgHeaderLen_def);
}


const char* CNetMessage::getBodyPtr() const
{ 
    return (m_pData + MsgHeaderLen_def);
}


char* CNetMessage::getBodyPtr()
{ 
    return (m_pData + MsgHeaderLen_def);
}


std::size_t CNetMessage::getBodyMax() const
{
    return (m_nMaxDataSize - MsgHeaderLen_def);
}


std::size_t CNetMessage::getBodyLength() const
{ 
    return (m_nDatLength);
}


void CNetMessage::setBodyLength(const std::size_t nLen)
{
    m_nDatLength = (unsigned int) nLen;
}


bool CNetMessage::decodeMsgHeader()
{
    m_nDatLength = 0;

    if (m_pData == nullptr)
        return false;

    NetworkDataHeaderInfo_def *pMsgHeader = (NetworkDataHeaderInfo_def *) m_pData;
        
    if (pMsgHeader->m_headerMarker != 0xFFFF)
        return false;

    if (pMsgHeader->m_nDataLen > m_nMaxDataSize)
        return false;

    m_nDatLength = (size_t) (pMsgHeader->m_nDataLen);

    return true;
}


bool CNetMessage::encodeMsgHeader()
{
    if (m_pData == nullptr)
        return false;

    NetworkDataHeaderInfo_def* pMsgHeader = (NetworkDataHeaderInfo_def*) m_pData;

    pMsgHeader->initialize(m_StreamType, (unsigned int) m_nDatLength);

    return true;
}


// CTcpSession class

CTcpSession::CTcpSession
    (
        CTcpAcceptor *pParent,
        eNetIoDirection eDir,
        asio::ip::tcp::socket socket,
        const unsigned int nBufSize
    ) :
    m_pParent(pParent),
    m_socket(std::move(socket)),
    m_nBufferize(nBufSize)
{

}


CTcpSession::~CTcpSession()
{
    if (m_pParent != nullptr)
    {
        m_pParent->removeSession(this);
    }
}


bool CTcpSession::initialize(const unsigned int nBufize)
{
    if (nBufize > 0)
        m_nBufferize = nBufize;

    if (m_nBufferize < 1)
        return false;

    switch (m_eDataDirection)
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


bool CTcpSession::start()
{
    bool bExit = false;

    while (bExit == false)
    { 
        switch (m_eDataDirection)
        { 
        case eNetIoDirection::eNetIoDirection_input:
            {
                if (readMsgHeader() == true)
                {
                    if (readMsgBody() == false)
                    {
                        bExit = true;
                    }
                }
                else
                {
                    bExit = true;
                }
            }
            break;

        case eNetIoDirection::eNetIoDirection_output:
            {
                if (writeMsgData() == false)
                {
                    bExit = true;
                }
            }
            break;

        default:
            return false;
        }
    }

    return true;
}


int CTcpSession::readInputData(void* pBuff, const unsigned int nMax)
{
    if (pBuff == nullptr || nMax < 1)
        return -1;

    auto pData = m_inputMsg.getBodyPtr();

    if (pData == nullptr)
        return -2;

    size_t nCopyLen = (size_t) nMax;

    auto nMsgLen = m_inputMsg.getBodyLength();

    if (nMsgLen < nCopyLen)
        nCopyLen = (size_t) nMsgLen;

    memcpy(pBuff, pData, nCopyLen);

    m_inputMsg.clear();

    return (int) nCopyLen;
}


int CTcpSession::writeOutputData(void* pBuff, const unsigned int nLen)
{
    if (pBuff == nullptr || nLen < 1)
        return -1;

    size_t nCopyLen = (size_t) nLen;

    auto nMsgLen = m_outputMsg.getBodyLength();

    if (nMsgLen < nCopyLen)
        nCopyLen = (size_t)nMsgLen;

    auto pData = m_outputMsg.getBodyPtr();

    if (pData == nullptr)
        return false;

    memcpy(pBuff, pData, nCopyLen);

    m_outputMsg.setBodyLength(nCopyLen);

    return (int) nCopyLen;
}


bool CTcpSession::readMsgHeader()
{
    auto self(shared_from_this());

    asio::async_read
    (
        m_socket, 
        asio::buffer(m_inputMsg.getDataPtr(), MsgHeaderLen_def),
        [this, self](const asio::error_code& error, std::size_t /*length*/) 
        {
            if (error) 
            {
                return false;
            }

            return true;
        }
    );

    m_inputMsg.decodeMsgHeader();

    return true;
}


bool  CTcpSession::readMsgBody()
{
    auto self(shared_from_this());

    asio::async_read
    (
        m_socket,
        asio::buffer(m_inputMsg.getBodyPtr(), m_inputMsg.getBodyLength()),
        [this, self](const asio::error_code& error, std::size_t /*length*/) 
        {
            if (error) 
            {
                return false;
            }

            return true;
        }
    );

    return true;
}


bool CTcpSession::writeMsgData()
{
    auto self(shared_from_this());

    m_outputMsg.encodeMsgHeader();

    asio::async_write
    (
        m_socket,
        asio::buffer(m_outputMsg.getDataPtr(), m_outputMsg.getMsgLength()),
        [this, self](const asio::error_code& error, std::size_t /*length*/) 
        {
            if (error) 
            {
                return false;
            }

            m_outputMsg.clear();

            return true;
        }
    );

    return true;
}


// CTcpAcceptor class

CTcpAcceptor::CTcpAcceptor
    (
        eNetIoDirection eDir,
        const unsigned int nBufSize,
        asio::io_context& io_context,
        const asio::ip::tcp::endpoint& endpoint
    ) :
    m_eIoDirection(eDir),
    m_nBufSize(nBufSize),
    m_ioContext(io_context), 
    m_acceptor(io_context, endpoint),
    m_bExit(false)
{
    acceptConnection();

    m_sessionList.clear();
}


CTcpAcceptor::~CTcpAcceptor()
{
    m_bExit = true;

    m_sessionList.clear();
}


int CTcpAcceptor::readInputData(void* pBuff, const unsigned int nMax)
{
    if (pBuff == nullptr || nMax < 0)
        return -1;

    int nCnt = 0;

    for (auto s = m_sessionList.begin(); s != m_sessionList.end(); s++)
    {
        if ((*s) != nullptr)
        {
            (*s)->readInputData(pBuff, nMax);

            nCnt++;
        }
    }

    return nCnt;
}


int CTcpAcceptor::writeOutputData(void* pBuff, const unsigned int nLen)
{
    if (pBuff == nullptr || nLen < 0)
        return -1;

    int nCnt = 0;

    for (auto s = m_sessionList.begin(); s != m_sessionList.end(); s++)
    {
        if ((*s) != nullptr)
        {
            (*s)->writeOutputData(pBuff, nLen);
        }

        nCnt++;
    }

    return nCnt;
}


void CTcpAcceptor::removeSession(CTcpSession* pSession)
{
    for (auto s = m_sessionList.begin(); s != m_sessionList.end(); s++)
    {
        if ((*s).get() == pSession)
        {
            m_sessionList.erase(s);
            return;
        }
    }
}


bool CTcpAcceptor::findSession(TcpSessionPtr_def pSession)
{
    for (auto s = m_sessionList.begin(); s != m_sessionList.end(); s++)
    {
        if ((*s) == pSession)
            return true;
    }

    return false;
}


void CTcpAcceptor::acceptConnection()
{
    m_acceptor.async_accept
    (
        [this](const asio::error_code& error, asio::ip::tcp::socket socket) 
        {
            if (!error) 
            {
                auto pSession = std::make_shared<CTcpSession>(this, m_eIoDirection, std::move(socket), m_nBufSize);

                if (pSession == nullptr)
                    return;

                if (findSession(pSession) == false)
                    m_sessionList.push_back(pSession);

                pSession->start();
            }
            else
            {

            }

            if (m_bExit == true)
                return;

            acceptConnection();
        }
    );
}


// CTcpServer class

CTcpServer::CTcpServer(eNetIoDirection eDir, const unsigned int nPort, const unsigned int nSize) :
    m_port(nPort),
    m_bufferSize(nSize),
    m_eIoDirection(eDir),
    m_pAcceptor(nullptr)
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


bool CTcpServer::run()
{
    try 
    {
        asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), m_port);

        m_pAcceptor = new CTcpAcceptor(m_eIoDirection, m_bufferSize, m_ioContext, endpoint);

        m_ioContext.run();
    }
    catch (...) 
    {
        if (m_pAcceptor != nullptr)
            delete m_pAcceptor;

        return false;
    }

    if (m_pAcceptor != nullptr)
        delete m_pAcceptor;

    return true;
}


