//**************************************************************************************************
//* FILE:		PocoHttpClient.cpp
//*
//* DESCRIP:	PocoHttpClient utility class
//*				This class impliments an HTTP client class
//*				using the Poco HTTP classes
//*

#ifndef _POCO_HTTP_CLIENT_H_
#define _POCO_HTTP_CLIENT_H_


#pragma warning( disable: 4081 )
#pragma warning( disable: 4101 )
#pragma warning( disable: 4251 )
#pragma warning( disable: 4996 )


#ifdef POCO_SSL
#define SUPPORT_POCO_SSL
#endif



#ifdef SUPPORT_POCO_SSL
#include <Poco/Net/HTTPSClientSession.h>
#include <cstring>
#endif
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPCredentials.h>
#include <Poco/Net/HTTPBasicCredentials.h>
#include <Poco/StreamCopier.h>
#include <Poco/NullStream.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#ifdef SUPPORT_POCO_SSL
#include "Poco/Net/SecureStreamSocket.h"
#endif

#include <string>
#include <iostream>



#ifndef CreateSemaphore
#ifdef UNICODE
#define CreateSemaphore  CreateSemaphoreW
#else
#define CreateSemaphore  CreateSemaphoreA
#endif // !UNICODE
#endif


#ifndef CreateProcess
#ifdef UNICODE
#define CreateProcess  CreateProcessW
#else
#define CreateProcess  CreateProcessA
#endif // !UNICODE
#endif



#ifdef SUPPORT_POCO_SSL
using Poco::Net::HTTPSClientSession;
#endif
using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPMessage;
using Poco::StreamCopier;
using Poco::Path;
using Poco::URI;
using Poco::Exception;
#ifdef SUPPORT_POCO_SSL
using Poco::Net::SecureStreamSocket;
#endif


#define PocoHttpRequest		Poco::Net::HTTPRequest


#define HEADER_SEPERATION_STR		"\r\n"
#define HEADER_SEPERATION_STR_LEN	2


#define DEFAULT_PORT	80
#define HTTPS_PORT		443


typedef enum 
{
	HTTP_GET_REQ,
	HTTP_POST_REQ,
	HTTP_PUT_REQ

} HTTP_REQ_TYPE;


class PocoHttpClientCls
{
protected:

	int						m_nPort;

	std::string				m_sPath;

	std::string				m_sUID;
	std::string				m_sPW;

	std::string				m_sAdditionalHeaders;

#ifdef SUPPORT_POCO_SSL
	bool					m_bEnableSSL;
#endif

	Poco::Net::HTTPClientSession	*m_pSession;

	Poco::Net::HTTPResponse		m_lastResp;

	class headerUtilCls
	{
		std::string			m_sHeaders;

		std::size_t			m_nCurPos;

	public:

		headerUtilCls() :
			m_nCurPos(0)
		{
			m_sHeaders = "";
		}

		headerUtilCls(std::string sHdrs) :
			m_nCurPos(0)
		{
			m_sHeaders = sHdrs;
		}

		void set(std::string sHdrs)
		{
			m_sHeaders = sHdrs;
		}

		bool getNext(std::string &sName, std::string &sVal)
		{
			try
			{
				if (m_sHeaders.length() < 3)
				{
					return false;
				}

				std::size_t nOff1 = 0;
				std::size_t nOff2 = 0;

				nOff1 = m_sHeaders.find(":", m_nCurPos);
				if (nOff1 == std::string::npos)
				{
					return false;
				}

				nOff2 = m_sHeaders.find(HEADER_SEPERATION_STR, (nOff1 + 1));
				if (nOff2 == std::string::npos)
				{
					sName = m_sHeaders.substr(m_nCurPos, (nOff1 - m_nCurPos));
					sVal = m_sHeaders.substr((nOff1 + 1), (m_sHeaders.length() - (nOff1 + 1)));
					m_nCurPos = m_sHeaders.length();
				}
				else
				{
					sName = m_sHeaders.substr(m_nCurPos, (nOff1 - m_nCurPos));
					sVal = m_sHeaders.substr((nOff1 + 1), (nOff2 - (nOff1 + 1)));
					m_nCurPos = nOff2 + strlen(HEADER_SEPERATION_STR);
				}

				return true;
			}
			catch (...)
			{

			}

			return false;
		}

	};

