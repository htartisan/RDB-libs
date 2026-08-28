//****************************************************************************
// FILE:    mail_send.cpp
//
// DESC:    C++ email definitions
//
// AUTHOR:  Russ Barker
//


#include <winsock2.h>

#include "main.h"

#include "mail_send.h"

#include "stl/stringUtils.h"

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <cstring>

using namespace std;
using namespace stringUtil;

#define	VERIFY_RET_VAL( arg, err ) \
            { int nRet = arg; if (nRet) throw runtime_error(err); }


#define SMTP_PORT       25
#define MAX_LINE_SIZE   1024
#define MAX_NAME_SIZE   64


//* SendMail data structures 

typedef struct SENDMAIL 
{
    LPCTSTR lpszHost;          // host name or dotted IP address
    LPCTSTR lpszSender;        // sender userID (optional)
    LPCTSTR lpszSenderName;    // sender display name (optional)
    LPCTSTR lpszRecipient;     // recipient userID
    LPCTSTR lpszRecipientName; // recipient display name (optional)
    LPCTSTR lpszReplyTo;       // reply to userID (optional)
    LPCTSTR lpszReplyToName;   // reply to display name (optional)
    LPCTSTR lpszMessageID;     // message ID (optional)
    LPCTSTR lpszSubject;       // subject of message
    LPCTSTR lpszMessage;       // message text
    LPCTSTR	lpszMailerID;		// application that is doing the mailing

} SENDMAIL;


//* Send - send the request to the SMTP server, and handle errors.

static int Send(SOCKET s, const char *lpszBuff, int nLen, int nFlags)
{
	int nCnt = 0;

	while (nCnt < nLen)
	{
		int nRes = send( s, lpszBuff + nCnt, nLen - nCnt, nFlags );

		if (nRes == SOCKET_ERROR)
		{
			return WSAGetLastError();
		}
		else if (nRes == 0)
		{
			return WSAECONNRESET;
		}
		else
		{
			nCnt += nRes;
		}
	}

	std::string text(lpszBuff, nLen);

	DLOG(text);

	return 0;
}


// Receive - receive a reply from the SMTP server, and verify that
// the request has succeeded by checking against the specified
// reply code.

int Receive
	( 
		SOCKET s, 
		LPTSTR lpszBuff, 
		int nLenMax, 
		int nFlags,
		LPCTSTR lpszReplyCode 
	)
{
	LPTSTR p;

	int    nRes = recv(s, lpszBuff, nLenMax, nFlags);

	if (nRes == SOCKET_ERROR)
	{
		return WSAGetLastError();
	}
	else
	{
		*(lpszBuff + nRes) = '\0';
	}

	std::string text = lpszBuff;

	DLOG(text);

	// check reply code for success/failure
	p = strtok(lpszBuff, "\n");
	while (p)
	{
		if (*(p + 3) == ' ')
		{
			if (!strncmp(p, lpszReplyCode, strlen(lpszReplyCode)))
			{
				return 0;
			}
			else
			{
				int nErr = 1;

				sscanf(p, "%d", &nErr);

				return -nErr;
			}
		}
		else
		{
			p = strtok(NULL, "\n");
		}
	}

	return -1;
}

//* SendMailMessage - 
//*		Actually sends the message. 
//*		This is a private function.

