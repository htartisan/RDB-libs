//****************************************************************************
// FILE:    UrlUtils.h
//
// DESC:    C++ URL definitions
//
// AUTHOR:  Russ Barker
//

#include <WinHttpClient.h>

#include <stringUtils.h>
#include <map>
#include <vector>


#ifndef _urlUtils_H_
#define _urlUtils_H_





//*************************************************************************//
/*  urlUtils

	Collection of classes to help create properly escaped URLs, and to
	parse out and convert escaped URLs (see RFC 2396).

	1) Creating a properly escaped url
	
		Create an urlManager object. Add the query data to it. Call escape() to
		retrieve the entire URL as a properly escaped string.
		
			urlManager ub("http", urlAuthority("127.0.0.1", "80", " "), "/this/is/a/test");
			ub.addQueryEntry(urlQueryEntry("full name", "First Last"));
			ub.addQueryEntry(urlQueryEntry("flag", ""));
			ub.addQueryEntry(urlQueryEntry("wierd_stuff", "!@#$%^&&&===\"\'*()"));
			cout << ub.escape() << endl; // print escaped url to screen
	
	2) Parse out query data from an escaped url
	
		These utilities do not parse out a complete url. They were designed
		to work in conjunction with the mclib webserver utility which
		takes care of parsing out everything up to the query string. As such,
		the parsing here is limited to the query.
		
			urlQuery uq(urlQuery::parse(getEscapedQueryString())); // parse the query string
			// now walk through the results and print to screen
			for (urlQuery::const_iterator i = uq.begin(); i != uq.end(); ++i)
			{
				cout << "key = " << (*i).getKey() << endl;
				cout << "value = " << (*i).getValue() << endl;
				cout << endl;
			}
		
*/
//*************************************************************************//

namespace urlUtils 
{

	class urlQueryEntry
	{
		std::pair<std::string, std::string> m_entry;

	public:
		urlQueryEntry()
		{
		}
		
		urlQueryEntry(const std::string &key, const std::string &value)
			:m_entry(key, value)
		{
		}
		
		urlQueryEntry(const urlQueryEntry &uqe)
			:m_entry(uqe.m_entry)
		{
		}
		
		urlQueryEntry& operator=(const urlQueryEntry &uqe)
		{ 
			m_entry = uqe.m_entry; 
			return *this; 
		}
		
		bool operator==(const urlQueryEntry &uqe) const throw() 
		{ 
			return m_entry == uqe.m_entry; 
		}
				
		// set with unescaped data	
		void set(const std::string &key,const std::string &value)
		{ 
			m_entry = std::pair<std::string, std::string>(key, value); 
		}
			
		// return query as escaped string
		std::string escape() const throw();
			
		// parse from escaped data
		static urlQueryEntry parse(const std::string &s) throw();
			
		// get unescaped values
		std::pair<std::string, std::string> get() const throw() 
		{ 
			return m_entry; 
		}

		std::string getKey() const throw()	
		{ 
			return m_entry.first; 
		}
		
		std::string getValue() const throw() 
		{ 
			return m_entry.second; 
		}
	};
		
	class urlQuery
	{
		std::vector<urlQueryEntry>	m_query;

	public:
		typedef std::vector<urlQueryEntry>::iterator iterator;
		typedef std::vector<urlQueryEntry>::const_iterator const_iterator;
			
		urlQuery()
		{
		
		}

		bool operator==(const urlQuery &uq) const throw() 
		{ 
			return m_query == uq.m_query; 
		}

		void addQueryEntry(const urlQueryEntry &uqe)	
		{ 
			m_query.push_back(uqe); 
		}

		void addQueryEntry(const std::string &key, const std::string &value) 
		{ 
			m_query.push_back(urlQueryEntry(key, value)); 
		}

		void addQueryXml(const std::string &xml)
		{
			m_query.push_back(urlQueryEntry("xml", xml)); 
		}
		
		std::string escape() const throw();
		
		bool empty() const throw()	
		{ 
			return m_query.empty(); 
		}
		
		size_t size() const throw() 
		{ 
			return m_query.size(); 
		}
		
		urlQueryEntry& operator[](size_t index) 
		{ 
			return m_query[index]; 
		}
		
		iterator begin() 
		{ 
			return m_query.begin(); 
		}
		