	std::string stream2string(std::istream &in)
	{
		std::string ret;

		char buffer[4096];

		memset(buffer, 0, sizeof(buffer));
		
		while (in.read(buffer, sizeof(buffer)))
		{
			ret.append(buffer, sizeof(buffer));

			memset(buffer, 0, sizeof(buffer));
		}

		ret.append(buffer, (unsigned long) in.gcount());
		
		return ret;
	}

	Poco::Net::HTTPRequest * createPocoRequest(const std::string &sType, const std::string sReqStr = "")
	{
		try
		{
			std::string sFullReqPath = m_sPath;

			if (sFullReqPath != "")
			{
				if (sReqStr != "")
				{
					long nLastChar = (long) (sFullReqPath.length() - 1);

					if (sFullReqPath[nLastChar] != '/')
					{
						sFullReqPath.append("/");
					}

					sFullReqPath.append(sReqStr);
				}
			}
			else
			{
				sFullReqPath = sReqStr;
			}

			Poco::Net::HTTPRequest *pReq = new Poco::Net::HTTPRequest(sType, sFullReqPath, (HTTPMessage::HTTP_1_1));

			if (m_sAdditionalHeaders != "")
			{
				std::string sName = "";
				std::string sVal = "";

				headerUtilCls	hdrUtil(m_sAdditionalHeaders);

				while (hdrUtil.getNext(sName, sVal) == true)
				{
					if (sName == "Content-type")
					{
						pReq->setContentType(sVal);
					}
					else if (sName == "Connection")
					{
						if ((sVal == "Keep-Alive") || (sVal == "keep-alive"))
						{
							pReq->setKeepAlive(true);
						}
						else
						{
							pReq->setKeepAlive(false);
						}
					}
					else 
					{
						pReq->set(sName, sVal);
					}
				}
			}

			return pReq;
		}
		catch (...)
		{
			;
		}

		return NULL;
	}

	bool doRequest(Poco::Net::HTTPRequest *pReq, const std::string sQuery, std::string &sReply)
	{
		if (m_pSession == NULL || pReq == NULL)
		{
			return false;
		}

		try
		{
			bool bAuthenticated = false;

		SendReq:

			std::ostream& osReq = m_pSession->sendRequest(*pReq);

			if (sQuery.length() > 0)
			{
				osReq << sQuery;
			}

			std::istream& isResp = m_pSession->receiveResponse(m_lastResp);

			if ((m_lastResp.getStatus() == Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED) && (bAuthenticated == false))
			{
				Poco::Net::HTTPCredentials httpAuth(m_sUID, m_sPW);

				httpAuth.authenticate(*pReq, m_lastResp);

				bAuthenticated = true;

				goto SendReq;
			}

			if (m_lastResp.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK)
			{
				sReply = m_lastResp.getReason();
			}
			else
			{
				sReply = stream2string(isResp);
			}

			return true;
		}
		catch (...)
		{
			;
		}

		return false;
	}

public:

	PocoHttpClientCls() :
#ifdef SUPPORT_POCO_SSL
		m_bEnableSSL(false),
#endif
		m_nPort(DEFAULT_PORT),
		m_pSession(NULL)
	{
		m_sUID = "";
		m_sPW = "";

		m_sAdditionalHeaders = "";
	};

	~PocoHttpClientCls()
	{
		try
		{
			if (m_pSession != NULL)
			{
				delete m_pSession;

				m_pSession = NULL;
			}
		}
		catch (...)
		{
			;
		}
	};

#ifdef SUPPORT_POCO_SSL
		void EnableSSL(bool bVal)
		{
			m_bEnableSSL = bVal;
		}
#endif

	void setCredentials
		(
			const std::string &sUID, 
			const std::string &sPW
		)
	{
		m_sUID =	sUID;
		m_sPW =		sPW;
	};

	bool createSession
		(
			const std::string &sHost, 
			const std::string &sPort = "",
			const std::string &sPath = "",
			const std::string &sUID = "", 
			const std::string &sPW = ""
		)
	{
		try
		{
			int nPort = 0;
			
			if (sPort != "")
			{
				nPort = atoi(sPort.c_str());
			}
			else
			{
#ifdef SUPPORT_POCO_SSL
				if (m_bEnableSSL == true)
				{
					nPort = HTTPS_PORT;
				}
				else
#endif
				{
					nPort = DEFAULT_PORT;
				}
			}

			return 
				createSession
				(
					sHost, 
					nPort,
					sPath,
					sUID, 
					sPW
				);
		}
		//catch (Exception& exc)
		catch (...)
		{

		}

		return false;
	};

