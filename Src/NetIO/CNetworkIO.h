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

#define NET_STREAM_TYPE_LEN                 12


struct NetworkDataHeaderInfo_def
{
    char        m_StreamType[NET_STREAM_TYPE_LEN];

    uint32_t    m_nDataLen;
};


class CNetworkDataIoHeader
{
  protected:

    NetworkDataHeaderInfo_def   m_data;

  public:

    CNetworkDataIoHeader()
    {
        clear();
    }

    CNetworkDataIoHeader(const std::string &sType, const unsigned int nLen = 0)
    {
        clear();

        setType(sType);

        setLen(nLen);
    }

    void clear()
    {
        memset(&m_data, 0, sizeof(m_data));
    }

    void setType(const std::string &sType)
    {
        for (unsigned int x = 0; x < NET_STREAM_TYPE_LEN; x++)
        {
            if (x >= (unsigned int) sType.length())
                break;

            m_data.m_StreamType[x] = sType[x];
        }
    }

    std::string getType()
    {
        std::string sOut = "";

        sOut.assign(m_data.m_StreamType, NET_STREAM_TYPE_LEN);

        return sOut;
    }

    void setLen(const unsigned int nLen)
    {
        m_data.m_nDataLen = nLen;
    }

    unsigned int getLen()
    {
        return m_data.m_nDataLen;
    }

    char * getdataPtr()
    {
        return (char *) &m_data;
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

    unsigned int                                    m_nCurrBufLen;

    std::string                                     m_sLastError;

  public:

    CNetworkIO() :
        m_nPort(0),
        m_pNetEndPoint(nullptr),
        m_bConnected(false),
        m_netSocket(m_ioContext),
        m_pDataBuffer(nullptr),
        m_nBufferSize(0),
        m_nCurrBufLen(0)
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

        if (m_pDataBuffer != nullptr)
        {
            free(m_pDataBuffer);
            m_pDataBuffer = nullptr;
        }

        m_pDataBuffer = (char *) calloc(1, nSize);

        if (m_pDataBuffer == nullptr)
            return false;

        m_nBufferSize = nSize;

        m_nCurrBufLen = 0;

        return true;
    }

    bool freeBuffer()
    {
        if (m_pDataBuffer == nullptr)
            return false;

        delete m_pDataBuffer;

        m_pDataBuffer = nullptr;

        m_nBufferSize = 0;
        m_nCurrBufLen = 0;

        return true;
    }

    bool clearBuffer()
    {
        if (m_pDataBuffer == nullptr)
            return false;

        memset(&m_pDataBuffer, 0, sizeof(m_nBufferSize));

        m_nCurrBufLen = 0;

        return true;
    }

    bool appendBuffer(const void *pSource, const unsigned int nLen)
    {
        if (pSource == nullptr || nLen < 1)
            return false;

        if ((nLen + m_nCurrBufLen)  > m_nBufferSize)
            return false;

        memcpy((m_pDataBuffer + m_nCurrBufLen), pSource, nLen);

        return true;
    }

    bool getBuffer(void *pTarget, const unsigned int nLen, const unsigned int nStartingAt = 0)
    {
        if (pTarget == nullptr || nLen < 1)
            return false;

        if (nLen > m_nBufferSize)
            return false;

        memcpy(pTarget, (m_pDataBuffer + nStartingAt), nLen);

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

    bool close()
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

    bool isConnected()
    {
        return m_bConnected;
    }

    int read(void *pTarget = nullptr, const unsigned int nLen = 0)
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
        }
        catch (...)
        {
            return -5;
        }

        if (pTarget != nullptr && nLen > 0)
        {
            memcpy(pTarget, m_pDataBuffer, readSize);
        }

        return readSize;
    }

    int write(const void *pSource = nullptr, const unsigned int nLen = 0)
    {
        if (m_pDataBuffer == nullptr && (pSource == nullptr || nLen < 1))
            return -1;

        if (nLen > m_nBufferSize)
            return -2;        

        if (pSource != nullptr && nLen > 0)
        {
            memcpy(m_pDataBuffer, pSource, nLen);
        }

        int nWriteSize = 0;

        try
        {
            void *pData = nullptr;

            if (pSource != nullptr)
                pData = (void *) pSource;
            else
                pData = (void *) m_pDataBuffer;

            std::size_t dataSize = 0;

            if (nLen > 0)
                dataSize = (std::size_t) nLen;
            else
                dataSize = (std::size_t) m_nBufferSize;

            asio::const_buffer send_buffer = asio::buffer(pData, dataSize);

            nWriteSize = (int) asio::write(m_netSocket, send_buffer);
        }
        catch (...)
        {
            return -5;
        }

        return nWriteSize;
    }

};



C_NETWORK_IO_H