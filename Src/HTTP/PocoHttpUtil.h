//****************************************************************************************************
//* 
//*  FILE: PocoHttpUtil.h
//* 
//*  "PocoHttpUtil" provide sample interfaces for using the POCO HTTP/SHTTP classes
//* 
//*  POCO needs to be installed: https://pocoproject.org/download/index.html
//* 
//*	 Compilation info... 
//*
//*		Windows: Compile solution/project using Visual Studio 2013  
//* 
//*     Linux:
//*      g++ -Wall -I/usr/local/include/Poco -o sp_api sp_api.cpp -L/usr/local/lib/ -std=c++11 -lPocoFoundation -lPocoJSON -lPocoNet -lPocoNetSSL
//* 


#ifndef _POCOHTTPUTIL_H_
#define _POCOHTTPUTIL_H_

//#define _SUPPORT_SSL_LOGIC_

#ifdef WIN32
#ifdef _UNICODE
#define POCO_WIN32_UTF8
#endif
#endif


#include <Poco/URI.h>
#include <Poco/Net/HTTPMessage.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/StreamCopier.h>
#include <Poco/NullStream.h>
#include <Poco/JSON/JSON.h>
#include <Poco/Net/FilePartSource.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/JSON/JSONException.h>
#include <Poco/Exception.h>
#include <Poco/Net/NetException.h>
#include <Poco/Mutex.h>
#include <Poco/ScopedLock.h>

//#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
//#include <Poco/Net/HTTPServerParams.h>
//#include <Poco/Net/HTTPServerRequestImpl.h>
//#include <Poco/Net/HTTPServerResponse.h>
//#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/SecureStreamSocket.h>
#include <Poco/Net/SecureServerSocket.h>
#include <Poco/Net/X509Certificate.h>
#include <Poco/Net/VerificationErrorArgs.h>
#include <Poco/Exception.h>
#include <Poco/SharedPtr.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/ConsoleCertificateHandler.h>
#include <Poco/Net/HTTPCredentials.h>
#include <Poco/Net/HTTPBasicCredentials.h>


#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>

#include <Poco/Util/ServerApplication.h>


#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>

#include <time.h>


#include <stringUtils.h>


#ifndef WRITE_TO_LOG
#define WRITE_TO_LOG(l, t, m)
#endif

//#ifndef CHECK_LOG_LEVEL
//CHECK_LOG_LEVEL(l)			false
//#endif



using namespace Poco::Net;
using namespace Poco;
using namespace Poco::JSON;
using namespace Poco::Dynamic;
using Poco::StreamCopier;
using Poco::Net::HTTPMessage;
using Poco::Net::HTTPRequest;
using Poco::Net::HTMLForm;
using Poco::URI;
using Poco::Exception;
using namespace std;


#ifndef _PATH_SEPERATOR_
#ifdef WIN32
#define _PATH_SEPERATOR_		'\\'
#define _PATH_SEPERATOR_STR_	"\\"
#else
#define _PATH_SEPERATOR_		'/'
#define _PATH_SEPERATOR_STR_	"/"
#endif
#endif




//* CPocoHttpUtil class definition

class CPocoHttpUtil
{
	//* class typedefs

public:

	typedef struct PocoServerInfo_tag
	{
		chrono::system_clock::time_point						m_tpTokenExpires;

		Poco::SharedPtr<Poco::Net::InvalidCertificateHandler>	m_pCertHandler;

		Poco::Net::Context::Ptr									m_pContext;

		Poco::Net::HTTPSClientSession							*m_pSession;

		Poco::Mutex												m_mutex;

		std::string												m_sServerURI;

		bool													m_bCreated;

		std::string												m_sAuthType;
		std::string												m_sAuthToken;

		bool													m_bAuthenticated;

		PocoServerInfo_tag() :
			m_pCertHandler(0),
			m_pContext(0),
			m_pSession(0),
			m_bCreated(false),
			m_bAuthenticated(false)
		{
			m_tpTokenExpires = chrono::system_clock::now();

			m_sServerURI = "";

			m_sAuthType = "";
			m_sAuthToken = "";
		}

	} PocoServerInfo_def;

#ifdef _SUPPORT_SSL_LOGIC_
	static void SSLInit()
	{
		Poco::Path mycert(Poco::Util::ServerApplication::instance().config().getString(“application.dir”));

		mycert.append(“mycert.pem”);

		Poco::SharedPtr<Poco::Net::KeyConsoleHandler> pConsoleHandler = new Poco::Net::KeyConsoleHandler(true);

		Poco::SharedPtr<Poco::Net::AcceptCertificateHandler> pCertHandler = new Poco::Net::AcceptCertificateHandler(true);

		Poco::Net::SSLManager::instance().initializeServer(pConsoleHandler, pCertHandler, NULL);

		Context::Ptr pContext = new
			Context
			(
				Context::SERVER_USE,
				mycert.toString(),
				mycert.toString(),
				“”,
				Context::VERIFY_RELAXED,
				9,
				false,
				“ALL:!ADH : !LOW : !EXP : !MD5 : @STRENGTH”
				);

		Poco::Net::SSLManager::instance().initializeServer(pConsoleHandler, pCertHandler, pContext);
	}
#endif

protected:

