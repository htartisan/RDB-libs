/// 
/// \file   CIpcMgr.h
/// 
///         CIpc class defs
///


#ifndef C_IPC_MGR_HPP
#define C_IPC_MGR_HPP

#include <string>
#include <iostream>
#include <cstring>

#ifdef WINDOWS

#include <winbase.h>
//#include <memoryapi.h>

#else

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

#endif


#ifdef DEBUG
#define IPC_TRACE_LOGGING
#endif

#ifdef INCLUDE_APP_LOGGING
#include "Logging/Logging.h"
#endif


#include "../String/StrUtils.h"


#define IPC_IO_TIMEOUT_ERROR    -99


#ifndef IPC_LOGGING_DEFS
#define IPC_LOGGING_DEFS

namespace IpcLog 
{

inline void WarningMsg(const std::string sMsg)
{
#ifndef INCLUDE_APP_LOGGING
    std::cerr << sMsg << "\n";
#else
    LogWarning(sMsg);
#endif
}

inline void ErrorMsg(const std::string sMsg)
{
#ifndef INCLUDE_APP_LOGGING
    std::cerr << sMsg << "\n";
#else
    LogError(sMsg);
#endif
}

inline void DebugMsg(const std::string sMsg)
{
#ifdef INCLUDE_APP_LOGGING
    LogDebug(sMsg);
#endif
}

inline void TraceMsg(const std::string sMsg)
{
#ifdef IPC_TRACE_LOGGING

#ifndef INCLUDE_APP_LOGGING
    std::cerr << sMsg << "\n";
#else
    LogInfo(sMsg);
#endif

#endif
}

};  //  namespace IpcLog  

#endif  //  IPC_LOGGING_DEFS


#ifndef TIME_EXPIRED_FUNC
#define TIME_EXPIRED_FUNC

namespace TsUtils 
{

bool hasTimeExpired(const timespec& tsCur, const timespec& tsEnd)
{
    if (tsCur.tv_sec < tsEnd.tv_sec)
        return false;

    if (tsCur.tv_sec > tsEnd.tv_sec)
        return true;

    // lhs.tv_sec = rhs.tv_sec

    if (tsCur.tv_nsec < tsEnd.tv_nsec)
        return false;

    return true;
}

};

#endif


//
//  IpcCtrl_def struct definition
//

typedef struct IpcCtrl_tag
{
    uint32_t        m_maxDataSize;

    bool            m_primaryConnected;
    bool            m_secondaryConnected;
    bool            m_started;
    bool            m_hasData;

    uint32_t        m_currDataSize;

    uint64_t        m_numDataBlocksWritten;

} IpcCtrl_def;


//
//  IpcSharedData_def struct definition
//

typedef struct IpcSharedData_tag
{

#ifndef WINDOWS

    pthread_mutex_t m_ipcMutex;

    pthread_cond_t  m_readCV;
    pthread_cond_t  m_writeCV;

#endif

    IpcCtrl_def  	m_ctrl;

} IpcSharedData_def;



#ifdef WINDOWS

#define MUTEX_LOCK			WaitForSingleObject(m_ipcMutex, INFINITE)
#define MUTEX_UNLOCK		ReleaseMutex(m_ipcMutex)

#define WAIT_FOR_READ_CV()	WaitForSingleObject(m_readCV, INFINITE)
#define WAIT_FOR_WRITE_CV()	WaitForSingleObject(m_writeCV, INFINITE)

#define SIGNAL_READ_CV		ReleaseSemaphore(m_readCV, 1, NULL)
#define SIGNAL_WRITE_CV		ReleaseSemaphore(m_writeCV, 1, NULL)

#define RETURN_SUCCESS		(0x00000000L)

#else

#define MUTEX_LOCK			pthread_mutex_lock(&m_pIpcSharedMem->m_ipcMutex)
#define MUTEX_UNLOCK		pthread_mutex_unlock(&m_pIpcSharedMem->m_ipcMutex)

#define WAIT_FOR_READ_CV()	pthread_cond_wait(&m_pIpcSharedMem->m_readCV, &m_pIpcSharedMem->m_ipcMutex)
#define WAIT_FOR_WRITE_CV()	pthread_cond_wait(&m_pIpcSharedMem->m_writeCV, &m_pIpcSharedMem->m_ipcMutex)

#define SIGNAL_READ_CV		pthread_cond_signal(&m_pIpcSharedMem->m_readCV)
#define SIGNAL_WRITE_CV		pthread_cond_signal(&m_pIpcSharedMem->m_writeCV);

#define RETURN_SUCCESS		(0)

#endif



//
//  CIpcMgr base class definition
//

class CIpcMgr
{
private:

    std::string         m_name;
    
#ifdef WINDOWS

	HANDLE				m_smHandle;

#else

	int                 m_smHandle;

#endif

