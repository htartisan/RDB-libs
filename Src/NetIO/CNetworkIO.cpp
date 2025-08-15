//****************************************************************************
// FILE:    CClientIO.cpp
//
// DESC:    C++ Network client input/output class 
//
// AUTHOR:  Russ Barker
//



#ifdef WINDOWS
#include <Windows.h>
#endif

#include "CNetworkIO.h"


using namespace CNetworkIO;



//*
//* utility functions
//*

bool parseServerAndPort(const std::string& sUri, std::string& sServer, std::string& sPort, std::string& sProtocol)
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



bool parseUriParams(const std::string& sPrmsStr, UriParamList_def& prmList)
{
    if (sPrmsStr == "")
    {
        return false;
    }

    std::string sParamName = "";
    std::string sParamValue = "";

    unsigned int nParamStrLen = (unsigned int) sPrmsStr.length();

    // first look for HTTP stype params (starting with '?')

    auto nParamStart = sPrmsStr.find("?");

    if (nParamStart != std::string::npos)
    {
        // found question mark

        nParamStart++;

        if (nParamStart >= nParamStrLen)
        {
            // no param data

            return false;
        }

        while (nParamStart < nParamStrLen)
        {
            auto nEqualSign = sPrmsStr.find("=", nParamStart);

            if (nEqualSign == std::string::npos)
            {
                // no equal sign found 

                sParamName = sPrmsStr.substr(nParamStart);

                if (sParamName == "")
                {
                    // no param name

                    return false;
                }

                // save just the param name

                UriParamData_def newParam(sParamName);

                prmList.push_back(newParam);

                return true;
            }

            sParamName = sPrmsStr.substr(nParamStart, nEqualSign);

            nParamStart = (nEqualSign + 1);

            // find start of next param

            auto nAndlSign = sPrmsStr.find("&", nParamStart);

            if (nAndlSign == std::string::npos)
            {
                // no 'next param' symbole 

                sParamValue = sPrmsStr.substr(nParamStart);

                if (sParamValue == "")
                {
                    // no param value

                    return false;
                }

                // save param, and exit

                UriParamData_def newParam(sParamName, sParamValue);

                prmList.push_back(newParam);

                return true;
            }

            // save param, and look for next param

            sParamValue = sPrmsStr.substr(nParamStart, nAndlSign);

            nParamStart = (nAndlSign + 1);

            UriParamData_def newParam(sParamName, sParamValue);

            prmList.push_back(newParam);
        }

        return true;
    }

    // If not found, look for XML style params

    auto nOpenBrace = sPrmsStr.find("{");

    if (nOpenBrace == std::string::npos)
    {
        // no start of XML

        return false;
    }

    auto nCloseBrace = sPrmsStr.rfind("}");

    if (nCloseBrace == std::string::npos)
    {
        // no end of XML

        return false;
    }

    nParamStart = (nOpenBrace + 1);

    if (nParamStart >= nCloseBrace)
    {
        // no XML data

        return false;
    }

    while (nParamStart < nCloseBrace)
    { 
        // find param name

        {
            auto nOpenQuote = sPrmsStr.find("\"", nParamStart);

            if (nOpenQuote == std::string::npos)
            {
                // no open quote

                return false;
            }

            nParamStart = (nOpenQuote + 1);

            auto nCloseQuote = sPrmsStr.find("\"", nParamStart);

            if (nCloseQuote == std::string::npos)
            {
                // no close quote

                return false;
            }

            sParamName = sPrmsStr.substr(nParamStart, nCloseQuote);

            nParamStart = (nCloseQuote + 1);
        }

        auto nColon = sPrmsStr.find(":", nParamStart);

        if (nColon == std::string::npos)
        {
            // no colon symbol

            auto nComma = sPrmsStr.find(",", nParamStart);

            UriParamData_def newParam(sParamName);

            prmList.push_back(newParam);

            if (nComma == std::string::npos)
            {
                // no comma (no more params)

                return true;
            }

            nParamStart = (nComma + 1);

            continue;
        }

        nParamStart = (nColon + 1);

        // find param value

        {
            auto nOpenQuote = sPrmsStr.find("\"", nParamStart);

            if (nOpenQuote == std::string::npos)
            {
                // no open quote, treat as non-quoted value

                std::string sTmp = "";

                auto nComma = sPrmsStr.find(",", nParamStart);

                if (nComma == std::string::npos)
                {
                    sTmp = sPrmsStr.substr(nParamStart, nComma);
                }
                else
                { 
                    sTmp = sPrmsStr.substr(nParamStart);
                }

                for (unsigned int i = 0; i < sTmp.length(); i++)
                {
                    // skip any leading spaces

                    if (sTmp[i] > ' ')
                    {
                        sParamValue = sTmp.substr(i);

                        nParamStart += sTmp.length();

                        goto saveParam;
                    }
                }

                // no non-quoted value

                return false;
            }

            nParamStart = (nOpenQuote + 1);

            auto nCloseQuote = sPrmsStr.find("\"", nParamStart);

            if (nCloseQuote == std::string::npos)
            {
                // no close quote

                return false;
            }

            sParamValue = sPrmsStr.substr(nParamStart, nCloseQuote);

            nParamStart = (nCloseQuote + 1);
        }

      saveParam:

        UriParamData_def newParam(sParamName, sParamValue);

        prmList.push_back(newParam);

        auto nComma = sPrmsStr.find(",", nParamStart);

        if (nComma == std::string::npos)
        {
            break;
        }

        nParamStart = (nComma + 1);

        // continue looking for next param
    }

    return true;
}