static void SendMailMessage(SENDMAIL *pMail) throw(runtime_error)
{
	char   szBuff[MAX_LINE_SIZE + 1]; // transmit/receive buffer
	char   szUser[MAX_NAME_SIZE + 1] = {0}; // user name buffer
	char   szName[MAX_NAME_SIZE + 1] = {0}; // host name buffer

	DWORD  dwSize = sizeof(szUser);

	SOCKET s = INVALID_SOCKET;

	struct hostent    *ph = 0;
	struct sockaddr_in addr;

	char         szTime[MAX_NAME_SIZE + 1] = {0}; // time related vars

	time_t       tTime;

	struct tm   *ptm = 0;
	struct timeb tbTime;

	try 
	{
		if (!pMail || !pMail->lpszHost || !pMail->lpszRecipient || !pMail->lpszMessage)
		{
			throw runtime_error("Invalid mail parameters");
		}

		std::string command;

		//* connect to the SMTP port on remote host
		if ((s = ::socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		{
			throw runtime_error("Error opening socket " + tos(::WSAGetLastError()));
		}

		if (isdigit(*pMail->lpszHost) && strchr(pMail->lpszHost, '.'))
		{
			unsigned long iaddr = inet_addr(pMail->lpszHost);
		
			ph = ::gethostbyaddr((const char *) &iaddr, 4, PF_INET);
		}
		else
		{
			ph = ::gethostbyname(pMail->lpszHost);
		}
			
		if (ph == NULL)
		{
			throw runtime_error
			(
				string("Error resolving mail host gethostbyaddr or gethostbyname failed for ") + 
				pMail->lpszHost + " " + tos(::WSAGetLastError())
			);
		}

		addr.sin_family = AF_INET;
		addr.sin_port   = htons(SMTP_PORT);

		memcpy(&addr.sin_addr, ph->h_addr_list[0], sizeof(struct in_addr));

		if (::connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr)))
		{
			throw runtime_error("Mail connect failed " + tos(::WSAGetLastError()));
		}

		//* receive signon message
		VERIFY_RET_VAL(Receive( s, szBuff, MAX_LINE_SIZE, 0, "220" );, "signon failure");

		//* get user name and local host name
		if (!GetUserName(szUser, &dwSize) ||
			::gethostname(szName, MAX_NAME_SIZE) == SOCKET_ERROR)
		{
			throw runtime_error("Unable to determine local mail identity");
		}
		szName[MAX_NAME_SIZE] = '\0';
		
		//* send HELO message
		command = "HELO ";
		command += szName;
		command += "\r\n";
		
		VERIFY_RET_VAL(Send(s, command.data(), (int) command.size(), 0);, "HELO send failure")
		VERIFY_RET_VAL(Receive( s, szBuff, MAX_LINE_SIZE, 0, "250" ); , "HELO reply failure")

		//* send MAIL message
		if (pMail->lpszSender)
		{
			command = "MAIL FROM: <";
			command += pMail->lpszSender;
		
			if( strchr(pMail->lpszSender, '@'))
			{
				command += ">\r\n";
			}
			else
			{
				command += "@";
				command += szName;
				command += ">\r\n";
			}
		}
		else
		{
			command = "MAIL FROM:<";
			command += szUser;
			command += "@";
			command += szName;
			command += ">\r\n";
		}

		VERIFY_RET_VAL(Send(s, command.data(), (int) command.size(), 0);, "Mailfrom send failure")
		
		VERIFY_RET_VAL(Receive( s, szBuff, MAX_LINE_SIZE, 0, "250"); ,"Mailfrom reply failure")

		//* send RCPT message
		vector<string> addrs = stringUtil::tokenizer(stringUtil::stripWhitespace(pMail->lpszRecipient),' ');

		for (vector<string>::const_iterator i = addrs.begin(); i != addrs.end(); ++i)
		{
			command = "RCPT TO: <";
			command += *i;
			command += ">\r\n";
		
			VERIFY_RET_VAL(Send(s, command.data(), (int) command.size(), 0); ,"RCPT TO send failure")
			
			VERIFY_RET_VAL(Receive( s, szBuff, MAX_LINE_SIZE, 0, "25"); ,"RCPT TO reply failure")
		}

		//* send DATA message
		command = "DATA\r\n";
		
		VERIFY_RET_VAL(Send(s, command.data(), (int) command.size(), 0); ,"DATA send failure")
		
		VERIFY_RET_VAL(Receive(s, szBuff, MAX_LINE_SIZE, 0, "354"); ,"DATA reply failure")

		//* construct date string
		tTime = ::time(NULL);
		ptm   = ::localtime(&tTime);

		strftime(szTime, MAX_NAME_SIZE, "%a, %d %b %Y %H:%M:%S %Z", ptm);

		//* find time zone offset and correct for DST
		ftime(&tbTime);
		if (tbTime.dstflag)
		{
			tbTime.timezone -= 60;
		}

		char szTimezone[16];
		const int timezoneMinutes = -tbTime.timezone;
		sprintf_s
		(
			szTimezone,
			sizeof(szTimezone),
			" %c%02d%02d",
			(timezoneMinutes < 0) ? '-' : '+',
			abs(timezoneMinutes / 60),
			abs(timezoneMinutes % 60)
		);

		std::string dateText = szTime;
		dateText += szTimezone;

		//* send mail headers
		//* Date:
		command = "Date: ";
		command += dateText;
		command += "\r\n";
		
		VERIFY_RET_VAL(Send(s, command.data(), (int) command.size(), 0); ,"Error sending date header")

		//* X-Mailer:
		if (pMail->lpszMailerID)
		{
			command = "X-Mailer: ";
			command += pMail->lpszMailerID;
			command += "\r\n";
		
			VERIFY_RET_VAL(Send(s, command.data(), (int) command.size(), 0);,"Error sending xmailer header" )
		}
			
		//* Message-ID:
		if (pMail->lpszMessageID)
		{
			command = "Message-ID: ";
			command += pMail->lpszMessageID;
			command += "\r\n";
			
			VERIFY_RET_VAL(Send(s, command.data(), (int) command.size(), 0); ,"Error sending message ID")
		}

		// To:
		command = "To: ";
		command += pMail->lpszRecipient;
		if (pMail->lpszRecipientName)
		{
			command += " (";
			command += pMail->lpszRecipientName;
			command += ")\r\n";
		}
		else
		{
			command += "\r\n";
		}

		VERIFY_RET_VAL(Send(s, command.data(), (int) command.size(), 0); ,"Error sending To: header")

		//* From:
		if (pMail->lpszSender)
		{
			command = "From: ";
			command += pMail->lpszSender;
			if	(pMail->lpszSenderName)
			{
				command += " (";
				command += pMail->lpszSenderName;
				command += ")\r\n";
			}
			else
			{
				command += "\r\n";
			}
		}
		else
		{
			command = "From: ";
			command += szUser;
			command += "@";
			command += szName;
			command += "\r\n";
		}

		VERIFY_RET_VAL(Send(s, command.data(), (int) command.size(), 0); ,"Error sending from: header")

		//* Reply-To:
		if (pMail->lpszReplyTo)
		{
			command = "Reply-To: ";
			command += pMail->lpszReplyTo;
			if (pMail->lpszReplyToName)
			{
				command += " (";
				command += pMail->lpszReplyToName;
				command += ")\r\n";
			}
			else
			{
				command += "\r\n";
			}
			
			VERIFY_RET_VAL(Send(s, command.data(), (int) command.size(), 0); ,"Error sending reply-to header")
		}

		//* Subject:
		command = "Subject: ";
		if (pMail->lpszSubject)
		{
			command += pMail->lpszSubject;
		}
		command += "\r\n";
		
		VERIFY_RET_VAL(Send(s, command.data(), (int) command.size(), 0); ,"Error sending subject")

		//* 08/13/98 rlb
		//* empty line needed after headers, RFC822

		VERIFY_RET_VAL(Send(s, "\r\n", 2, 0);,"Sending empty line after headers" )

		//* send message text
		VERIFY_RET_VAL(Send(s, pMail->lpszMessage, strlen(pMail->lpszMessage), 0); ,"Error sending message body")

		//* send message terminator and receive reply
		VERIFY_RET_VAL(Send(s, "\r\n.\r\n", 5, 0); ,"Error sending message terminator")

		VERIFY_RET_VAL(Receive(s, szBuff, MAX_LINE_SIZE, 0, "250"); ,"Error reply message terminator")

		//* send QUIT message
		VERIFY_RET_VAL(Send(s, "QUIT\r\n", 6, 0);,"Error sending quit" )
		
			VERIFY_RET_VAL(Receive( s, szBuff, MAX_LINE_SIZE, 0, "221"); ,"Error reply quit")

		::closesocket(s);
	}
	catch(...)
	{
		if (s != INVALID_SOCKET)
		{
			::closesocket(s);
		}

		s = INVALID_SOCKET;
		
		throw;
	}
}