    int                 m_ipcSecurity;   
    
    IpcSharedData_def  *m_pIpcSharedMem;

#ifdef WINDOWS

	HANDLE				m_ipcMutex;

	HANDLE				m_readCV;
	HANDLE				m_writeCV;

#endif
    
    void                *m_pIpcData;

    bool                m_isCreator;
    bool                m_isInitialized;
    bool                m_isOpen;
    
    unsigned int        m_maxDataSize;

	std::string			m_sLastError;

public:

    CIpcMgr() :
		m_name(""),
		m_ipcSecurity(0666),
		m_isCreator(false),
		m_maxDataSize(0),
		m_pIpcSharedMem(nullptr),
		m_pIpcData(nullptr),
		m_isInitialized(false),
		m_isOpen(false),
		m_sLastError("")
	{
#ifdef WINDOWS

		m_smHandle = NULL;

#else

		m_smHandle = -1;

#endif
	}

    CIpcMgr(const std::string shmName, unsigned int maxDataSize, bool isCreator = false) :
		m_name(shmName), 
		m_ipcSecurity(0666),
		m_isCreator(isCreator),
		m_maxDataSize(maxDataSize),
		m_pIpcSharedMem(nullptr),
		m_pIpcData(nullptr),
		m_isOpen(false),
		m_sLastError("")
	{
#ifdef WINDOWS

		m_smHandle = NULL;

#else

		m_smHandle = -1;

#endif

		if (m_name != "" && maxDataSize != 0)
		{
			m_isInitialized = true;
		}
		else
		{
			m_isInitialized = false;
		}
	}

    ~CIpcMgr() 
	{
		if (m_isOpen == true)
		{
			close();
		}
	}

	std::string getLastErrorStr()
	{
		std::string sOut = m_sLastError;

		m_sLastError = "";

		return sOut;
	}

    bool init(const std::string shmName, unsigned int maxDataSize, bool isCreator = false)
	{
		if (shmName.empty() || maxDataSize < 1)
		{
			return false;
		}  

		m_name = shmName;
		m_isCreator = isCreator;
		m_maxDataSize = maxDataSize;
		m_pIpcSharedMem = nullptr;
		m_pIpcData = nullptr;
		m_isOpen = false;

		m_isInitialized = true;

		return true;
	}

    bool setIpcSecurity(int ipcSecurity)
	{
		m_ipcSecurity = ipcSecurity;

		return true;
	}

    bool open()
	{
		if (m_isInitialized == false || m_isOpen == true || m_ipcSecurity == 0)
		{
			m_sLastError = ("open: invalid param");
			IpcLog::DebugMsg(m_sLastError);
			return false;
		}

		if (m_name == "")
		{
			m_sLastError = ("Invalid name for shared memory IPC channel: " + m_name);
			IpcLog::DebugMsg(m_sLastError);
			return false;
		}

		if (m_maxDataSize < 1)
		{
			m_sLastError = ("Invalid max data size specified for shared memory IPC channel: " + m_name);
			IpcLog::DebugMsg(m_sLastError);
			return false;
		}

		unsigned int ipcHeaderSize = sizeof(IpcSharedData_def);

		unsigned int sharedMemSize = (ipcHeaderSize + m_maxDataSize + 8);

		if (m_isCreator) 
		{

#ifdef WINDOWS

			std::string sIpcName = "Local\\";
			sIpcName.append(m_name);
			sIpcName.append("-buffer");

			std::wstring wsIpcName = StrUtils::tows(sIpcName);

			m_smHandle = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sharedMemSize, wsIpcName.c_str());

			if (m_smHandle == NULL)
			{
				m_sLastError = ("Failed to truncate shared memory for: " + m_name);
				return false;
			}

#else

			// Create shared memory
			m_smHandle = shm_open(m_name.c_str(), O_CREAT | O_RDWR, m_ipcSecurity);

			if (m_smHandle != -1)
			{
				if (ftruncate(m_smHandle, sharedMemSize) == -1)
				{
					m_sLastError = ("Failed to truncate shared memory for: " + m_name);
					IpcLog::DebugMsg(m_sLastError);
					::close(m_smHandle);
					shm_unlink(m_name.c_str());
					return false;
				}
			}

#endif

		}
		else 
		{

#ifdef WINDOWS

			std::string sIpcName = "Local\\";
			sIpcName.append(m_name);
			sIpcName.append("-buffer");

			std::wstring wsIpcName = StrUtils::tows(sIpcName);

			m_smHandle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, false, wsIpcName.c_str());

#else

			// Open existing shared memory
			m_smHandle = shm_open(m_name.c_str(), O_RDWR, m_ipcSecurity);

#endif

		}

#ifdef WINDOWS
		if (m_smHandle == NULL)
#else
		if (m_smHandle < 0)
