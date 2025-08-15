//****************************************************************************
// FILE:    CServerIO.cpp
//
// DESC:    C++ Network server input/output class 
//
// AUTHOR:  Russ Barker
//


#include "CServerIO.h"



using namespace CNetworkIO;


//*
//* CUdpProcessingContext class defs
//*


void CUdpProcessingContext::run()
{
    m_ioContext.run();

    while (m_bExit == false)
    {
        if (m_pProcessMsgProc == nullptr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else
        {
            m_pProcessMsgProc(m_pUdpServer, m_inputMsg, m_outputMsg);

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}


//*
//* CUdpSession class defs
//*


// CUdpSession class

CUdpSession::CUdpSession
(
    asio::ip::udp::socket &socket,
    asio::ip::udp::endpoint &endpoint,
    std::string &sMsgType
) :
    m_socket(socket),
    m_endpoint(endpoint),
    m_sMsgType(sMsgType)
{
    m_sLastError.clear();
}


CUdpSession::~CUdpSession()
{
}


//asio::ip::udp::socket* CUdpSession::socketPtr()
//{
//    return &m_socket;
//}


bool CUdpSession::readMsgHeader(CNetMessageData& msgData)
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
        auto recv_buffer = asio::buffer(pData, nLen);

        //auto status = asio::read(m_socket, recv_buffer);
        auto status = m_socket.receive_from(recv_buffer, m_endpoint);

        msgData.releasePtr();

        bReleased = true;

        if (status < nLen)
        {
            return false;
        }

        msgData.decodeMsgHeader(m_sMsgType);
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


bool  CUdpSession::readMsgBody(CNetMessageData& msgData)
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
        auto recv_buffer = asio::buffer(pData, nLen);

        //auto status = asio::read(m_socket, recv_buffer);
        auto status = m_socket.receive_from(recv_buffer, m_endpoint);


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


bool CUdpSession::writeMsgData(CNetMessageData& msgData)
{
    std::scoped_lock lock(m_mutex);

    bool bReleased = false;

    auto nLen = msgData.getCurDataLen();

    if (nLen < 1)
    {
        return false;
    }

    // encode/format the msg header

    msgData.encodeMsgHeader(m_sMsgType);

    // write the message to the net

    auto pData = msgData.getDataPtr();

    if (pData == nullptr)
    {
        msgData.releasePtr();

        return false;
    }

    try
    {
        auto send_buffer = asio::buffer(pData, nLen);

        //auto status = asio::write(m_socket, send_buffer);
        auto status = m_socket.send_to(send_buffer, m_endpoint);

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


bool CUdpSession::readMsgData(CNetMessageData& msgData)
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


// CUdpServer class

CUdpServer::CUdpServer(eNetIoDirection eDir, const unsigned int nPort, const unsigned int nSize) :
    m_port(nPort),
    m_bufferSize(nSize),
    m_eIoDirection(eDir),
    m_pSession(nullptr),
    m_pEndpoint(nullptr),
    m_inputMsg(MsgHeaderLen_def),
    m_outputMsg(MsgHeaderLen_def),
    m_pProcessingContext(nullptr),
    m_bInitialized(false),
    m_bRunning(false)
{
    m_pProcessingContext = new CUdpProcessingContext(this, m_ioContext, m_inputMsg, m_outputMsg);

    if (m_pProcessingContext != nullptr)
    {
        m_pProcessingContext->setProcessMsgProc(ioMsgHandler);
    }

    m_srvrThread.setContext(m_pProcessingContext);
}


CUdpServer::~CUdpServer()
{
    if (m_ioContext.stopped() == false)
    {
        m_ioContext.stop();
    }
}


bool CUdpServer::setPort(const unsigned int nPort)
{
    if (m_bRunning == true)
        return false;

    m_port = nPort;

    return true;
}


bool CUdpServer::setBufferSize(const unsigned int nSize)
{
    if (m_bInitialized == true)
        return false;

    m_bufferSize = nSize;

    return true;
}


bool CUdpServer::setDataType(const std::string& sType)
{
    if (sType == "")
        return false;

    m_sDataType = sType;

    return true;
}


bool CUdpServer::setName(const std::string& sName)
{
    if (sName == "")
        return false;

    m_sSrvrName = sName;

    return true;
}


bool CUdpServer::initialize(const unsigned int nBufize)
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


bool CUdpServer::start()
{
    if (m_bRunning == true)
        return false;

    try 
    {
        m_pEndpoint = new asio::ip::udp::endpoint(asio::ip::udp::v4(), m_port);

        if (m_pEndpoint == nullptr)
        {
            return false;
        }

        m_pSocket = new asio::ip::udp::socket(m_ioContext, *m_pEndpoint);

        if (m_pSocket == nullptr)
        {
            return false;
        }

        m_pSession = new CUdpSession(*m_pSocket, *m_pEndpoint, m_sDataType);

        if (m_pSession == nullptr)
        {
            return false;
        }

        m_srvrThread.setContext(m_pProcessingContext);

        m_srvrThread.setName(m_sSrvrName + "_server");

        if (m_srvrThread.createThread() == false)
        {
            return false;
        }
    }
    catch (...) 
    {
        return false;
    }

    m_bRunning = true;

    return true;
}


bool CUdpServer::stop()
{
    if (m_bRunning == false)
        return true;

    try
    {
        m_ioContext.stop();

        if (m_srvrThread.isActive() == true)
        {
            if (m_pProcessingContext != nullptr)
            {
                m_pProcessingContext->stop();
            }

            m_srvrThread.stopThread();
        }
    }
    catch (...)
    {
        return false;
    }

    m_bRunning = false;

    return true;
}


bool CUdpServer::isRunning()
{
    return m_bRunning;
}


void CUdpServer::setRunning(bool bVal)
{
    m_bRunning = false;
}


int CUdpServer::readInputData(void* pBuff, const unsigned int nMax)
{
    if (pBuff == nullptr || nMax < 1)
    { 
        return -1;
    }

    size_t nCopyLen = (size_t)nMax;

    auto nMsgLen = m_inputMsg.getBodyLength();

    if (nMsgLen == 0)
    {
        return 0;
    }

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

    return (int) nCopyLen;
}


int CUdpServer::writeOutputData(const void* pBuff, const unsigned int nLen)
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


bool CUdpServer::sendOutput()
{
    m_outputMsg.setUpdated(true);

    return true;
}


bool CUdpServer::processInputMsg(CNetMessageData& inputMsg, CNetMessageData& outputMsg)
{
    // This is a placeholder for an input message processing function
    // for server I/O processing.  Override this function for your 
    // processing logic.

    return true;
}


void CUdpServer::ioMsgHandler(CUdpServer *pSrvr, CNetMessageData& inputMsg, CNetMessageData& outputMsg)
{
    try
    {
        if (pSrvr->m_pSession != nullptr)
        {
            auto pSocket = pSrvr->m_pSession->getSocket();

            if (pSocket != nullptr)
            {
                if (pSocket->is_open() == false)
                    return;

                asio::socket_base::keep_alive option(true);

                pSocket->set_option(option);

                // handle server session

                {
                    switch (pSrvr->m_eIoDirection)
                    {
                    case eNetIoDirection::eNetIoDirection_input:
                    {
                        // read input msg
                        //m_inputMsg.clearAll();
                        pSrvr->m_pSession->readMsgData(pSrvr->m_inputMsg);
                    }
                    break;

                    case eNetIoDirection::eNetIoDirection_output:
                        // if needed, write output msg
                        if (pSrvr->m_outputMsg.isUpdated() == true)
                        {
                            pSrvr->m_pSession->writeMsgData(pSrvr->m_outputMsg);
                            //m_outputMsg.clearAll();
                        }
                        break;

                    case eNetIoDirection::eNetIoDirection_IO:
                    {
                        // read input msg
                        //m_inputMsg.clearAll();
                        pSrvr->m_pSession->readMsgData(pSrvr->m_inputMsg);

                        // process input msg
                        if (pSrvr->processInputMsg(pSrvr->m_inputMsg, pSrvr->m_outputMsg) == false)
                            break;

                        // if needed, write output msg
                        if (pSrvr->m_outputMsg.isUpdated() == true)
                        {
                            pSrvr->m_pSession->writeMsgData(pSrvr->m_outputMsg);
                            //m_outputMsg.clearAll();
                        }
                    }
                    break;
                    }
                }
            }
        }
    }
    catch (...)
    {
        return;
    }

    return;   //bRet;
}


//*
//* CTcpServer class defs
//*


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
        asio::error_code error;

        error.clear();

        auto status = asio::read(m_socket, asio::buffer(pData, nLen), error);

        msgData.releasePtr();

        bReleased = true;

        if (error)
        {
            m_sLastError = error.message();

            return false;
        }

        m_sLastError.clear();

        if (status < nLen)
        {
            return false;
        }

        if (status >= sizeof(NetworkDataHeaderInfo_def))
        {
            // get msg info from header

            msgData.decodeMsgHeader(m_sMsgType);
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


bool  CTcpSession::readMsgBody(CNetMessageData& msgData)
{
    bool bReleased = false;

    auto nLen = msgData.getBodyLength();

    if (nLen < 1)
    {
        return false;
    }

    // read the message body

    msgData.clearMsgBody();

    auto pData = msgData.getBodyPtr();

    if (pData == nullptr)
    {
        msgData.releasePtr();
        return false;
    }

    try
    {
        asio::error_code error;

        error.clear();

        auto status = asio::read(m_socket, asio::buffer(pData, nLen), error);

        msgData.releasePtr();

        bReleased = true;

        if (error)
        {
            m_sLastError = error.message();

            return false;
        }

        m_sLastError.clear();

        msgData.setBodyLength(status);

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

    msgData.encodeMsgHeader(m_sMsgType);

    // write the message to the net

    auto pData = msgData.getDataPtr();

    if (pData == nullptr)
    {
        msgData.releasePtr();
        
        return false;
    }

    try
    {
        asio::error_code error;

        error.clear();

        auto status = asio::write(m_socket, asio::buffer(pData, nLen), error);

        msgData.releasePtr();

        bReleased = true;

        if (error)
        {
            m_sLastError = error.message();

            return false;
        }

        m_sLastError.clear();

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


bool CTcpSession::sendMsgData(CNetMessageData& msgData)
{
    std::scoped_lock lock(m_mutex);

    bool bReleased = false;

    auto nLen = msgData.getCurDataLen();

    if (nLen < 1)
    {
        return false;
    }

    // encode/format the msg header

    msgData.encodeMsgHeader(m_sMsgType);

    // write the message to the net

    auto pData = msgData.getDataPtr();

    if (pData == nullptr)
    {
        msgData.releasePtr();

        return false;
    }

    try
    {
        asio::error_code error;

        error.clear();

        auto status = m_socket.send(asio::buffer(pData, nLen), (asio::socket_base::message_flags) 0, error);

        msgData.releasePtr();

        bReleased = true;

        if (error)
        {
            m_sLastError = error.message();

            return false;
        }

        m_sLastError.clear();

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
    m_ctrlMsg(MsgHeaderLen_def),
    m_heartBeatInterval(0),
    m_bInitialized(false),
    m_bRunning(false),
    m_bTcpNoDelay(TCP_NO_DELAY_DEFAULT),
    m_bMultiMsgSession(TCP_SRVR_SESSION_LOOP_DEFAULT),
    m_bExitSession(false),
    m_activeSessions(0)
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
    {
        m_bufferSize = nBufize;
    }

    if (m_bufferSize < 1)
    {
        return false;
    }

    if (m_ctrlMsg.allocBuffer(8) == false)
    {
        return false;
    }

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

        // set flag to report aborted socket ops

        asio::socket_base::enable_connection_aborted option(true);
        
        m_pAcceptor->set_option(option);

        if (m_bTcpNoDelay == true)
        {
            LogDebugInfoMsg("setting TCP_KEEP_ALIVE and TCP_NO_DELAY");

            asio::socket_base::keep_alive keepAlive(true);

            m_pAcceptor->set_option(keepAlive);

            asio::ip::tcp::no_delay noDelay(true);

            m_pAcceptor->set_option(noDelay);
        }

        // start accepting socket connections from clients

        acceptConnection();

        m_srvrThread.setContext(&m_ioContext);

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

    m_bExitSession = true;

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


bool CTcpServer::acceptConnection()
{
    if (m_pAcceptor == nullptr)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(5));

        acceptConnection(); // Accept next connection

        return false;
    }

    LogDebugInfoMsg("accepting TCP connection");

    m_pAcceptor->async_accept
    (
        [this](const asio::error_code& acceptError, asio::ip::tcp::socket socket)
        {
            if (!acceptError)
            {
                m_activeSessions++;

                if (m_heartBeatInterval > 0)
                {
                    // update timeout interval start
                    m_heartBeatTimestamp = std::chrono::system_clock::now();
                }

                bool bExitReceivedFromClient = false;

#if 0
                if (m_bTcpNoDelay == true)
                {
                    LogDebugInfoMsg("setting TCP_KEEP_ALIVE and TCP_NO_DELAY");

                    asio::socket_base::keep_alive keepAlive(true);

                    socket.set_option(keepAlive);

                    asio::ip::tcp::no_delay noDelay(true);

                    socket.set_option(noDelay);
                }
#endif

                if (socket.is_open() == false)
                { 
                    LogDebugInfoMsg("TCP session ended - not open");

                    m_activeSessions--;

                    acceptConnection(); // Accept next connection

                    return;
                }

                auto pSession = new CTcpSession(socket, m_sDataType);

                if (pSession != nullptr)
                {
                    LogDebugInfoMsg("created new TcpSession class");

                    // handle server session

                    bool bSessionLoop = false;

                    if (m_bMultiMsgSession == true)
                    {
                        bSessionLoop = true;
                        m_bExitSession = false;
                    }

                    asio::error_code ioError;

                    do
                    {
                        ioError.clear();

                        auto bytesAvail = socket.available(ioError);

                        if (ioError)
                        {
                            LogDebugInfoMsg("error getting TCP dataLen, ec: {}", ioError.message());
                            bSessionLoop = false;
                            m_bExitSession = true;
                            break;
                        }

                        switch (m_eIoDirection)
                        {
                        case eNetIoDirection::eNetIoDirection_input:
                            {
                                // check for msg timeout
                                if (m_heartBeatInterval > 0)
                                {
                                    auto now = std::chrono::system_clock::now();

                                    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_heartBeatTimestamp);

                                    if (dur.count() > m_heartBeatInterval)
                                    {
                                        bSessionLoop = false;
                                        m_bExitSession = true;
                                        break;
                                    }
                                }

                                // read input msg
                                //m_inputMsg.clearAll();

                                auto status = pSession->readMsgData(m_inputMsg);
                                if (status == true)
                                {
                                    if (m_heartBeatInterval > 0)
                                    {
                                        // update timeout interval start
                                        m_heartBeatTimestamp = std::chrono::system_clock::now();
                                    }

                                    if (m_inputMsg.compareMsgData("exit", 4) == true)
                                    {
                                        // msg client has 'exit'ed the session
                                        LogDebugInfoMsg("client 'exit' msg received - ending session");
                                        bExitReceivedFromClient = true;
                                        bSessionLoop = false;
                                        m_bExitSession = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    LogDebugInfoMsg("error reading TCP data, ec: {} - ending session", pSession->getLastError());
                                    bSessionLoop = false;
                                    m_bExitSession = true;
                                }
                            }
                            break;

                        case eNetIoDirection::eNetIoDirection_output:
                            {
                                if (m_heartBeatInterval > 0)
                                {
                                    auto now = std::chrono::system_clock::now();

                                    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_heartBeatTimestamp);

                                    if (dur.count() > m_heartBeatInterval)
                                    {
                                        // send heatbeat

                                        m_ctrlMsg.setMsgData("beat", 4);

                                        if (pSession->sendMsgData(m_ctrlMsg) == false)
                                        {
                                            LogDebugInfoMsg("error sebding TCP heartbeat, ec: {} - ending session", pSession->getLastError());
                                            bSessionLoop = false;
                                            m_bExitSession = true;
                                            break;
                                        }

                                        // read 'ack' reply

                                        if (pSession->readMsgData(m_ctrlMsg) == false)
                                        {
                                            LogDebugInfoMsg("error reading TCP heartbeat, ec: {} - ending session", pSession->getLastError());
                                            bSessionLoop = false;
                                            m_bExitSession = true;
                                            break;
                                        }

                                        if (m_ctrlMsg.compareMsgData("ack", 3) == false)
                                        {
                                            LogDebugInfoMsg("error TCP data != 'ack - ending session'");
                                            bSessionLoop = false;
                                            m_bExitSession = true;
                                            break;
                                        }
                                    }
                                }

                                if (bytesAvail > 0)
                                {
                                    auto status = pSession->readMsgData(m_ctrlMsg);
                                    if (status == true)
                                    {
                                        if (m_ctrlMsg.compareMsgData("exit", 4) == true)
                                        {
                                            // msg client has 'exit'ed the session
                                            LogDebugInfoMsg("client 'exit' msg received - ending session");
                                            bExitReceivedFromClient = true;
                                            bSessionLoop = false;
                                            m_bExitSession = true;
                                            break;
                                        }
                                    }
                                }

                                // if needed, write output msg
                                if (m_outputMsg.isUpdated() == true)
                                {
                                    auto nLen = m_outputMsg.getBodyLength();

                                    if (m_bMultiMsgSession == true)
                                    {
                                        LogDebugInfoMsg("sending TCP data, len: {}", nLen);

                                        if (pSession->sendMsgData(m_outputMsg) == false)
                                        {
                                            LogDebugInfoMsg("error sending TCP data, ec: {} - ending session", pSession->getLastError());
                                            bSessionLoop = false;
                                            m_bExitSession = true;
                                            break;
                                        }
                                    }
                                    else
                                    { 
                                        LogDebugInfoMsg("writing TCP data, len: {}", nLen);

                                        auto status = pSession->writeMsgData(m_outputMsg);
                                        if (status == false)
                                        {
                                            LogDebugInfoMsg("error writing TCP data, ec: {} - ending session", pSession->getLastError());
                                            bSessionLoop = false;
                                            m_bExitSession = true;
                                            break;
                                        }
                                    }

                                    m_outputMsg.setUpdated(false);
                                }
                                //else
                                //{
                                //    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                //}
                            }
                            break;

                        case eNetIoDirection::eNetIoDirection_IO:
                            {
                                // check for msg timeout
                                if (m_heartBeatInterval > 0)
                                {
                                    auto now = std::chrono::system_clock::now();

                                    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_heartBeatTimestamp);

                                    if (dur.count() > m_heartBeatInterval)
                                    {
                                        bSessionLoop = false;
                                        m_bExitSession = true;
                                        break;
                                    }
                                }

                                // read input msg

                                if (pSession->readMsgData(m_inputMsg) == true)
                                {
                                    if (m_heartBeatInterval > 0)
                                    {
                                        // update timeout interval start
                                        m_heartBeatTimestamp = std::chrono::system_clock::now();
                                    }

                                    if (m_inputMsg.compareMsgData("exit", 4) == true)
                                    {
                                        // msg client has 'exit'ed the session
                                        LogDebugInfoMsg("client 'exit' msg received - ending session");
                                        bExitReceivedFromClient = true;
                                        bSessionLoop = false;
                                        m_bExitSession = true;
                                        break;
                                    }
                                }
                                else
                                {
                                    LogDebugInfoMsg("error reading TCP data, ec: {} - ending session", pSession->getLastError());
                                    bSessionLoop = false;
                                    m_bExitSession = true;
                                    break;
                                }

                                // process input msg
                                if (processInputMsg(m_inputMsg, m_outputMsg) == false)
                                { 
                                    LogDebugInfoMsg("error writing TCP data, ec: {} - ending session", pSession->getLastError());
                                    bSessionLoop = false;
                                    m_bExitSession = true;
                                    break;
                                }

                                // if needed, write output msg
                                if (m_outputMsg.isUpdated() == true)
                                {
                                    if (m_bMultiMsgSession == true)
                                    {
                                        if (pSession->sendMsgData(m_outputMsg) == false)
                                        {
                                            LogDebugInfoMsg("error sending TCP data, ec: {} - ending session", pSession->getLastError());
                                            bSessionLoop = false;
                                            m_bExitSession = true;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        if (pSession->writeMsgData(m_outputMsg) == false)
                                        {
                                            LogDebugInfoMsg("error writing TCP data, ec: {} - ending session", pSession->getLastError());
                                            bSessionLoop = false;
                                            m_bExitSession = true;
                                            break;
                                        }
                                    }
                                }
                            }
                            break;
                        }

                        if (m_bMultiMsgSession == true)
                        { 
                            if (socket.is_open() == false)
                            {
                                LogDebugInfoMsg("TCP socket closed - exiting session");
                                // end of server session
                                bSessionLoop = false;
                                m_bExitSession = true;
                            }
                            //else
                            //{
                            //    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                            //}
                        }

                        if (m_bExitSession == true)
                        {
                            bSessionLoop = false;
                            break;
                        }

                        if (getRemoteAddress(socket) == "")
                        {
                            LogDebugInfoMsg("no connected endpoint");
                            bSessionLoop = false;
                            break;
                        }
                    } 
                    while (bSessionLoop == true);

                    // send session exit msg

                    if (bExitReceivedFromClient == false)
                    {
                        LogDebugInfoMsg("sending session exit msg to client");
 
                        m_ctrlMsg.setMsgData("exit", 4);

                        if (m_bMultiMsgSession == true)
                        {
                            pSession->sendMsgData(m_ctrlMsg);
                        }
                        else
                        {
                            pSession->writeMsgData(m_ctrlMsg);
                        }
                    }

                    LogDebugInfoMsg("deleting TcpSession class");

                    delete pSession;

                    pSession = nullptr;
                }

                try
                {
                    socket.shutdown(asio::ip::tcp::socket::shutdown_send);
                }
                catch (...)
                {
                }

                try
                {
                    socket.close();
                }
                catch (...)
                {
                }

                m_activeSessions--;
            }

            LogDebugInfoMsg("ending TCP connection");

            acceptConnection(); // Accept next connection
        }
    );

    return true;   //bRet;
}


