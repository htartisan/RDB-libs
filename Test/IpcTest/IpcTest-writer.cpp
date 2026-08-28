/// 
/// \file       main.cpp
/// 
///             The application "main" definition file
///             for the IPV test "writer" app.
///


#include <cstdio>
#include <filesystem>
#include <chrono>
#include <thread>
#include <condition_variable>

#include <string>

#include <Logging/Logging.h>
#include <Thread/ThreadBase.h>
#include <KbInput/CConsoleMgr.h>
#include <FileIO/CAudioFileIO.h>

#include "../Common/AppUtils.h"

#include "../../Src/IPC/CIpcMgr.h"    

#include "AppGlobalsCls.h"


#define IPC_IO_TIMEOUT              1000                          // 0 = no timeout, > 0 = MS
#define DATA_BLOCK_SIZE             1024                        // This is the number of 156 bit words (samples)
#define BUFFER_SIZE_IN_BYTES        (DATA_BLOCK_SIZE * 2)       // This is the number of bytes (words * 2)

#define IPC_TEST_NAME               "/ipcBuffer"

#define GENERATE_TEST_DATA          "generateTestData"


enum eIpcState_def
{
    eIpcState_unknown = 0,
    eIpcState_waitingForReader,
    eIpcState_readerConnected

};


AppGlobalsCls g_globals;   // global app variables and functions


bool handleKbInput(CConsoleMgr &kbMgr, bool &bExitApp)
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

        return true;
    }

    return false;
}