#endif
		{
			m_sLastError = ("Failed to open shared memory for: " + m_name);
			IpcLog::DebugMsg(m_sLastError);
			return false;
		}

		// Map the memory

#ifdef WINDOWS

		int8_t* pShMem = (int8_t*)
			MapViewOfFile(m_smHandle, FILE_MAP_ALL_ACCESS, 0, 0, sharedMemSize);

		if (pShMem == NULL)
		{
			m_sLastError = ("Failed to map shared memory for: " + m_name);
			IpcLog::DebugMsg(m_sLastError);
			return false;
		}

#else

		int8_t *pShMem = (int8_t *)
			mmap(0, sharedMemSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_smHandle, 0);

		if (pShMem == MAP_FAILED)
		{
			m_sLastError = ("Failed to map shared memory for: " + m_name);
			IpcLog::DebugMsg(m_sLastError);
			return false;
		}

#endif

		// Set the IPC shared memory header pointer to the start of the mapped memory

		m_pIpcSharedMem = (IpcSharedData_def*) pShMem;

		// Set the IPC data pointer to the first byte after the IpcSharedData_def struct

#ifdef WINDOWS

		std::string sMutexName = "Local\\";
		sMutexName.append(m_name);
		sMutexName.append("-mutex");

		std::wstring wsMutexName = StrUtils::tows(sMutexName);

		m_ipcMutex =
			CreateMutexW(NULL, false, wsMutexName.c_str());

		std::string sCvName = "Local\\";
		sCvName.append(m_name);
		sCvName.append("-readCV");

		std::wstring wsCvName = StrUtils::tows(sCvName);

		m_readCV = m_isCreator
			? CreateSemaphoreW(NULL, 0, LONG_MAX, wsCvName.c_str())
			: OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, false, wsCvName.c_str());

		sCvName = "Local\\";
		sCvName.append(m_name);
		sCvName.append("-writeCV");

		wsCvName = StrUtils::tows(sCvName);

		m_writeCV = m_isCreator
			? CreateSemaphoreW(NULL, 0, LONG_MAX, wsCvName.c_str())
			: OpenSemaphoreW(SEMAPHORE_ALL_ACCESS, false, wsCvName.c_str());

		if (m_ipcMutex == NULL || m_readCV == NULL || m_writeCV == NULL)
		{
			m_sLastError = ("Failed to create or open IPC synchronization objects for: " + m_name);
			UnmapViewOfFile(m_pIpcSharedMem);
			CloseHandle(m_smHandle);
			return false;
		}

		if (m_isCreator)
		{
			memset((void*)m_pIpcSharedMem, 0, ipcHeaderSize);

			m_pIpcSharedMem->m_ctrl.m_primaryConnected = true;
			m_pIpcSharedMem->m_ctrl.m_secondaryConnected = false;

			m_pIpcSharedMem->m_ctrl.m_maxDataSize = m_maxDataSize;

			m_pIpcSharedMem->m_ctrl.m_started = false;
			m_pIpcSharedMem->m_ctrl.m_hasData = false;

			m_pIpcSharedMem->m_ctrl.m_currDataSize = 0;
			m_pIpcSharedMem->m_ctrl.m_numDataBlocksWritten = 0;
		}