	//* class member variables

#ifdef _SUPPORT_SSL_LOGIC_
	bool				m_bUseSecureSession;
#endif

	std::string			m_sCertFile;

	std::string			m_sServerAddress;

	std::string			m_sUserID;
	std::string			m_sUserPW;

	std::string			m_sAccessToken;
	std::string			m_sRefreshToken;

	PocoServerInfo_def	m_serverInfo;

	std::string			m_sLastStatus;

public:

	void init()
	{
#ifdef _SUPPORT_SSL_LOGIC_
		m_bUseSecureSession = false;
#endif

		m_sServerAddress = "";

		m_sCertFile = "";

		m_sUserID = "";
		m_sUserPW = "";

		m_sAccessToken = "";
		m_sRefreshToken = "";

		m_sLastStatus = "";

#ifdef _SUPPORT_SSL_LOGIC_
		SSLInit::init();
#endif
	}

	//* class constructor

	CPocoHttpUtil()
	{
		init();
	}

	//* public member functions

#ifdef _SUPPORT_SSL_LOGIC_
	void setSecureSessionFlag(bool bVal)
	{
		m_bUseSecureSession = bVal;
	}
#endif

	void setCertFile(const std::string &sVal)
	{
		m_sCertFile = sVal;
	}

	void setUserID(const std::string &sVal)
	{
		m_sUserID = sVal;
	}

	void setUserPW(const std::string &sVal)
	{
		m_sUserPW = sVal;
	}

	bool authenticate(PocoServerInfo_def &srvrInfo, HTTPRequest &req, const std::string &sID = "", const std::string &sPW = "")
	{
		try
		{
			if (sID != "")
			{
				m_sUserID = sID;
			}

			if (sPW != "")
			{
				m_sUserPW = sPW;
			}

			if (m_sUserID == "")
			{
				WRITE_TO_LOG(1, "WARNING", ("Invalid DeepGram User ID "));

				return false;
			}

			HTTPBasicCredentials cred(m_sUserID, m_sUserPW);

			cred.authenticate(req);

			std::string sType = "";
			std::string sToken = "";

			req.getCredentials(sType, sToken);

			if (sType != "Basic")
			{
				WRITE_TO_LOG(2, "STATUS", ("Credentials auth type: " + sType));
			}

			srvrInfo.m_sAuthType = sType;
			srvrInfo.m_sAuthToken = sToken;

			srvrInfo.m_bAuthenticated = true;

			return true;
		}
		catch (...)
		{
			WRITE_TO_LOG(1, "ERROR", "Unknown exception during HTTP request authentication ");
		}

		return false;
	}

	bool setRequestAuth(PocoServerInfo_def &srvrInfo, HTTPRequest &req)
	{
		try
		{
			if (srvrInfo.m_bAuthenticated == false)
			{
				authenticate(m_serverInfo, req);
			}
			else
			{
				req.add("Authorization", (m_serverInfo.m_sAuthType + " " + m_serverInfo.m_sAuthToken));	
			}

			return true;
		}
		catch (...)
		{
			WRITE_TO_LOG(1, "ERROR", "Unknown exception while setting HTTP Authorization header ");
		}

		return false;
	}

	std::string getLastStatus()
	{
		Mutex::ScopedLock lock(m_serverInfo.m_mutex);

		return m_sLastStatus;
	}

	std::string reqHeadersToString(HTTPRequest &req, bool bAddLR = false)
	{
		std::string sOut = "";

		bool bFirst = true;

		for (auto x = req.begin(); x != req.end(); x++)
		{
			if (bFirst == true)
			{
				bFirst = false;
			}
			else
			{
				sOut.append(",");

				if (bAddLR == true)
				{
					sOut.append("\r\n");
				}
				else
				{
					sOut.append("  ");
				}
			}

			sOut.append((*x).first);
			sOut.append(": ");
			sOut.append((*x).second);
		}

		return sOut;
	}

protected:

	//* internal functions

	void updateStatus(const std::string &sVal)
	{
		Mutex::ScopedLock lock(m_serverInfo.m_mutex);

		m_sLastStatus = sVal;
	}

