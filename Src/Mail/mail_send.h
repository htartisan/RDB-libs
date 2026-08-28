//****************************************************************************
// FILE:    mail_send.h
//
// DESC:    C++ email definitions
//
// AUTHOR:  Russ Barker
//


#ifndef mail_send_H_
#define mail_send_H_



#include <string>
#include <stdexcept>



class mailSend
{
public:
	std::string	m_host;				//* host name or dotted IP address
	std::string m_sender;			//* sender userID (optional)
	std::string m_senderName;		//* sender display name (optional)	
	std::string m_recipient;		//* recipient userID
	std::string m_recipientName;	//* recipient display name (optional)
	std::string m_replyTo;			//* replay to userID (optional)
	std::string m_replyToName;		//* reply to display name (optional)
	std::string m_messageID;		//* message Id (optional)
	std::string m_subject;			//* subject of message
	std::string m_message;			//* message text
	std::string m_mailerID;			//* identity of mailing program
	
	int m_errCounter;
	
public:
	mailSend
	(
		const std::string &mailto,
		const std::string &mailhost,
		const std::string &mailfrom,
		const std::string &subject,
		const std::string &message,
		const std::string &mailerID = "SendMail"
	);
			
	void send() throw(std::runtime_error);
			
};

#endif