#else
		if (m_isCreator)
		{
			memset((void*)m_pIpcSharedMem, 0, ipcHeaderSize);

			// Initialize m_ipcMutex and condition variable for cross-process use

			pthread_mutexattr_t mutexAttr;

			if (pthread_mutexattr_init(&mutexAttr) != 0)
			{
				m_sLastError = ("Failed to initialize mutex attributes for: " + m_name);
				IpcLog::DebugMsg(m_sLastError);
				munmap(m_pIpcSharedMem, sharedMemSize);
				::close(m_smHandle);
				shm_unlink(m_name.c_str());
				return false;
			}
			if (pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED) != 0)
			{
				m_sLastError = ("Failed to set mutex attributes for process shared for: " + m_name);
				IpcLog::DebugMsg(m_sLastError);
				pthread_mutexattr_destroy(&mutexAttr);
				munmap(m_pIpcSharedMem, sharedMemSize);
				::close(m_smHandle);
				shm_unlink(m_name.c_str());
				return false;
			}

			if (pthread_mutex_init(&m_pIpcSharedMem->m_ipcMutex, &mutexAttr) != 0)
			{
				m_sLastError = ("Failed to initialize mutex for: " + m_name);
				IpcLog::DebugMsg(m_sLastError);
				pthread_mutexattr_destroy(&mutexAttr);
				munmap(m_pIpcSharedMem, sharedMemSize);
				::close(m_smHandle);
				shm_unlink(m_name.c_str());
				return false;
			}

			if (pthread_mutexattr_destroy(&mutexAttr) != 0)
			{
				m_sLastError = ("Failed to destroy mutex attributes for: " + m_name);
				IpcLog::DebugMsg(m_sLastError);
				munmap(m_pIpcSharedMem, sharedMemSize);
				::close(m_smHandle);
				shm_unlink(m_name.c_str());
				return false;
			}

			pthread_condattr_t cvAttr;

			if (pthread_condattr_init(&cvAttr) != 0)
			{
				m_sLastError = ("Failed to initialize condition variable attributes for: " + m_name);
				IpcLog::DebugMsg(m_sLastError);
				pthread_mutex_destroy(&m_pIpcSharedMem->m_ipcMutex);
				munmap(m_pIpcSharedMem, sharedMemSize);
				::close(m_smHandle);
				shm_unlink(m_name.c_str());
				return false;
			}
			if (pthread_condattr_setpshared(&cvAttr, PTHREAD_PROCESS_SHARED) != 0)
			{
				m_sLastError = ("Failed to set condition variable attributes for process shared for: " + m_name);
				IpcLog::DebugMsg(m_sLastError);
				pthread_condattr_destroy(&cvAttr);
				pthread_mutex_destroy(&m_pIpcSharedMem->m_ipcMutex);
				munmap(m_pIpcSharedMem, sharedMemSize);
				::close(m_smHandle);
				shm_unlink(m_name.c_str());
				return false;
			}

			if (pthread_cond_init(&m_pIpcSharedMem->m_readCV, &cvAttr) != 0)
			{
				m_sLastError = ("Failed to initialize read condition variable for: " + m_name);
				IpcLog::DebugMsg(m_sLastError);
				pthread_condattr_destroy(&cvAttr);
				pthread_mutex_destroy(&m_pIpcSharedMem->m_ipcMutex);
				munmap(m_pIpcSharedMem, sharedMemSize);
				::close(m_smHandle);
				shm_unlink(m_name.c_str());
				return false;
			}
			if (pthread_cond_init(&m_pIpcSharedMem->m_writeCV, &cvAttr) != 0)
			{
				m_sLastError = ("Failed to initialize write condition variable for: " + m_name);
				IpcLog::DebugMsg(m_sLastError);
				pthread_condattr_destroy(&cvAttr);
				pthread_mutex_destroy(&m_pIpcSharedMem->m_ipcMutex);
				munmap(m_pIpcSharedMem, sharedMemSize);
				::close(m_smHandle);
				shm_unlink(m_name.c_str());
				return false;
			}

			if (pthread_condattr_destroy(&cvAttr) != 0)
			{
				m_sLastError = ("Failed to destroy condition variable attributes for: " + m_name);
				IpcLog::DebugMsg(m_sLastError);
				pthread_mutex_destroy(&m_pIpcSharedMem->m_ipcMutex);
				munmap(m_pIpcSharedMem, sharedMemSize);
				::close(m_smHandle);
				shm_unlink(m_name.c_str());
				return false;
			}

			m_pIpcSharedMem->m_ctrl.m_primaryConnected = true;
			m_pIpcSharedMem->m_ctrl.m_secondaryConnected = false;

			m_pIpcSharedMem->m_ctrl.m_maxDataSize = m_maxDataSize;

			m_pIpcSharedMem->m_ctrl.m_started = false;
			m_pIpcSharedMem->m_ctrl.m_hasData = false;

			m_pIpcSharedMem->m_ctrl.m_currDataSize = 0;
			m_pIpcSharedMem->m_ctrl.m_numDataBlocksWritten = 0;
		}
