/// 
/// \file       main.cpp
/// 
///             The application "main" definition file
///             for the IPV test "reader" app.
///


#include <cstdio>
#include <filesystem>
#include <chrono>
#include <thread>
#include <condition_variable>

#include <Logging/Logging.h>
#include <Thread/ThreadBase.h>
#include <KbInput/CConsoleMgr.h>
#include <FileIO/CAudioFileIO.h>

#include "../Common/AppUtils.h"

#include "../../Src/IPC/CIpcMgr.h"    

#include "AppGlobalsCls.h"


#define IPC_IO_TIMEOUT                  1000                          // 0 = no timeout, > 0 = MS
#define DATA_BLOCK_SIZE                 1024                        // This is the number of 156 bit words (samples)
#define BUFFER_SIZE_IN_BYTES            (DATA_BLOCK_SIZE * 2)       // This is the number of bytes (words * 2)

#define IPC_TEST_NAME                   "/ipcBuffer"

#define WRITE_TO_OUTPUT_WHILE_STOPPED   true


enum eIpcState_def
{
    eIpcState_unknown = 0,
    eIpcState_waitingForWriter,
    eIpcState_writerConnected

};


AppGlobalsCls g_globals;   // global app variables and functions


bool handleKbInput(CConsoleMgr &kbMgr, bool &bExitApp, CIpcReader *pReader, bool &bPlayActive)
{
    int keysPressed = (int) kbMgr.getNumKeyPress();
    if (keysPressed > 0)
    {
        char cKeyPress = (char) kbMgr.getachar();

        if (cKeyPress == 'e' || cKeyPress == 'E')
        {
            LogInfo("App exit key (E) pressed. App terminating.");

            bExitApp = true;
        }

        if (cKeyPress == 'p' || cKeyPress == 'P')
        {
            LogInfo("Flow control Start key (P) pressed. Sending start to writer.");

            if (pReader != nullptr)
            {
                //if(pReader->isWriterConnected() == true)
                {
                    pReader->setStarted(true);

                    bPlayActive = true;
                }
                //else
                //{
                //    LogInfo("Writer not connected - Can't send start to writer.");
                //}
            }
            else
            {
                LogInfo("Reader not created - Can't send start to writer.");
            }
        }

        if (cKeyPress == 's' || cKeyPress == 'S')
        {
            LogInfo("Flow control Stop key (S) pressed. Sending stop to writer.");

            if (pReader != nullptr)
            {
                //if(pReader->isWriterConnected() == true)
                {
                    pReader->setStarted(false);

                    bPlayActive = false;
                }
                //else
                //{
                //    LogInfo("Writer not connected - Can't send stop to writer.");
                //}
            }
            else
            {
                LogInfo("Reader not created - Can't send stop to writer.");
            }
        }

        return true;
    }

    return false;
}


typedef struct KbThreadData_tag
{
    CConsoleMgr     &m_kbMgr;

    bool            &m_bExitApp;

    CIpcReader      *m_pReader;

    bool            &m_bPlayActive;

    KbThreadData_tag(CConsoleMgr &kbMgrRef, bool &bExitAppRef, bool &bPlayActiveRef) : 
        m_kbMgr(kbMgrRef), 
        m_bExitApp(bExitAppRef),
        m_bPlayActive(bPlayActiveRef),
        m_pReader(nullptr)
    {
    }

    bool setReader(CIpcReader *pReader)
    {
        m_pReader = pReader;

        return true;
    }

} KbThreadData_def;


using namespace ThreadBaseDefs;