typedef struct KbThreadData_tag
{

    CConsoleMgr     &m_kbMgr;

    bool            &m_bExitApp;

    KbThreadData_tag(CConsoleMgr &kbMgrRef, bool &bExitAppRef) : 
        m_kbMgr(kbMgrRef), 
        m_bExitApp(bExitAppRef)
    {
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
        LogDebug("Distroying keyboard input handler class ");
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
            handleKbInput(m_pThreadData->m_kbMgr, m_pThreadData->m_bExitApp);

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

    auto status = g_globals.appInit("ipcTest-writer", "1.0");
 
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

    if (g_globals.m_sTestInputFile == "")
    {
        LogError("Test input file not specified");
        return -2;
    }

    // Open IO...

    std::shared_ptr<CAudioFileIO> pInputFile = nullptr;

    if (g_globals.m_sTestInputFile == GENERATE_TEST_DATA)
    {
        LogInfo("Generating test input data... ");

        g_globals.m_bGenerateTestData = true;
    }
    else
    {
        LogInfo("Opening test input file: {} ", g_globals.m_sTestInputFile);

        pInputFile = 
            CAudioFileIO::openFileTypeByExt
            (
                g_globals.m_sTestInputFile,
                eFileIoMode_def::eFileIoMode_input,
                1,
                48000,
                DATA_BLOCK_SIZE  
            );

        if (pInputFile == nullptr)
        {
            LogError("Failed to open test input file: {} ", g_globals.m_sTestInputFile);
            return -3;
        }

        pInputFile->setLoopingRead(true);
    }

    CConsoleMgr kbMgr;

    kbMgr.dsableKbBuffering(true);
    kbMgr.dsableKbEcho(true);

    bool bExitApp = false;

    bool bStarted = false;

    KbThreadData_def threadData(kbMgr, bExitApp);

    CKbHandlerThread kbThread;

    if (kbThread.init(&threadData) == false)
    {
        LogError("Failed to initialize keyboard handler thread");

        return -5;
    }

    LogInfo("Opening IPC output channel: {}, MaxBlkSize: {}, Primary: true ", IPC_TEST_NAME, BUFFER_SIZE_IN_BYTES);

    CIpcWriter  writer(IPC_TEST_NAME, BUFFER_SIZE_IN_BYTES, true);

    if (writer.open() == false)
    {
        LogError("IPC writer open failed - exiting");
        LogError("CIpcWriter 'lastError' = {}", writer.getLastErrorStr());
        return -4;
    }

    int32_t lastSampleValue = 0;

    int8_t buffer[BUFFER_SIZE_IN_BYTES];

    memset(buffer, 0, BUFFER_SIZE_IN_BYTES);

    eIpcState_def eIpcState = eIpcState_def::eIpcState_unknown;

    bool bLastPlayState = false;
    bool bCurrPlayState = false;

    if (kbThread.start() == false)  
    {
        LogError("Failed to start keyboard handler thread");

        return -5;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "Press 'E' to exit. \n";

    while (bExitApp == false)
    {
        // Do initial wait for IPC reader to connect

        while (true)
        {
            if (eIpcState != eIpcState_def::eIpcState_waitingForReader)
            {
                LogInfo("Waiting for IPC reader to connect... ");
                eIpcState = eIpcState_def::eIpcState_waitingForReader;
            }

            if (writer.isReaderConnected() == true)
            {
                break;
            }

            if (bExitApp == true)
            {
                break;
            }
        }

        if (bExitApp == true)
        {
            break;
        }

        LogInfo("IPC reader connected... ");
        
        eIpcState = eIpcState_def::eIpcState_readerConnected;

        // Process the input file and write to IPC channel

        LogInfo("Starting main processing loop... ");

        while (true)
        {
            if (bExitApp == true)
            {
                break;
            }

            // Check if reader is still connected

            auto bConnected = writer.isReaderConnected();
            if (bConnected == false)
            {
                LogWarning("IPC reader disconnected... ");
                eIpcState = eIpcState_def::eIpcState_unknown;
                bCurrPlayState = false;
                break;
            }

            if (bExitApp == true)
            {
                break;
            }

            // Check flow control state from reader

            bCurrPlayState = writer.isStarted();
            if (bCurrPlayState != bLastPlayState)
            {
                std::string sFlowControlStr = 
                    (bCurrPlayState == true) ? "Started" : "Stopped";

                LogInfo("Reader flow control updated to: {} ", sFlowControlStr);
                
                bLastPlayState = bCurrPlayState;
            }

            bStarted = bCurrPlayState;

            if (bExitApp == true)
            {
                break;
            }

            // If flow control is "start"...

            if (bCurrPlayState == true)
            {
                if (g_globals.m_bGenerateTestData == true)
                {
                    // Generate test data - ramp from min to max value, then wrap around

                    int16_t *pSampleData = (int16_t *) buffer;

                    for (int i = 0; i < DATA_BLOCK_SIZE; i++)
                    {
                        *(pSampleData + i) = (int16_t) lastSampleValue;

                        lastSampleValue += 1;

                        if (lastSampleValue > 32766)
                        {
                            lastSampleValue = -32766;
                        }
                    }
                }
                else
                {
                    // If flow control is started, read from input file and write to IPC channel
                    if (pInputFile->readBlock(&buffer[0], DATA_BLOCK_SIZE) == false)
                    {
                        LogInfo("Finished processing test input file");
                        bExitApp = false;
                        break;
                    }
                }

                auto writeStatus = writer.write(buffer, BUFFER_SIZE_IN_BYTES, IPC_IO_TIMEOUT);
                if (writeStatus < 1)
                {
                    switch (writeStatus)
                    {
                    case -1:
                        LogError("IPC Write failed - IPC channel not open");
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -2:
                        LogError("IPC Write failed - param error");
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -3:
                        LogError("IPC Write failed - Ctrl ptr error");
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -4:
                        LogError("IPC Write failed - Data ptr error");
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -5:
                        LogError("IPC Write failed - Reader not connected");
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -6:
                        LogError("IPC Write failed - IPC not started");
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -7:
                        //LogError("IPC Write failed - wait timeout");
#if 0                        
                        writer.setWriterConnected(false); 
                        writer.close();
                        eIpcState = eIpcState_def::eIpcState_unknown;
                        eLastFlowControl = eIpcFlowControl_def::eIpcFlowControl_unknown;
#else
                        LogInfo("Retrying IPC Write");
                        writeStatus = writer.write(&buffer[0], BUFFER_SIZE_IN_BYTES, IPC_IO_TIMEOUT);
                        if (writeStatus > 0)
                        {
                            break;
                        }
#endif
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -8:
                        LogError("IPC Write failed - data not read");
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;

                    case -10:
                        LogError("IPC Write failed - unknown exception");
                        std::this_thread::sleep_for(std::chrono::milliseconds(2));
                        break;
                    }
                }
            }
            else
            {
                // If flow control is not "start", just sleep for 10 ms
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

    } 

    LogInfo("Exiting main processing loop... ");

    writer.resetWriterConnected();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (kbThread.isActive())
    {
        kbThread.stopThread();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    writer.close();

    if (pInputFile != nullptr)
    {
        pInputFile->closeFile();
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