#endif
		else
		{
			if (m_pIpcSharedMem->m_ctrl.m_maxDataSize > m_maxDataSize)
			{
				m_pIpcSharedMem->m_ctrl.m_maxDataSize = m_maxDataSize;
			}
			else
			{
				m_maxDataSize = m_pIpcSharedMem->m_ctrl.m_maxDataSize;
			}

			m_pIpcSharedMem->m_ctrl.m_secondaryConnected = true;
		}

		m_pIpcData = (void *) (pShMem + ipcHeaderSize);

		m_isOpen = true;

		return true;
	}

    bool close()
	{
		if (m_isOpen == false)
		{
			m_sLastError = ("Shared memory channel is already closed for: " + m_name);
			IpcLog::DebugMsg(m_sLastError);
			return false;
		}

#ifdef WINDOWS

		CloseHandle(m_ipcMutex);
		CloseHandle(m_readCV);
		CloseHandle(m_writeCV);

		if (m_isCreator)
		{
			m_pIpcSharedMem->m_ctrl.m_primaryConnected = false;
		}
		else
		{
			m_pIpcSharedMem->m_ctrl.m_secondaryConnected = false;
		}

		UnmapViewOfFile(m_pIpcSharedMem);

#else

		if (m_isCreator)
		{
			m_pIpcSharedMem->m_ctrl.m_primaryConnected = false;

			if (pthread_mutex_destroy(&m_pIpcSharedMem->m_ipcMutex) != 0)
			{
				m_sLastError = ("Failed to destroy mutex for: " + m_name);
				IpcLog::DebugMsg(m_sLastError);
				return false;
			}

			if (pthread_cond_destroy(&m_pIpcSharedMem->m_readCV) != 0)
			{
				m_sLastError = ("Failed to destroy read condition variable for: " + m_name);
				IpcLog::DebugMsg(m_sLastError);
				return false;
			}

			if (pthread_cond_destroy(&m_pIpcSharedMem->m_writeCV) != 0)
			{
				m_sLastError = ("Failed to destroy write condition variable for: " + m_name);
				IpcLog::DebugMsg(m_sLastError);
				return false;
			}
		}
		else
		{
			m_pIpcSharedMem->m_ctrl.m_secondaryConnected = false;
		}

		const unsigned int sharedMemSize = sizeof(IpcSharedData_def) + m_maxDataSize + 8;
		if (munmap(m_pIpcSharedMem, sharedMemSize) != RETURN_SUCCESS)
		{
			m_sLastError = ("Failed to unmap shared memory for: " + m_name);
			IpcLog::DebugMsg(m_sLastError);
			return false;
		}

#endif

		m_pIpcSharedMem = nullptr;

#ifdef WINDOWS

		CloseHandle(m_smHandle);

		m_smHandle = NULL;

#else

		if (::close(m_smHandle) != RETURN_SUCCESS)
		{
			m_sLastError = ("Failed to close shared memory handle for: " + m_name);
			IpcLog::DebugMsg(m_sLastError);
			return false;
		}

		m_smHandle = -1;

		if (m_isCreator) 
		{
			m_sLastError = ("Unlinking shared memory object: " + m_name);
			IpcLog::DebugMsg(m_sLastError);
			shm_unlink(m_name.c_str());
		}

#endif

		m_isOpen = false;

		return true;
	}

    bool isOpen()
	{
		return m_isOpen;
	}

	bool isCreator()
	{
		return m_isCreator;
	}

    bool isPrimaryConnected()
	{
		if (m_isOpen == false)
		{
			return false;
		}

		if (m_pIpcSharedMem == nullptr)
		{
			return false;
		}

		MUTEX_LOCK;

		auto bRet = m_pIpcSharedMem->m_ctrl.m_primaryConnected;

		MUTEX_UNLOCK;

		return bRet;
	}

    bool resetPrimaryConnected()
	{
		if (m_isOpen == false)
		{
			return false;
		}

		if (m_pIpcSharedMem == nullptr)
		{
			return false;
		}

		MUTEX_LOCK;

		m_pIpcSharedMem->m_ctrl.m_primaryConnected = false;

		MUTEX_UNLOCK;

		return true;
	}

    bool isSecondaryConnected()
	{
		if (m_isOpen == false)
		{
			return false;
		}

		if (m_pIpcSharedMem == nullptr)
		{
			return false;
		}

		MUTEX_LOCK;

		auto bRet = m_pIpcSharedMem->m_ctrl.m_secondaryConnected;

		MUTEX_UNLOCK;

		return bRet;
	}

    bool resetSecondaryConnected()
	{
		if (m_isOpen == false)
		{
			return false;
		}

		if (m_pIpcSharedMem == nullptr)
		{
			return false;
		}

		MUTEX_LOCK;

		m_pIpcSharedMem->m_ctrl.m_secondaryConnected = false;

		MUTEX_UNLOCK;

		return true;
	}

    bool setStarted(bool val)
	{
		if (m_isOpen == false)
		{
			return false;
		}

		if (m_pIpcSharedMem == nullptr)
		{
			return false;
		}

		MUTEX_LOCK;

		m_pIpcSharedMem->m_ctrl.m_started = val;

		MUTEX_UNLOCK;

		return true;
	}

    bool isStarted()
	{
		if (m_isOpen == false)
		{
			return false;
		}

		if (m_pIpcSharedMem == nullptr)
		{
			return false;
		}

		MUTEX_LOCK;

		auto bRet = m_pIpcSharedMem->m_ctrl.m_started;

		MUTEX_UNLOCK;

		return bRet;
	}

    int writeData(const void *pData, unsigned int nByteLen, unsigned int msTimeout) 
	{
		if (pData == nullptr || nByteLen < 1 || nByteLen > m_maxDataSize)
		{
			m_sLastError = ("writeData: Invalid parameter(s)");
			IpcLog::DebugMsg(m_sLastError);
			return -1;
		}

		if (m_pIpcSharedMem == nullptr)
		{
			m_sLastError = ("writeData: Shared memory channel is not mapped");
			IpcLog::DebugMsg(m_sLastError);
			return -2;
		}

		if (m_isOpen == false)
		{
			m_sLastError = ("writeData: Shared memory channel is not open");
			IpcLog::DebugMsg(m_sLastError);
			return -3;
		}

		if (m_pIpcSharedMem->m_ctrl.m_started == false)
		{
			m_sLastError = ("writeData: Shared memory channel is not started");
			IpcLog::DebugMsg(m_sLastError);
			return -4;
		}

		MUTEX_LOCK;

		// Check if we need to wait for the reader

		if (m_pIpcSharedMem->m_ctrl.m_hasData == true)
		{
			if (msTimeout == 0)
			{
				// Do CV wait operation without timeout

				while (m_pIpcSharedMem->m_ctrl.m_hasData == true) 
				{
					auto status = WAIT_FOR_WRITE_CV();
					if (status != RETURN_SUCCESS)
					{
						m_sLastError = ("writeData: Failed to wait on write condition variable");
						IpcLog::DebugMsg(m_sLastError);
						MUTEX_UNLOCK;
						return -5;
					}
				}
			}
#ifndef WINDOWS
			else
			{
				// Do CV wait operation with timeout

				struct timespec expTime;

				if (clock_gettime(CLOCK_REALTIME, &expTime) == -1)   // Get the current time
				{ 
					m_sLastError = ("writeData: Failed to get current time for timeout");
					IpcLog::DebugMsg(m_sLastError);
					MUTEX_UNLOCK;
					return -6;
				}

				expTime.tv_nsec += (msTimeout * 1000000L);          // Add timeout relative to current time    

				// Handle nanosecond overflow (if ns > 1 second)
				if (expTime.tv_nsec >= 1000000000L) 
				{
					expTime.tv_sec += expTime.tv_nsec / 1000000000L;
					expTime.tv_nsec = expTime.tv_nsec % 1000000000L;
				}

				int status = 0;

				while ((m_pIpcSharedMem->m_ctrl.m_hasData == true) && (status == 0))
				{
					struct timespec curTime;

					if (clock_gettime(CLOCK_REALTIME, &curTime) == -1)   // Get the current time
					{ 
						m_sLastError = ("writeData: Failed to get current time for timeout");
						IpcLog::DebugMsg(m_sLastError);
						MUTEX_UNLOCK;
						return -6;
					}

					// check for timeout

					if (TsUtils::hasTimeExpired(curTime, expTime) == true)
					{
						m_sLastError = ("writeData: Timeout");
						MUTEX_UNLOCK;
						return IPC_IO_TIMEOUT_ERROR;
					}

					// do cond_wait with timeout

					status = 
						pthread_cond_timedwait
						(
							&(m_pIpcSharedMem->m_writeCV), 
							&(m_pIpcSharedMem->m_ipcMutex), 
							&expTime
						);
				}

				int nErrCode = 0;
				
				switch (status)
				{
					case 0:             // no error
						nErrCode = 0;
						break;

					case EINVAL:        // Invalid param
						nErrCode = -7;
						break;

					case EPERM:         // Mutex access error
						nErrCode = -8;
						break;

					case ETIMEDOUT:     // Wait timeout
						nErrCode = IPC_IO_TIMEOUT_ERROR;
						break;
				}

				if (nErrCode != 0)
				{
					// Return error
					m_sLastError = ("writeData: Failed");
					IpcLog::DebugMsg(m_sLastError);
					MUTEX_UNLOCK;
					return nErrCode;
				}
			}
#endif
		}

		try
		{
			// Write source data to shared memory data area

			memcpy(m_pIpcData, pData, nByteLen);

			m_pIpcSharedMem->m_ctrl.m_currDataSize = nByteLen;

			m_pIpcSharedMem->m_ctrl.m_numDataBlocksWritten += 1;

			m_pIpcSharedMem->m_ctrl.m_hasData = true;
			
			SIGNAL_READ_CV;
		}
		catch(...)
		{
			m_sLastError = ("writeData: Exception occurred while writing data");
			IpcLog::DebugMsg(m_sLastError);
			MUTEX_UNLOCK;
			return -9;
		}

		MUTEX_UNLOCK;

		return nByteLen;
	}

    bool ipcHasData()
	{
		if (m_pIpcSharedMem == nullptr)
		{
			return false;
		}

		MUTEX_LOCK;

		auto bRet = m_pIpcSharedMem->m_ctrl.m_hasData;

		MUTEX_UNLOCK;

		return bRet;
	}

    uint64_t getNumDataBlocksWritten()
	{
		if (m_pIpcSharedMem == nullptr)
		{
			return 0;
		}

		if (m_isOpen == false)
		{
			return 0;
		}

		MUTEX_LOCK;

		auto nRet =  m_pIpcSharedMem->m_ctrl.m_numDataBlocksWritten;
		
		MUTEX_UNLOCK;

		return nRet;
	}

    void resetNumDataBlocksWritten()
	{
		if (m_pIpcSharedMem == nullptr)
		{
			return;
		}

		if (m_isOpen == false)
		{
			return;
		}

		MUTEX_LOCK;

		m_pIpcSharedMem->m_ctrl.m_numDataBlocksWritten = 0;
		
		MUTEX_UNLOCK;
	}

    int readData(void *pData, unsigned int nByteLen, unsigned int msTimeout)
	{
		if (pData == nullptr || nByteLen < 1 || nByteLen > m_maxDataSize)
		{
			m_sLastError = ("readData: Invalid parameter(s)");
			IpcLog::DebugMsg(m_sLastError);
			return -1;
		}

		if (m_pIpcSharedMem == nullptr)
		{
			m_sLastError = ("readData: Shared memory channel is not mapped");
			IpcLog::DebugMsg(m_sLastError);
			memset(pData, 0, nByteLen);
			return -2;
		}

		if (m_isOpen == false)
		{
			m_sLastError = ("readData: Shared memory channel is not open");
			IpcLog::DebugMsg(m_sLastError);
			memset(pData, 0, nByteLen);
			return -3;
		}

		if (m_pIpcSharedMem->m_ctrl.m_started == false)
		{
			m_sLastError = ("readData: Shared memory channel is not started");
			IpcLog::DebugMsg(m_sLastError);
			memset(pData, 0, nByteLen);
			return 0;
		}

		MUTEX_LOCK;

		memset(pData, 0, nByteLen);

		// Check if we need to wait for the writer

		if (m_pIpcSharedMem->m_ctrl.m_hasData == false)
		{
			if (msTimeout == 0)
			{
				// Do CV wait operation without timeout

				while (m_pIpcSharedMem->m_ctrl.m_hasData == false) 
				{
					auto status = WAIT_FOR_READ_CV();
					if (status != RETURN_SUCCESS)
					{
						m_sLastError = ("readData: Failed to wait on read condition variable");
						IpcLog::DebugMsg(m_sLastError);
						MUTEX_UNLOCK;
						return -5;
					}
				}
			}
#ifndef WINDOWS
			else
			{
				// Do CV wait operation with timeout

				struct timespec expTime;

				if (clock_gettime(CLOCK_REALTIME, &expTime) == -1)   // Get the current time
				{ 
					m_sLastError = ("readData: Failed to get current time for timeout");
					IpcLog::DebugMsg(m_sLastError);
					MUTEX_UNLOCK;
					return -6;
				}

				expTime.tv_nsec += (msTimeout * 1000000L);          // Add timeout relative to current time    

				// Handle nanosecond overflow (if ns > 1 second)
				if (expTime.tv_nsec >= 1000000000L) 
				{
					expTime.tv_sec += expTime.tv_nsec / 1000000000L;
					expTime.tv_nsec = expTime.tv_nsec % 1000000000L;
				}

				int status = 0;

				while ((m_pIpcSharedMem->m_ctrl.m_hasData == false) && (status == 0))
				{
					struct timespec curTime;

					if (clock_gettime(CLOCK_REALTIME, &curTime) == -1)   // Get the current time
					{ 
						m_sLastError = ("readData: Failed to get current time for timeout");
						IpcLog::DebugMsg(m_sLastError);
						MUTEX_UNLOCK;
						return -6;
					}

					// check for timeout

					if (TsUtils::hasTimeExpired(curTime, expTime) == true)
					{
						m_sLastError = ("readData: Timeout occurred while waiting for data");
						MUTEX_UNLOCK;
						return IPC_IO_TIMEOUT_ERROR;
					}

					// do cond_wait with timeout

					status = 
						pthread_cond_timedwait
						(
							&(m_pIpcSharedMem->m_readCV), 
							&(m_pIpcSharedMem->m_ipcMutex), 
							&expTime
						);
				}

				int nErrCode = 0;
				
				switch (status)
				{
					case 0:             // no error
						nErrCode = 0;
						break;

					case EINVAL:        // Invalid param
						nErrCode = -6;
						break;

					case EPERM:         // Mutex access error
						nErrCode = -7;
						break;

					case ETIMEDOUT:     // Wait timeout
						nErrCode = IPC_IO_TIMEOUT_ERROR;
						break;
				}

				if (nErrCode != 0)
				{
					// Return error
					m_sLastError = ("readData: Failed");
					IpcLog::DebugMsg(m_sLastError);
					MUTEX_UNLOCK;
					return nErrCode;
				}
			}
#endif
		}

		try
		{
			// Read target data from shared memory data area

			if (nByteLen > m_pIpcSharedMem->m_ctrl.m_currDataSize)
			{
				nByteLen = m_pIpcSharedMem->m_ctrl.m_currDataSize;
			}

			memcpy(pData, m_pIpcData, nByteLen);

			m_pIpcSharedMem->m_ctrl.m_currDataSize = 0;

			m_pIpcSharedMem->m_ctrl.m_hasData = false;

			SIGNAL_WRITE_CV;
		}
		catch(...)
		{
			m_sLastError = ("readData: Exception occurred while reading data");
			IpcLog::DebugMsg(m_sLastError);
			MUTEX_UNLOCK;
			return -9;
		}

		MUTEX_UNLOCK;

		return nByteLen;
	}
	
};


