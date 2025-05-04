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

#include <asio.hpp>


#define DEFAULT_TCP_BUFFER_SIZE             4096

#define NET_STREAM_TYPE_LEN                 10


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


// CTcpClient class


class CTcpClient
{
  protected:

    std::string                                     m_sURI;

    unsigned int                                    m_nPort;

    asio::io_context                                m_ioContext;

    asio::ip::tcp::socket                           m_netSocket;

    asio::ip::tcp::endpoint                         *m_pNetEndPoint;

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

    void setDataType(const std::string& sType);

    std::string getDataType();

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

    bool setPort(const unsigned int nPort);

    bool open();

    bool close();

    bool isConnected();

    int read(void *pTarget = nullptr, const unsigned int nLen = 0);

    int write(const void *pSource = nullptr, const unsigned int nLen = 0);

};


// CTcpServer class


#define MsgHeaderLen_def        sizeof(NetworkDataHeaderInfo_def)


struct CNetMessage
{
    char                    m_StreamType[NET_STREAM_TYPE_LEN];

    char                    *m_pData;

    unsigned int            m_nMaxDataSize;

    unsigned int            m_nDatLength;

    CNetMessage();

    bool allocBuffer(unsigned int nBufSize);

    void clear();

    const char* getDataPtr() const ;

    char* getDataPtr();

    std::size_t getDataMax() const ;

    std::size_t getMsgLength() const ;

    const char* getBodyPtr() const ;

    char* getBodyPtr(); 

    std::size_t getBodyMax() const ;

    std::size_t getBodyLength() const ;

    void setBodyLength(const std::size_t nLen);

    bool decodeMsgHeader();

    bool encodeMsgHeader();

};


class CTcpAcceptor;


class CTcpSession : 
    public std::enable_shared_from_this<CTcpSession>
{
    CTcpAcceptor                *m_pParent;

    asio::ip::tcp::socket       m_socket;

    CNetMessage                 m_inputMsg;
    CNetMessage                 m_outputMsg;

    eNetIoDirection             m_eDataDirection;

    unsigned int                m_nBufferize;

public:

    CTcpSession
        (
            CTcpAcceptor *pParent,
            eNetIoDirection eDir,
            asio::ip::tcp::socket socket,
            const unsigned int nBufSize = 0
        );

    ~CTcpSession();

    bool initialize(const unsigned int nBufize = 0);

    bool start();

    int readInputData(void* pBuff, const unsigned int nMax);

    int writeOutputData(void* pBuff, const unsigned int nLen);

private:

    bool readMsgHeader();

    bool  readMsgBody();

    bool writeMsgData();

};


typedef std::shared_ptr<CTcpSession>                TcpSessionPtr_def;

typedef std::vector<TcpSessionPtr_def>              TcpSessionList_def;


class CTcpAcceptor 
{
    eNetIoDirection                 m_eIoDirection;

    unsigned int                    m_nBufSize;

    asio::io_context&               m_ioContext;

    asio::ip::tcp::acceptor         m_acceptor;

    TcpSessionList_def              m_sessionList;

    bool                            m_bExit;

public:
    CTcpAcceptor
        (
            eNetIoDirection eDir,
            const unsigned int nBufSize,
            asio::io_context& io_context,
            const asio::ip::tcp::endpoint& endpoint
        );

    ~CTcpAcceptor();

    int readInputData(void* pBuff, const unsigned int nMax);

    int writeOutputData(void* pBuff, const unsigned int nLen);

    void removeSession(CTcpSession* pSession);

private:

    bool findSession(TcpSessionPtr_def pSession);

    void acceptConnection();

};


class CTcpServer
{
    unsigned int                m_port;

    unsigned int                m_bufferSize;

    eNetIoDirection             m_eIoDirection;

    CTcpAcceptor                *m_pAcceptor;

    asio::io_context            m_ioContext;


public:

    CTcpServer(eNetIoDirection eDir, const unsigned int nPort = 0, const unsigned int nSize = 0);

    ~CTcpServer();

    void setPort(const unsigned int nPort);

    void setBufferSize(const unsigned int nSize);

    bool run();

};


};  //  namespace CNetworkIO


#endif  //  C_NETWORK_IO_H