	bool checkHttpResponseCodes(HTTPResponse &response)
	{
		try
		{
			updateStatus("");

			HTTPResponse::HTTPStatus eResponseCode = response.getStatus();

			//* HTTP response successful

			if (eResponseCode == Poco::Net::HTTPResponse::HTTP_OK)
			{
				updateStatus("HTTP_OK, Request completed successfully ");

				WRITE_TO_LOG(3, "STATUS", "HTTP_OK, Request completed successfully ");

				return true;
			}

			//* HTTP response errors

			if (eResponseCode == Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED)
			{
				updateStatus("ERROR: HTTP_UNAUTHORIZED, Token retry call needed... getToken(session, false) ");

				WRITE_TO_LOG(2, "ERROR", "HTTP_UNAUTHORIZED, Token retry call needed... getToken(session, false) ");

				return false;
			}

			if (eResponseCode == Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE)
			{
				updateStatus("ERROR: HTTP_SERVICE_UNAVAILABLE, Service Unavailable, please try later ");

				WRITE_TO_LOG(2, "ERROR", "HTTP_SERVICE_UNAVAILABLE, Service Unavailable, please try later ");

				return false;
			}

			if (eResponseCode == Poco::Net::HTTPResponse::HTTP_BAD_REQUEST)
			{
				updateStatus("ERROR: HTTP_BAD_REQUEST, Service not found ");

				WRITE_TO_LOG(2, "ERROR", "HTTP_BAD_REQUEST, Service not found ");

				return false;
			}

			//* TODO: check other response codes


			std::string sMsg = "";

			sMsg = "HTTP_CODE[";
			sMsg += stringUtil::tos((unsigned int)eResponseCode);
			sMsg += "], ";
			sMsg += response.getReason();

			WRITE_TO_LOG(2, "ERROR", sMsg);
		}
		catch (const std::exception& e)
		{
			std::string sErr = e.what();

			updateStatus("ERROR: Exception while checking HTTP response codes - " + sErr);

			WRITE_TO_LOG(2, "ERROR", ("Exception while checking HTTP response codes - " + sErr));

			return false;
		}
		catch (...)
		{
			updateStatus("ERROR: Unknown exception while checking HTTP response codes ");

			WRITE_TO_LOG(2, "ERROR", "Unknown exception while checking HTTP response codes ");

			return false;
		}

		return true;
	}

	//bool getToken(bool bDoRefresh);

	//bool checkTokenLocal();

	std::string getStreamReply(std::istream& isBuf)
	{
		std::string sOut = "";

		std::string sTmp = "";

		//long nMax = sTmp.max_size();

		while (isBuf.eof() == false)
		{
			StreamCopier::copyToString(isBuf, sTmp);
			//StreamCopier::copyToString(isBuf, sTmp, (nMax - 1));

			sOut += sTmp;

			sTmp = "";
		}

		return sOut;
	}

	int getStreamToFile(std::istream& isBuf, const std::string &sFilePath)
	{
#if 0
		std::filebuf fbOut;

		if (fbOut.open(sFilePath, std::ios::out) == 0)
		{
			WRITE_TO_LOG(3, "DEBUG", "Unable to create/open output file ");

			return -1;
		}

		std::ostream osOut(&fbOut);

		std::streamsize	nTmp = 0;
#else
		ofstream osOut;

		try
		{
			osOut.open(sFilePath);
		}
		catch (...)
		{
			return -1;
		}

		std::string sBuf = "";

		int	nLen = 0;
#endif

		long nRead = 0;

		while (isBuf.eof() == false)
		{
#if 0
			try
			{
				//StreamCopier::copyStream(isBuf, osOut);
				nTmp = StreamCopier::copyStreamUnbuffered(isBuf, osOut);
			}
			catch (...)
			{
				break;
			}

			nRead += (long)nTmp;

			nLen = 0;
#else
			try
			{
				StreamCopier::copyToString(isBuf, sBuf);

				nLen = sBuf.length();

				if (nLen > 0)
				{
					for (int x = 0; x < nLen; x++)
					{
						//* filter out any garbage chars

						if (sBuf[x] < 0)
						{
							sBuf[x] = 0;
						}
					}

					osOut << sBuf;

					nRead += nLen;
				}

				sBuf = "";
			}
			catch (...)
			{
				break;
			}
#endif
		}

#if 0
		fbOut.flush();

		fbOut.close();
#else
		osOut.flush();

		osOut.close();
#endif

		return nRead;
	}

public:

	//* HTTP session management functions