		iterator end() 
		{ 
			return m_query.end(); 
		}
		
		static urlQuery parse(const std::string &s) throw();
	};
		
	class urlAuthority
	{
		std::string m_host;
		std::string	m_nPort;

		bool		m_sec;

		std::string m_uid;
		std::string	m_pw;

	public:
		urlAuthority()
		{
			m_host = "";
			m_nPort = "";

			m_sec = false;

			m_uid = "";
			m_pw = "";
		}

		urlAuthority
			(
				const std::string &host,
				const std::string &port,
				const std::string &user = "",
				const std::string &pw = ""
			): 
			m_host(host),
			m_nPort(port)
		{
			setLogin(user, pw);
		}

		std::string escape() const throw();

		char * getHost()
		{
			return ((char *) (m_host.c_str()));
		}

		int getPort()
		{
			if (m_nPort.length() < 1)
			{
				return -1;
			}

			return atoi(m_nPort.c_str());
		}

		void setHost(const std::string &host)
		{
			m_host = host;
		}

		void setPort(const int port)
		{
			m_nPort = port;
		}

		bool useSec()
		{
			return m_sec;
		}

		std::string getUserInfo()
		{
			std::string sUI = "";
			
			if (m_uid != "")
			{
				sUI.append(m_uid);

				if (m_pw != "")
				{
					sUI.append(":");
					sUI.append(m_pw);
				}
			}

			return sUI;
		}

		std::string getUser()
		{
			return m_uid;
		}

		std::string getPW()
		{
			return m_pw;
		}

		void setLogin(const std::string &id, const std::string &pw)
		{
			if (id != "")
			{
				m_sec = true;

				m_pw = pw;
			}
			else
			{
				m_sec = false;

				m_pw = "";
			}

			m_uid = id;
		}
	};

	typedef std::map<std::string, std::string> HttpHeaderMap_def;


	//-------------------------------------------------
	// HttpHeadersCls
	//
	// HTTP header map creation utility class
	// ------------------------------------------------

	class HttpHeadersCls
	{
		HttpHeaderMap_def	m_headerMap;

	public:

		HttpHeadersCls()
		{
			if (m_headerMap.size() != 0)
			{
				m_headerMap.clear();
			}
		};

		~HttpHeadersCls()
		{
			if (m_headerMap.size() != 0)
			{
				m_headerMap.clear();
			}
		};

		int numHeaders()
		{
			return m_headerMap.size();
		};

		bool add(const std::string &name, const std::string &value)
		{
			if (name == "")
			{
				return false;
			}

			const std::string normalizedName = stringUtil::toLower(name);
			for (HttpHeaderMap_def::iterator it = m_headerMap.begin(); it != m_headerMap.end();)
			{
				if (stringUtil::toLower((*it).first) == normalizedName)
				{
					HttpHeaderMap_def::iterator old = it++;
					m_headerMap.erase(old);
				}
				else
				{
					++it;
				}
			}

			m_headerMap[name] = value;

			return true;
		};

		bool check(const std::string &name)
		{
			if (m_headerMap.size() == 0)
			{
				return false;
			}

			const std::string normalizedName = stringUtil::toLower(name);
			for (HttpHeaderMap_def::const_iterator it = m_headerMap.begin(); it != m_headerMap.end(); ++it)
			{
				if (stringUtil::toLower((*it).first) == normalizedName)
				{
					return true;
				}
			}

			return false;
		}

		bool check(const std::string &name, const std::string &value)
		{
			if (m_headerMap.size() == 0)
			{
				return false;
			}

			const std::string normalizedName = stringUtil::toLower(name);
			for (HttpHeaderMap_def::const_iterator x = m_headerMap.begin(); x != m_headerMap.end(); ++x)
			{
				if (stringUtil::toLower((*x).first) == normalizedName && (*x).second == value)
				{
					return true;
				}
			}

			return false;
		}

		std::string get()
		{
			std::string sRet = "";

			if (m_headerMap.size() == 0)
			{
				return sRet;
			}

			for (HttpHeaderMap_def::const_iterator x = m_headerMap.begin(); x != m_headerMap.end(); x++)
			{
				sRet.append((*x).first);
				sRet.append(": ");
				sRet.append((*x).second);
				sRet.append("\r\n");
			}

			return sRet;
		};
	};

