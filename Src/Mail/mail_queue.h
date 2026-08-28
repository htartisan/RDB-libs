//****************************************************************************
// FILE:    mail_queue.h
//
// DESC:    C++ email definitions
//
// AUTHOR:  Russ Barker
//


#ifndef mail_queue_H_
#define mail_queue_H_

#include "mutex.h"

#include "mail/mail_send.h"

#include "ZThread/thread.h"

#include <deque>


using namespace ZThread;


class mailQueue 
{
private:
	
	class mailQueueData
	{

	protected:

		XP_MUTEX				m_lock;

		int						m_milliSpeed;

		int						m_errorFrequency; // how many errors before a mail logs an error (modulo)
	
		bool					m_quit;

		void					*m_pParent;

	public:
		
		std::deque<mailSend>	m_mailQueue;
	
		mailQueueData(void * pPrnt) :
			m_milliSpeed(2000),
			m_errorFrequency(300),
			m_quit(false)
		{
			XP_MUTEX_INIT(&m_lock);

			//* default values
			//* m_milli_speed = 2000, 
			//* m_errorFrequency = 300

			m_pParent = pPrnt;
		};

		void init(int nSpeed, int nFreq)
		{
			m_milliSpeed = (nSpeed < 0) ? 0 : nSpeed;
			m_errorFrequency = (nFreq < 1) ? 1 : nFreq;
		};

		int	milliSpeed()
		{
			return m_milliSpeed;
		};

		int errorFreq()
		{
			return m_errorFrequency;
		};

		bool quit()
		{
			return m_quit;
		};

		void quit(bool bQuit)
		{
			m_quit = bQuit;
		};

		void mutexLock()
		{
			XP_MUTEX_LOCK(&m_lock);
		};

		void mutexUnlock()
		{
			XP_MUTEX_UNLOCK(&m_lock);
		};

		void queueEmpty()
		{
			m_mailQueue.clear();
		};
	};

protected:

	mailQueueData			*m_pQueueData;

	bool					m_bInitialized;
	
	Task					*m_pQueueTask;
	Thread					*m_pQueueHandler;


public:

    class mailQueueHandler :  public Runnable
    {

    protected:

		mailQueueData	*m_pQueueData;

    public:

		mailQueueHandler(mailQueueData	*pQueueData) :
			m_pQueueData(pQueueData)
		{

		};

		~mailQueueHandler()
		{

		};

		void run();
	};

public:		

	mailQueue() :
		m_bInitialized(false),
		m_pQueueData(NULL)
	{
		m_pQueueTask = NULL;
		m_pQueueHandler = NULL;

		m_pQueueData = new 
			mailQueueData((void *) this);
	};

	~mailQueue()
	{
		if (m_pQueueData != NULL)
		{
			m_pQueueData->mutexLock();
			m_pQueueData->quit(true);
			m_pQueueData->mutexUnlock();
		}

		if (m_pQueueHandler != NULL)
		{
			try
			{
				m_pQueueHandler->wait();
			}
			catch(...)
			{
				m_pQueueHandler->cancel();
			}

			delete m_pQueueHandler;
			m_pQueueHandler = NULL;
		}

		if (m_pQueueTask != NULL)
		{
			delete m_pQueueTask;
			m_pQueueTask = NULL;
		}

		if (m_pQueueData != NULL)
		{
			delete m_pQueueData;
			m_pQueueData = NULL;
		}
	}

	void config(int nSpeed, int nFreq)
	{
		m_pQueueData->init(nSpeed, nFreq);
	};

	void init()
	{
		if (m_pQueueData == NULL || m_bInitialized == true)
		{
			return;
		}

		//* create a "Task" class for the "mailQueueHandler"

		try
		{
			m_pQueueTask = new Task(new mailQueueHandler(m_pQueueData));
		}
		catch(...)
		{
			m_pQueueTask = NULL;
			return;
		}

		m_bInitialized = true;
	};

	int start()
	{
		if (m_bInitialized == false)
		{
			return 1;
		}

		try
		{
			if (m_pQueueHandler != NULL)
			{
				delete m_pQueueHandler;
				m_pQueueHandler = NULL;
			}

			//* create new "Thread" class for previously created "Task" class
			m_pQueueHandler = new Thread(*m_pQueueTask, false);
			m_pQueueHandler->wait();
		}
		catch(...)
		{
			return -1;
		}

		return 0;
	};

	void queue(const mailSend &mail);
	
	void quit();

	void cancel();
};

#endif