	bool createSession
		(
			const std::string &sHost, 
			const int nPort = -1,
			const std::string &sPath = "",
			const std::string &sUID = "", 
			const std::string &sPW = ""
		)
	{
		try
		{
			//* If nPort = -1 ... use default port

			if ((nPort < 1) && (nPort != -1))
			{
				return false;
			}

			m_sPath = sPath;

			if (sUID != "")
			{
				m_sUID = sUID;

				if (sPW != "")
				{
					m_sPW = sPW;
				}
			}

#ifdef SUPPORT_POCO_SSL
			if (m_bEnableSSL == true)
			{
				if (nPort == -1)
				{
					m_nPort = HTTPS_PORT;
				}
				else
				{
					m_nPort = nPort;
				}

				m_pSession = new HTTPSClientSession(sHost, m_nPort);
			}
			else
#endif
			{
				if (nPort == -1)
				{
					m_nPort = DEFAULT_PORT;
				}
				else
				{
					m_nPort = nPort;
				}

				m_pSession = new HTTPClientSession(sHost, m_nPort);
			}
		}
		//catch (Exception& exc)
		catch (...)
		{
			return false;
		}

		return true;
	};

	bool endSession()
	{
		try
		{
			if (m_pSession != NULL)
			{
				delete m_pSession;

				m_pSession = NULL;

				return true;
			}
		}
		catch (...)
		{
			;
		}

		return false;;
	}

	bool setTimeout(int nDur)
	{
		if (m_pSession == NULL)
		{
			return false;
		}

		m_pSession->setKeepAliveTimeout(nDur);

		return true;
	}

	void setPath(const std::string &sPath)
	{
		m_sPath = sPath;
	}

	void setAdditionalReqHeaders(const std::string sHeaders)
	{
		m_sAdditionalHeaders = sHeaders;
	}

	void addAdditionalReqHeaders(const std::string sHeaders)
	{
		if (m_sAdditionalHeaders != "")
		{
			long nHeadersLen = (long) m_sAdditionalHeaders.length();

			std::string sTmp = m_sAdditionalHeaders.substr((nHeadersLen - HEADER_SEPERATION_STR_LEN));

			if (sTmp != HEADER_SEPERATION_STR)
			{
				m_sAdditionalHeaders += HEADER_SEPERATION_STR;
			}
		}

		m_sAdditionalHeaders += sHeaders;
	}

	void addAdditionalReqHeaders(const std::string sName, const std::string sVal)
	{
		if (m_sAdditionalHeaders != "")
		{
			long nHeadersLen = (long) m_sAdditionalHeaders.length();

			std::string sTmp = m_sAdditionalHeaders.substr((nHeadersLen - HEADER_SEPERATION_STR_LEN));

			if (sTmp != HEADER_SEPERATION_STR)
			{
				m_sAdditionalHeaders += HEADER_SEPERATION_STR;
			}
		}

		m_sAdditionalHeaders += sName;
		m_sAdditionalHeaders += ":";
		m_sAdditionalHeaders += sVal;
	}

	inline PocoHttpRequest * createHttpRequest(HTTP_REQ_TYPE eReqType, const std::string &sReqStr = "")
	{
		std::string sType;

		switch (eReqType)
		{
		case HTTP_GET_REQ:
			sType = HTTPRequest::HTTP_GET;
			break;
				
		case HTTP_POST_REQ:
			sType = HTTPRequest::HTTP_POST;
			break;
				
		case HTTP_PUT_REQ:
			sType = HTTPRequest::HTTP_PUT;
			break;

		default:
			return nullptr;
		};

		return createPocoRequest(sType, sReqStr);
	}

	inline PocoHttpRequest * createHttpRequest(const std::string &sType, const std::string &sReqStr = "")
	{
		return createPocoRequest(sType, sReqStr);
	}

	inline bool callServer(PocoHttpRequest *pReq, std::string &sReply)
	{
		if (pReq == NULL)
		{
			return false;
		}

		bool bStatus = false;

		try
		{
			sReply = "";

			bStatus = doRequest(pReq, "", sReply);

			return bStatus;
		}
		catch (...)
		{
			;
		}

		return false;
	};