//
//  CIpcWriter class definition
//

class CIpcWriter : 
    protected CIpcMgr
{
    
public:
    CIpcWriter() :
		CIpcMgr("", false)
	{

	}

    CIpcWriter(const std::string sIpcName, unsigned int nMaxLen, bool bPrimary = false) :
		CIpcMgr(sIpcName, nMaxLen, bPrimary)
	{

	}

    ~CIpcWriter()
	{
		
	}

	std::string getLastErrorStr()
	{
		return CIpcMgr::getLastErrorStr();
	}

    bool init(const std::string sIpcName, unsigned int maxDataSize, bool isCreator = false)
	{
		return CIpcMgr::init(sIpcName, maxDataSize, isCreator);
	}

    bool setIpcSecurity(int ipcSecurity)
	{
		return CIpcMgr::setIpcSecurity(ipcSecurity);
	}

    bool open()
	{
		return CIpcMgr::open();
	}

    bool close()
	{
		return CIpcMgr::close();
	}

    bool isOpen()
	{
		return CIpcMgr::isOpen();
	}

    bool resetWriterConnected()
	{
		if (isCreator() == false)
		{
			return CIpcMgr::resetSecondaryConnected();
		}

		return CIpcMgr::resetPrimaryConnected();
	}

    bool isWriterConnected()
	{
		if (isCreator() == false)
		{
			return CIpcMgr::isPrimaryConnected();
		}

		return CIpcMgr::isPrimaryConnected();
	}

    bool resetReaderConnected()
	{
		if (isCreator() == false)
		{
			return CIpcMgr::resetSecondaryConnected();
		}

		return CIpcMgr::resetPrimaryConnected();
	}

    bool isReaderConnected()
	{
		if (isCreator() == false)
		{
			return CIpcMgr::isPrimaryConnected();
		}

		return CIpcMgr::isPrimaryConnected();
	}

    bool isStarted()
	{
		return CIpcMgr::isStarted();
	}

    bool setStarted(bool val)
	{
		return CIpcMgr::setStarted(val);
	}

    int write(void *pData, unsigned int nByteLen, unsigned int nTimeoutMs = 0)
	{
		return writeData(pData, nByteLen, nTimeoutMs);
	}

};