class CKbHandlerThread : 
    public CThreadBase
{

    KbThreadData_def *m_pThreadData;
    
public:

    CKbHandlerThread() : 
        CThreadBase("KbHandler"),
        m_pThreadData(nullptr)
    {
        LogDebug("Creating keyboard input handler class ");
    }

    ~CKbHandlerThread()
    {
        LogDebug("Destroying keyboard input handler class ");
    }

    bool init(KbThreadData_def *pThreadData)
    {
        if (pThreadData == nullptr)
            return false;

        m_pThreadData = pThreadData;

        return true;
    }

    bool start()
    {
        if (m_pThreadData == nullptr)
            return false;

        return CThreadBase::createThread();
    }

    void threadProc()
    {
        if (m_pThreadData == nullptr)
            return;

        LogInfo("Starting keyboard input handler thread... ");
        
        while (m_pThreadData->m_bExitApp == false)    
        {
            handleKbInput
            (
                m_pThreadData->m_kbMgr, 
                m_pThreadData->m_bExitApp, 
                m_pThreadData->m_pReader,
                m_pThreadData->m_bPlayActive
            );

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        LogInfo("Keyboard input handler thread exiting... ");
    }

};


/// 
/// \brief Main application entry point.
/// This is used for:
///     - launching the software as a standard app
///     - launching the softwate as a system service
///     - launching the software as a daemon
/// 
/// \fn int appMain(int argc, char *argv[])
/// \param arg      Arg count
/// \param argv     Pointer to each command line arg string
/// \returns        Error code (0 = none)
/// 
int appMain(int argc, char *argv[])
{
    // Initialize the application (app name, logging, etc...)

    auto status = g_globals.appInit("ipcTest-reader", "1.0");
 
    if (status == false)
    {
        LogError("App / logger initialization failed");
        return -1;
    }

    LogInfo("Application: {} - Build date: {} ", g_globals.m_sAppName, STRINGIFY(__DATE__));
    
#ifdef DEBUG
    LogInfo("DEBUG build - IPC trace logging enabled ");
#else
    LogInfo("Release build");
#endif

    LogInfo("Current application dir: {} ", g_globals.m_sCWD);

    // Initialize app configuration

    LogInfo("Parsing application command line... ");

    if (g_globals.parseCommandLine(argc, argv) == false)
    {
        LogError("Error parsing command line arguments");
    }

    g_globals.setLoggerConfig();

    LogInfo("Log level set to: {} ", g_globals.m_nLogLevel);

    LogInfo("Block size set to: {} ", DATA_BLOCK_SIZE);

    if (g_globals.m_sTestOutputFile == "")
    {
        LogError("Test output file not specified");
        return -2;
    }

    // Open IO...

    LogInfo("Opening test output file: {} ", g_globals.m_sTestOutputFile);

    auto pOutputFile = 
        CAudioFileIO::openFileTypeByExt
        (
            g_globals.m_sTestOutputFile,
            eFileIoMode_def::eFileIoMode_output,
            1,
            48000,
            DATA_BLOCK_SIZE  
        );

    if (pOutputFile == nullptr)
    {
        LogError("Failed to open test output file: {} ", g_globals.m_sTestOutputFile);
        return -3;
    }

    char cKeyPress;

    CConsoleMgr kbMgr;

    kbMgr.dsableKbBuffering(true);
    kbMgr.dsableKbEcho(true);

    bool bExitApp = false;
    bool bExitProcessingLoop = false;
    bool bPlayActive = false;
    bool bLastPlayState = false;

    KbThreadData_def threadData(kbMgr, bExitApp, bPlayActive);

    CKbHandlerThread kbThread;

    if (kbThread.init(&threadData) == false)
    {
        LogError("Failed to initialize keyboard handler thread");

        return -5;
    }

    LogInfo("Opening IPC input channel: {}, MaxBlkSize: {}, Primary: false ", IPC_TEST_NAME, BUFFER_SIZE_IN_BYTES);

    CIpcReader  reader(IPC_TEST_NAME, BUFFER_SIZE_IN_BYTES, false);

    threadData.setReader(&reader);

    eIpcState_def eIpcState = eIpcState_def::eIpcState_unknown;

    int8_t buffer[BUFFER_SIZE_IN_BYTES];

    memset(buffer, 0, BUFFER_SIZE_IN_BYTES);

    if (kbThread.start() == false)  
    {
        LogError("Failed to start keyboard handler thread");

        return -6;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::cout << "Press 'E' to exit. \n";
    std::cout << "Press 'P' to start flow control. \n";
    std::cout << "Press 'S' to stop flow control. \n";

    unsigned int nWriteSize = 0;

    while (bExitApp == false)
    {
        // Wait for IPC channel open

        auto bOpened = reader.isOpen();

        while (bOpened == false && bExitApp == false)
        {
            if (eIpcState != eIpcState_def::eIpcState_unknown)
            {
                LogInfo("Waiting for IPC writer to open connection... ");
                eIpcState = eIpcState_def::eIpcState_unknown;
            }

            // if no input from writer... write 0's to file

            memset(buffer, 0, BUFFER_SIZE_IN_BYTES);

            if (pOutputFile->writeBlock(buffer, BUFFER_SIZE_IN_BYTES) == false)
            {
                bExitApp = false;
                LogWarning("FileIO:writeBlock returned false");
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            bOpened = reader.open();
        }

        if (bExitApp == true)
        {
            reader.resetReaderConnected();
            break;
        }

        // Wait for IPC writer to connect

        bool bWriterConnected = reader.isWriterConnected();
        while (bWriterConnected == false && bExitApp == false)
        {
            // if writer not currently connected... write 0's to file

            memset(buffer, 0, BUFFER_SIZE_IN_BYTES);

            if (pOutputFile->writeBlock(buffer, DATA_BLOCK_SIZE) == false)
            {
                break;
            }

            if (eIpcState != eIpcState_def::eIpcState_waitingForWriter)
            {
                LogInfo("Waiting for IPC writer to connect... ");
                eIpcState = eIpcState_def::eIpcState_waitingForWriter;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            bWriterConnected = reader.isWriterConnected();
        }

        eIpcState = eIpcState_def::eIpcState_writerConnected;

        LogInfo("IPC writer connected... ");

        if (g_globals.m_bSetIpcStartOnLaunch == true)
        {
            if (reader.isStarted() == false)
            {
                LogInfo("Setting flow control to (auto) start on app launch");

                reader.setStarted(true);

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            bPlayActive = true;

        }

        // Read from IPC and write to output file

        bExitProcessingLoop = false;

        LogInfo("Starting main reader processing loop... ");

        while (bExitProcessingLoop == false)
        {
            if (bExitApp == true)
            {
                break;
            }

            int readStatus = 0;

            bool bWriterConnected = reader.isWriterConnected();
            if (bWriterConnected == false)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(4));

                if (eIpcState == eIpcState_def::eIpcState_writerConnected)
                {
                    LogInfo("Writer had disconnected - reseting");
                    eIpcState = eIpcState_def::eIpcState_waitingForWriter;
                    reader.setStarted(false);
                    reader.resetReaderConnected();
                    reader.close();
                    bExitProcessingLoop = true;
                }

                continue;
            }

            if (bPlayActive != bLastPlayState)
            {
                bLastPlayState = bPlayActive;

                std::string sFlowControlStr = 
                    (bPlayActive == true) ? "Started" : "Stopped";

                LogInfo("Reader flow control updated to: {} ", sFlowControlStr);
            }

            if (bPlayActive == true)
            {
                // Read a block of data from IPC

                readStatus = reader.read(buffer, BUFFER_SIZE_IN_BYTES, IPC_IO_TIMEOUT); 
                if (readStatus < 1)
                {
                    switch (readStatus)
                    {
                    case -1:
                        LogError("IPC Read failed - IPC channel not open");
                        bPlayActive = false;
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -2:
                        LogError("IPC Read failed - param error");
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -3:
                        LogError("IPC Read failed - Ctrl ptr error");
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -4:
                        LogError("IPC Read failed - Data ptr error");
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -5:
                        LogError("IPC Read failed - Writer not connected");
                        bWriterConnected = false;
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -6:
                        LogError("IPC Read failed - IPC not started");
                        bPlayActive = false;
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -7:
                        LogError("IPC Read failed - wait timeput");
#if 0                        
                        bPlayActive = false;
                        reader.close();
                        //bExitProcessingLoop = true;
#endif
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -8:
                        LogError("IPC Read failed - No data");
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -10:
                        LogError("IPC Read failed - unknown exception");
                        bPlayActive = false;
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;
                    }

                    nWriteSize = 0;
                }
                else
                {
                    nWriteSize = (unsigned int) (readStatus / 2);
                }
            }
            else
            {
                nWriteSize = 0;
            }

            if ((bPlayActive == true) && (nWriteSize > 0))
            {
                // Read successful...

                if (nWriteSize < DATA_BLOCK_SIZE)
                {
                    LogWarning("Read < full block from IPC channel - readSize: {}, expected: {} ", nWriteSize, DATA_BLOCK_SIZE);
                }

                // Write data to file

                if (pOutputFile->writeBlock(buffer, nWriteSize) == false)
                {
                    LogError("Failed to write block to output file - exiting");
                    reader.setStarted(false);
                    reader.resetReaderConnected();
                    bExitApp = true;
                    bExitProcessingLoop = true;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            else
            {
                if (WRITE_TO_OUTPUT_WHILE_STOPPED)
                {
                    // writer not connected or read failed
                    // write zeroes to file

                    memset(buffer, 0, BUFFER_SIZE_IN_BYTES);

                    if (pOutputFile->writeBlock(buffer, DATA_BLOCK_SIZE) == false)
                    {
                        LogError("Failed to write block to output file - exiting");
                        reader.setStarted(false);
                        reader.resetReaderConnected();
                        bExitApp = true;
                        bExitProcessingLoop = true;
                    }
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
            }
        } 
    }
    
    LogInfo("Exiting main processing loop... ");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (kbThread.isActive())
    {
        kbThread.stopThread();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    reader.close();

    if (pOutputFile != nullptr)
    {
        pOutputFile->closeFile();
    }

    kbMgr.restore();
       
    return 0;
}


/// 
/// \brief This is the entry point when this software is
/// launched as a standard application.
/// 
/// \fn int main(int argc, char *argv[])
/// \param argc     Arg count
/// \param argv     Pointer to each command line arg string
/// \returns        Error code (0 = none)
/// 
int main(int argc, char *argv[])
{
    // Catch any stray / unhandled exceptions and log them.
    // NOTE: If spdlog has not have been initialized yet, 
    // it will still log to stderr.

    try
    {
        auto retValue = appMain(argc, argv);

        g_globals.m_logger.Flush();

        return retValue;
    }
    catch (const std::exception &e)
    {
        LogCritical("unhandled exception={}", e.what());

        return 1;
    }
    catch (...)
    {
        LogCritical("unhandled exception, unknown");
        
        return 1;
    }
}