	inline bool callServer(PocoHttpRequest *pReq, const std::string &sQuery, std::string &sReply)
	{
		if (pReq == NULL)
		{
			return false;
		}

		bool bStatus = false;

		try
		{
			sReply = "";

			bStatus = doRequest(pReq, sQuery, sReply);

			return bStatus;
		}
		//catch (Exception& exc)
		catch (...)
		{
			;
		}

		return false;
	};

	inline bool callServer(const std::string &sType, std::string &sReply)
	{
		if (sType == "")
		{
			return false;
		}

		bool bStatus = false;

		PocoHttpRequest *pReq = NULL;

		try
		{
			pReq = createHttpRequest(sType);
			if (pReq == NULL)
			{
				return false;
			}

			sReply = "";

			bStatus = doRequest(pReq, "", sReply);

			delete pReq;

			return bStatus;
		}
		catch (...)
		{
			;
		}

		if (pReq != NULL)
		{
			delete pReq;
		}

		return false;
	};

	inline bool callServer(const std::string &sType, const std::string &sQuery, std::string &sReply)
	{
		if (sType == "")
		{
			return false;
		}

		bool bStatus = false;

		PocoHttpRequest *pReq = NULL;

		try
		{
			pReq = createHttpRequest(sType);
			if (pReq == NULL)
			{
				return false;
			}

			sReply = "";

			bStatus = doRequest(pReq, sQuery, sReply);

			delete pReq;

			return bStatus;
		}
		catch (...)
		{
			;
		}

		if (pReq != NULL)
		{
			delete pReq;
		}

		return false;
	};

	inline bool callServer(HTTP_REQ_TYPE eReqType, std::string &sReply)
	{
		bool bStatus = false;

		PocoHttpRequest *pReq = NULL;

		try
		{
			pReq = createHttpRequest(eReqType);
			if (pReq == NULL)
			{
				return false;
			}

			sReply = "";

			bStatus = doRequest(pReq, "", sReply);

			delete pReq;

			return bStatus;
		}
		catch (...)
		{
			;
		}

		if (pReq != NULL)
		{
			delete pReq;
		}

		return false;
	};

	inline bool callServer(HTTP_REQ_TYPE eReqType, const std::string &sQuery, std::string &sReply)
	{
		bool bStatus = false;

		PocoHttpRequest *pReq = NULL;

		try
		{
			pReq = createHttpRequest(eReqType);
			if (pReq == NULL)
			{
				return false;
			}

			sReply = "";

			bStatus = doRequest(pReq, sQuery, sReply);

			delete pReq;

			return bStatus;
		}
		catch (...)
		{
			;
		}

		if (pReq != NULL)
		{
			delete pReq;
		}

		return false;
	};

	inline bool httpGet(const std::string &sQuery, std::string &sReply)
	{
		bool bStatus = false;

		PocoHttpRequest *pReq = NULL;

		try
		{
			pReq = createHttpRequest(HTTPRequest::HTTP_GET, sQuery);
			if (pReq == NULL)
			{
				return false;
			}

			sReply = "";

			bStatus = doRequest(pReq, sQuery, sReply);

			delete pReq;

			return bStatus;
		}
		catch (...)
		{
			;
		}

		return false;
	}

	inline bool httpPut(const std::string &sQuery, std::string &sReply)
	{
		bool bStatus = false;

		PocoHttpRequest *pReq = NULL;

		try
		{
			pReq = createHttpRequest(HTTPRequest::HTTP_PUT, sQuery);
			if (pReq == NULL)
			{
				return false;
			}

			sReply = "";

			bStatus = doRequest(pReq, sQuery, sReply);

			delete pReq;

			return bStatus;
		}
		catch (...)
		{
			;
		}

		return false;
	}


	inline bool httpPost(const std::string &sQuery, std::string &sReply)
	{
		bool bStatus = false;

		PocoHttpRequest *pReq = NULL;

		try
		{
			pReq = createHttpRequest(HTTPRequest::HTTP_POST, sQuery);
			if (pReq == NULL)
			{
				return false;
			}

			sReply = "";

			bStatus = doRequest(pReq, sQuery, sReply);

			delete pReq;

			return bStatus;
		}
		catch (...)
		{
			;
		}

		return false;
	}

};

#endif //  _POCO_HTTP_CLIENT_H_