	class urlManager
	{
	protected:

		std::string		m_scheme;

		urlAuthority	m_authority;

		std::string		m_path;

		urlQuery		m_query;

		HttpHeadersCls	m_reqHeaders;

		std::string		m_respHeaders;
		
		std::string		m_xml;
		std::string		m_queryText;
		
		int				m_timeout;
		int				m_maxRetries;
		int				m_retryCntr;

		WinHttpClient	m_client;
			
	public:
		urlManager
			(
				const std::string &scheme = "",
				const urlAuthority &auth = urlAuthority(),
				const std::string &path = "",
				const urlQuery &query = urlQuery()
			):
			m_scheme(scheme),
			m_authority(auth),
			m_path(path),
			m_query(query),
			m_timeout(60),
			m_maxRetries(0),
			m_retryCntr(0)
		{
			m_xml = "";
			m_queryText = "";
			m_respHeaders = "";
		}
		
			// convenience constructor
		urlManager
			(
				const std::string &addr,
				const std::string &port,
				const std::string &path
			):
			m_scheme("http"), 
			m_authority(addr, port), 
			m_path(path),
			m_timeout(60),
			m_maxRetries(0),
			m_retryCntr(0)
		{
			m_xml = "";
			m_queryText = "";
			m_respHeaders = "";
		}

		urlManager
			(
				const std::string &addr,
				const std::string &port,
				const std::string &uid,
				const std::string &pw,
				const std::string &path
			):
			m_scheme("http"), 
			m_path(path),
			m_timeout(60),
			m_maxRetries(0),
			m_retryCntr(0)
		{
			m_xml = "";
			m_queryText = "";
			m_respHeaders = "";

			m_authority.setLogin(uid, pw);
		}

			void setRetries(int retries)
		{
			m_maxRetries = retries;
		}

		void setUserInfo(const std::string &id, const std::string &pw)
		{
			m_authority.setLogin(id, pw);
		}

		void addHttpHeader(const std::string sHeader, const std::string sValue)
		{
			m_reqHeaders.add(sHeader, sValue);
		}

		std::string getRespHeader()
		{
			return m_respHeaders;
		}

		void addQueryEntry(const std::string &key, const std::string &value) 
		{ 
			m_query.addQueryEntry(key, value); 
		}

		void addQueryEntry(const urlQueryEntry &uqe) 
		{ 
			m_query.addQueryEntry(uqe); 
		}

		void addQueryXml(const std::string &xml) 
		{ 
			m_xml = xml;

			addHttpHeader("Content-type", "application/xml");
			addHttpHeader("Accept", "text/xml");
		}

		void setQuery(const urlQuery &q) 
		{ 
			m_query = q; 
		}

		void setTimeout(int timeout)
		{
			m_timeout = timeout;
		}

		void setAuth(urlAuthority auth)
		{
			m_authority = auth;
		}

		const urlQuery& getQuery() const throw() 
		{ 
			return m_query; 
		}

		std::string escape() const throw();

		std::string  buildQuery()
		{
			m_queryText.clear();

			//m_queryText = escape();
			std::string sTmp = escape();

			if (m_xml.empty() == false) 			
			{
				//m_queryText.append(" ");
				m_queryText.append((char *) (m_xml.c_str()), (int) (m_xml.size()));
			}

			return m_queryText;
		}

		bool get(std::string &reply) throw();

		bool put(std::string &query) throw();

		bool post(std::string &query, std::string &reply) throw();
		bool post(std::string &reply)
		{
			return post(m_queryText, reply);
		}

		//static void *m_pThisInstance;
		//void *m_pThisInstance;

		//static bool m_bMsgReceived;
		bool m_bMsgReceived;

		//static int m_nHttpCnt;
		int m_nHttpCnt;

		//static std::string m_sReply;
		std::string m_sReply;

		//* HTTP util functions

		void initHttp()
		{
			//m_pThisInstance = (void *) this;

			addHttpHeader("Connection", "close");
		}
	};

	std::string escape(const std::string &s) throw();

	std::string unescapeString(const std::string &s) throw();

	std::string mimeEncode(const std::string sSrc);
};


#endif  // _urlUtils_H_
