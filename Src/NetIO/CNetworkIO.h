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

#include <asio.hpp>


#define DEFAULT_TCP_BUFFER_SIZE             4096


struct NetworkDataIoHeader_def
{
    char            m_StreamType[10];

    unsigned int    m_nDataLen;

    NetworkDataIoHeader_def()
    {
        memset(&m_StreamType, 0, sizeof(m_StreamType));

        m_nDataLen = 0;
    }
};


class CNetworkIO
{
  protected:

    std::string                                     m_sURI;

    unsigned int                                    m_nPort;

    asio::io_context                                m_ioContext;

    asio::ip::tcp::socket                           m_netSocket;

    asio::ip::tcp::endpoint                         *m_pNetEndPoint;

    bool                                            m_bConnected;

    char                                            *m_pDataBuffer;

    unsigned int                                    m_nBufferSize;

    std::string                                     m_sLastError;

  public:

    CNetworkIO() :
        m_nPort(0),
        m_pNetEndPoint(nullptr),
        m_bConnected(false),
        m_netSocket(m_ioContext),
        m_pDataBuffer(nullptr),
        m_nBufferSize(0)
    {
        m_sURI = "";

        m_sLastError = "";
    }

    ~CNetworkIO()
    {
        close();

        freeBuffer();
    }

    bool allocBuffer(const unsigned nSize = DEFAULT_TCP_BUFFER_SIZE)
    {
        if (nSize < 2)
            return false;

        m_pDataBuffer = (char *) calloc(1, nSize);

        if (m_pDataBuffer == nullptr)
            return false;

        m_nBufferSize - nSize;

        return true;
    }

    bool freeBuffer()
    {
        if (m_pDataBuffer == nullptr)
            return false;

        delete m_pDataBuffer;

        m_pDataBuffer = nullptr;

        m_nBufferSize = 0;

        return true;
    }

    bool clearBuffer()
    {
        if (m_pDataBuffer == nullptr)
            return false;

        memset(&m_pDataBuffer, 0, sizeof(m_nBufferSize));

        return true;
    }

    bool setUri(const std::string &sURI)
    {
        if (sURI == "")
            return false;

        m_sURI = sURI;

        return true;
    }

    bool setPort(const unsigned int nPort)
    {
        if (nPort < 1)
            return false;

        m_nPort = nPort;

        return true;
    }

    bool open()
    {
        if (setUri == "")
            return false;

        if (m_nPort < 1)
            return false;

        if (m_bConnected == true || m_pTcpEndPoint != nullptr)
        {
            try
            {
                m_tcpSocket.close();
            }
            catch (...)
            { }

            if (m_pTcpEndPoint != nullptr)
            {
                delete m_pTcpEndPoint;

                m_pTcpEndPoint = nullptr;
            }

            m_bConnected = false;
        }

        m_pTcpEndPoint = new asio::ip::tcp::endpoint(asio::ip::address::from_string(m_sURI), m_nPort);

        if (m_pTcpEndPoint == nullptr)
        {
            return false;
        }

        try
        {
            m_tcpSocket.connect(*m_pTcpEndPoint);
        }
        catch (...)
        {
            return false;
        }

        m_bConnected = true;

        return true;
    }

    bool close()
    {
        try
        {
            m_tcpSocket.close();
        }
        catch (...)
        {
        }

        if (m_pTcpEndPoint != nullptr)
        {
            delete m_pTcpEndPoint;

            m_pTcpEndPoint = nullptr;
        }

        m_bConnected = false;

        return true;
    }

    bool isConnected()
    {
        return m_bConnected;
    }

    int read(void *pTarget, const unsigned int nLen)
    {
        if (pTarget == nullptr)
            return -1;

        if (nLen < 2)
            return -2;

        if (m_pTcpBuffer == nullptr)
            return -3;

        int readSize = 0;

        try
        {
            asio::error_code error;

            readSize = (int) asio::read(m_tcpSocket, asio::buffer(m_pTcpBuffer, nLen), error);

            m_sLastError = error.message();
        }
        catch (...)
        {
            return -5;
        }

        return readSize
    }

    int write(const void *pSource, const unsigned int nLen)
    {
        if (pSource == nullptr)
            return -1;

        if (nLen < 2)
            return -2;

        int nWriteSize = 0;

        try
        {
            void* pData = cmdList.data();

            std::size_t dataSize = (std::size_t) nLen;

            asio::const_buffer send_buffer = asio::buffer(pSource, dataSize);

            nWriteSize = (int) asio::write(m_tcpSocket, send_buffer);
        }
        catch (...)
        {
            return -5;
        }

        return nWriteSize;
    }

};



C_NETWORK_IO_H