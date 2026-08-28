//****************************************************************************
// FILE:    mail_queue.cpp
//
// DESC:    C++ email definitions
//
// AUTHOR:  Russ Barker
//


#include "mail_queue.h"
#include "main.h"


void mailQueue::mailQueueHandler::run()
{
	LOG("Mail Queue running");

	while (true)
	{
		m_pQueueData->mutexLock();
		
		bool sentSomething(false);
		
		bool q = m_pQueueData->quit();
		
		if (!m_pQueueData->m_mailQueue.empty())
		{
			mailSend sm = m_pQueueData->m_mailQueue.front();
			
			m_pQueueData->m_mailQueue.pop_front();
	
			m_pQueueData->mutexUnlock();
			
			try 
			{
				sm.send();
			
				sentSomething = true;
				
				LOG("Sent mail to " + sm.m_recipient);
			}
			catch(const std::exception &err)
			{
				if (!((sm.m_errCounter++) % m_pQueueData->errorFreq()))
				{
					ELOG(err.what());
				}
				
				if (!q)
				{
					m_pQueueData->mutexLock();
					m_pQueueData->m_mailQueue.push_back(sm);
					m_pQueueData->mutexUnlock();
				}
				
				::Sleep(m_pQueueData->milliSpeed());
			}
			catch(...)
			{
				if (!((sm.m_errCounter++) % m_pQueueData->errorFreq()))
				{
					ELOG("Unknown exception sending mail to " + sm.m_recipient);
				}

				if (!q)
				{
					m_pQueueData->mutexLock();
					m_pQueueData->m_mailQueue.push_back(sm);
					m_pQueueData->mutexUnlock();
				}

				::Sleep(m_pQueueData->milliSpeed());
			}
		
			m_pQueueData->mutexLock();
		}
		else if (!q)
		{
			m_pQueueData->mutexUnlock();

			::Sleep(m_pQueueData->milliSpeed());
			
			m_pQueueData->mutexLock();
		}	
		else if (!sentSomething)
		{
			// don't quit if we are dumping out our queue
			// any failed send will terminate queue dumping on quit

			break;
		}
	}

	LOG("Mail Queue exiting");
	
	m_pQueueData->mutexUnlock();

	return;
}


void mailQueue::queue(const mailSend &mail)
{
	m_pQueueData->mutexLock();

	m_pQueueData->m_mailQueue.push_back(mail);

	m_pQueueData->mutexUnlock();
}

void mailQueue::quit()
{
	m_pQueueData->mutexLock();

	m_pQueueData->quit(true);

	m_pQueueData->mutexUnlock();

	//* wait a few secs for the que to empty out

	::Sleep(2000);
}


void mailQueue::cancel()
{
	if (m_pQueueHandler != NULL)
	{
		m_pQueueHandler->cancel();
	}
}