////////////////

mailSend::mailSend
	(
		const std::string &mailto,
		const std::string &mailhost,
		const std::string &mailfrom,
		const std::string &subject,
		const std::string &message,
		const std::string &mailerID
	) :
	m_host(mailhost),
	m_sender(mailfrom),
	m_recipient(mailto),
	m_subject(subject),
	m_message(message),
	m_mailerID(mailerID),
	m_errCounter(0)
{

}

void mailSend::send() throw(std::runtime_error)
{
	WORD    wVersion = MAKEWORD( 1, 1 );
	WSADATA wsaData;
	int		retval;

	if((retval = ::WSAStartup(wVersion, &wsaData)))
	{
		throw runtime_error("winsock init faiure. " + tos(retval));
	}

	SENDMAIL sm;

	sm.lpszHost			= (m_host == ""			? 0 : m_host.c_str());
	sm.lpszSender		= (m_sender == ""		? 0 : m_sender.c_str());
	sm.lpszSenderName	= (m_senderName == ""	? 0 : m_senderName.c_str()); 
	sm.lpszRecipient	= (m_recipient == ""	? 0 : m_recipient.c_str());
	sm.lpszRecipientName= (m_recipientName == ""? 0 : m_recipientName.c_str());
	sm.lpszReplyTo		= (m_replyTo == ""		? 0 : m_replyTo.c_str());
	sm.lpszReplyToName	= (m_replyToName == ""	? 0 : m_replyToName.c_str());
	sm.lpszMessageID	= (m_messageID == ""	? 0 : m_messageID.c_str());
	sm.lpszSubject		= (m_subject == ""		? 0 : m_subject.c_str());
	sm.lpszMessage		= (m_message == ""		? 0 : m_message.c_str());
	sm.lpszMailerID		= (m_mailerID == ""		? 0 : m_mailerID.c_str());

	try 
	{
		SendMailMessage(&sm);
	}
	catch(...)
	{
		::WSACleanup();
		throw;
	}
		
	::WSACleanup();
}