//
//  CIpcReader class definition
//

class CIpcReader : 
    protected CIpcMgr
{

public:
    CIpcReader() :
		CIpcMgr("", false)
	{

	}

    CIpcReader(const std::string sIpcName, unsigned int nMaxLen, bool bPrimary = false) :
		CIpcMgr(sIpcName, nMaxLen, bPrimary)
	{

	}

    ~CIpcReader()
	{
		
	}

	std::string getLastErrorStr()
	{
		return CIpcMgr::getLastErrorStr();
	}

	bool init(const std::string sIpcName, unsigned int maxDataSize, bool isCreator = false)
	{
		return CIpcMgr::init(sIpcName, maxDataSize, isCreator);
	}
	
	bool setIpcSecurity(int ipcSecurity)
	{
		return CIpcMgr::setIpcSecurity(ipcSecurity);
	}

    bool open()
	{
		return CIpcMgr::open();
	}

    bool close()
	{
		return CIpcMgr::close();
	}

    bool isOpen()
	{
		return CIpcMgr::isOpen();
	}

    bool resetWriterConnected()
	{
		if (isCreator() == false)
		{
			return CIpcMgr::resetSecondaryConnected();
		}

		return CIpcMgr::resetPrimaryConnected();
	}

    bool isWriterConnected()
	{
		if (isCreator() == false)
		{
			return CIpcMgr::isPrimaryConnected();
		}

		return CIpcMgr::isPrimaryConnected();
	}

    bool resetReaderConnected()
	{
		if (isCreator() == false)
		{
			return CIpcMgr::resetSecondaryConnected();
		}

		return CIpcMgr::resetPrimaryConnected();
	}

    bool isReaderConnected()
	{
		if (isCreator() == false)
		{
			return CIpcMgr::isPrimaryConnected();
		}

		return CIpcMgr::isPrimaryConnected();
	}

    bool isStarted()
	{
		return CIpcMgr::isStarted();
	}

    bool setStarted(bool val)
	{
		return CIpcMgr::setStarted(val);
	}

    int read(void *pData, unsigned int nByteLen, unsigned int nTimeoutMs = 0)
	{
		return readData(pData, nByteLen, nTimeoutMs);
	}

};


#endif  //  C_IPC_MGR_HPP