//*
//* CNetMessageData utility class defs
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
    if (m_nDataLength == 0)
    {
        return 0;
    }

    if (m_nDataLength < m_nHeaderLength)
    {
        return 0;
    }

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


bool CNetMessageData::setMsgData(const void *pData, const unsigned int nLen)
{
    if (pData == nullptr || nLen < 1)
    {
        return false;
    }

    if (nLen > (m_nMaxDataSize - m_nHeaderLength))
    {
        return false;
    }

    std::scoped_lock lock(m_dataLock);

    memcpy((m_pData + m_nHeaderLength), pData, nLen);

    m_nDataLength = (unsigned int) (m_nHeaderLength + nLen);

    m_bUpdated = true;

    return true;
}


bool CNetMessageData::compareMsgData(const char* pData, const unsigned int nLen)
{
    if (pData == nullptr || nLen < 1)
    {
        return false;
    }

    if (nLen > (m_nMaxDataSize - m_nHeaderLength))
    {
        return false;
    }

    std::scoped_lock lock(m_dataLock);

    for (unsigned int x = 0; x < nLen; x++)
    {
        if (*(pData + x) != *(m_pData + m_nHeaderLength + x))
        {
            return false;
        }
    }

    return true;
}


bool CNetMessageData::decodeMsgHeader(const std::string& sType)
{
    setBodyLength(0);

    if (isBufferAllocated() == false)
        return false;

    NetworkDataHeaderInfo_def *pHeader = (NetworkDataHeaderInfo_def *) getDataPtr();
        
    if (pHeader->m_headerMarker != 0xFFFF)
    {
        releasePtr();
        return false;
    }

    if (pHeader->m_nDataLen >= (uint32_t) getMaxDataLen())
    {
        releasePtr();
        return false;
    }

    setBodyLength(pHeader->m_nDataLen);

    releasePtr();

    return true;
}


bool CNetMessageData::encodeMsgHeader(const std::string& sType)
{
    if (isBufferAllocated() == false)
        return false;

    NetworkDataHeaderInfo_def* pMsgHeader = (NetworkDataHeaderInfo_def*) getDataPtr();

    pMsgHeader->initialize(sType.c_str(), (unsigned int) getBodyLength());

    releasePtr();

    return true;
}