	bool createSession(PocoServerInfo_def &srvrInfo, std::string sURL = "")
	{
		try
		{
			Mutex::ScopedLock lock(srvrInfo.m_mutex);

			if (srvrInfo.m_bCreated == true)
			{
				endSession(srvrInfo);
			}

			if (sURL == "")
			{
				srvrInfo.m_sServerURI = m_sServerAddress;
			}
			else
			{
				srvrInfo.m_sServerURI = sURL;
			}

			if (srvrInfo.m_sServerURI == "")
			{
				return false;
			}

			URI uri(srvrInfo.m_sServerURI);

			std::string sCertLower = stringUtil::toLower(m_sCertFile);

			try
			{
				if (m_sCertFile != "" && sCertLower != "ignore")
				{
					WRITE_TO_LOG(3, "DEBUG", ("Creating HTTPS context from certificate file: " + m_sCertFile));

					srvrInfo.m_pContext = new
						Poco::Net::Context
						(
							Poco::Net::Context::CLIENT_USE,
							"",
							"",
							m_sCertFile,
							Poco::Net::Context::VerificationMode::VERIFY_RELAXED,
							9,
							true
						);
				}
				else
				{
					if (sCertLower == "ignore")
					{
						WRITE_TO_LOG(3, "DEBUG", "Creating HTTPS/SSL (accept) certificate handler ");

						srvrInfo.m_pCertHandler = new
							Poco::Net::AcceptCertificateHandler(false);
					}
					else
					{
						WRITE_TO_LOG(3, "DEBUG", "Creating HTTPS/SSL (console) certificate handler ");

						srvrInfo.m_pCertHandler = new
							Poco::Net::ConsoleCertificateHandler(false); // ask the user via console
					}

					if (!srvrInfo.m_pCertHandler)
					{
						WRITE_TO_LOG(1, "ERROR", "Problem creating HTTPS certificate handler ");
					}

					WRITE_TO_LOG(3, "DEBUG", "Creating HTTPS context from default SSL certificate ");

					srvrInfo.m_pContext = new
						Poco::Net::Context
						(
							Poco::Net::Context::CLIENT_USE,
							"",
							"",
							"",
							Poco::Net::Context::VerificationMode::VERIFY_RELAXED,
							9,
							true
						);
				}
			}
			catch (...)
			{
				WRITE_TO_LOG(1, "ERROR", ("HTTPS context creation failed (Certificate file: " + m_sCertFile + ") "));
			}

			if (srvrInfo.m_pContext)
			{
				WRITE_TO_LOG(3, "DEBUG", ("Creating HTTPS session (from context) "));

				srvrInfo.m_pSession = new HTTPSClientSession(uri.getHost(), uri.getPort(), srvrInfo.m_pContext);

				SSLManager::instance().initializeClient(0, srvrInfo.m_pCertHandler, srvrInfo.m_pContext);
			}
			else
			{
				WRITE_TO_LOG(3, "DEBUG", ("Creating HTTPS session (without context) "));

				srvrInfo.m_pSession = new HTTPSClientSession(uri.getHost(), uri.getPort());
			}

			if (srvrInfo.m_pSession != NULL)
			{
				srvrInfo.m_bCreated = true;

				return true;
			}

			WRITE_TO_LOG(1, "ERROR", ("HTTPS session creation failed "));
		}
		catch (...)
		{
			updateStatus("ERROR: Unknown exception while creating HTTP session. ");
		}

		return false;
	}

	bool endSession(PocoServerInfo_def &srvrInfo)
	{
		try
		{
			Mutex::ScopedLock lock(srvrInfo.m_mutex);

			if (srvrInfo.m_pSession != 0)
			{
				WRITE_TO_LOG(3, "DEBUG", ("Ending HTTP session "));

				delete srvrInfo.m_pSession;

				srvrInfo.m_pSession = 0;
			}

			if (srvrInfo.m_pContext)
			{
				WRITE_TO_LOG(3, "DEBUG", ("Deleting HTTP context "));

				delete srvrInfo.m_pContext;

				srvrInfo.m_pContext = 0;
			}

			//if (srvrInfo.m_pCertHandler)
			//{
			//	WRITE_TO_LOG(3, "DEBUG", ("Deleting HTTP Certificate Handler "));

			//	delete srvrInfo.m_pCertHandler;

			//	srvrInfo.m_pCertHandler = 0;
			//}

			srvrInfo.m_bCreated = false;
		}
		catch (...)
		{
			updateStatus("ERROR: Unknown exception while ending HTTP session. ");
		}

		return false;
	}

	bool isSessionCreated()
	{
		if (m_serverInfo.m_pSession != 0)
		{
			return true;
		}

		return false;
	}

	bool isSessionCreated(PocoServerInfo_def &srvrInfo)
	{
		if (srvrInfo.m_pSession != 0)
		{
			return true;
		}

		return false;
	}

	//* class destructor

	~CPocoHttpUtil()
	{
		if (m_serverInfo.m_bCreated == true)
		{
			endSession(m_serverInfo);
		}
	}


};


#endif  // _POCOHTTPUTIL_H_
