/// 
/// \file       main.cpp
/// 
///             The application "main" definition file
///             for the NetTxTest app.
///


#define _CRT_SECURE_NO_WARNINGS


#include <cstdio>
#include <filesystem>
#include <condition_variable>
#include <csignal>

#ifdef WINDOWS
#include <conio.h>
#else
#include <RDB-libs/Src/KbInput/KbInput.c>
#endif

//#include <RDB-libs/Src/NetIO/CNetworkIO.h>
#include <RDB-libs/Src/NetIO/CServerIO.h>

#include <RDB-libs/Src/String/StrUtils.h>

#include "AppGlobalsCls.h"

#include "AppUtils.h"


#include <ME-Common/Src/NetIoDataDefs.h>

#include <RDB-libs/Src/FileIO/CAudioFileIO.h>



// Set this option to 'true' to enable 
// multi-I/O TCP sessions

#define SET_TCP_MULTI_SESSION           true


#pragma pack(push, 1)

struct CmdInfo_def
{
    NetCmdHeader_def    cmdHeader;
    NetCmdData_def      cmdData;
};

#pragma pack(pop)


/// \brief Application 'globals' class
/// 
/// Internal application class to manage 
/// global data and global functions.
/// 
AppGlobalsCls           g_globals;


/// 
/// \brief Application 'signal' handler
/// 
/// This function is called if an application 
/// interrupt signal is forced by the OS.
/// 
/// \fn void signal_handler(int signal_num)
/// \param signal_num   'signal' number triggered by the OS
/// 
void signal_handler(int signal_num)
{
    if (signal_num == 2)
    {
        LogError("App 'break' signal received");

        g_globals.m_bExitApp = true;
    }
    else
    { 
        std::cout << "App interrupt signal: " << signal_num << " \n";
    }

    // It terminates the  program 
    exit(100 + signal_num);
}


/// 
/// \brief This function is called to diaplay app info
/// such as: 
///     - current working directory
///     - app version
///     - etc...
/// 
/// \fn void appInfo()
/// 
void appInfo()
{
    // TODO: Display version info

    LogInfo("Cureent app working dir: {}", g_globals.m_sCWD);

#if DEBUG
    LogInfo("compiled with DEBUG");
#endif
}


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
    bool status;

    // Setup application 'signal' handling

    std::signal(SIGINT, signal_handler),
        std::signal(SIGILL, signal_handler),
        std::signal(SIGFPE, signal_handler),
        std::signal(SIGSEGV, signal_handler),
        std::signal(SIGTERM, signal_handler),
#ifdef WINDOWS
        std::signal(SIGBREAK, signal_handler),
#else
        std::signal(SIGHUP, signal_handler),
#endif
        std::signal(SIGABRT, signal_handler);

    // Initialize the application (app name, logging, etc...)

    status = g_globals.appInit("NetTxTest", "1.0");
 
    if (status == false)
    {
        LogError("App / logger initialization failed");
        return -1;
    }

    appInfo();

    LogInfo("Parsing application command line");

    if (g_globals.parseCommandLine(argc, argv) == false)
    {
        LogError("Error parsing command line arguments");
    }

    g_globals.setLoggerConfig();

    LogInfo("Log level set to: {}", g_globals.m_AppConfigMgr.m_nLogLevel);

    g_globals.m_bExitApp = false;

    std::unique_lock<std::mutex> exitLock(g_globals.m_appExitMutex);

    unsigned int nLastNumActiveSessions = 0;

    do
    {
        g_globals.m_logger.Flush();

        LogInfo("Initializing Network IO interface...");

        CNetworkIO::CTcpServer  netOutput(CNetworkIO::eNetIoDirection::eNetIoDirection_output);

        unsigned int nPort = (unsigned int) stringToUint(g_globals.m_AppConfigMgr.m_sNetPort);

        netOutput.setPort(nPort);

        // main data processing loop

        char cKeyPress;

        bool status;

        bool bDisplayOpenStatus = true;
        bool bDisplayWaitingStatus = true;

        unsigned int nHeaderLen = 0;
        unsigned int nDataLen = 0;

        // audio file variables

        unsigned int nBlockSize = 1024;
        unsigned int nSampleRate = 48000;
        unsigned int nNumChannels = 2;

        int16_t *pAudioBuffer = nullptr;

        std::shared_ptr<CAudioFileIO> m_pInputFile;

        eAudioFileType_def eFileType = eAudioFileType_def::eFileType_unknown;

        std::chrono::high_resolution_clock::time_point lastTimeStamp;
        std::chrono::high_resolution_clock::time_point nextSampleTime;

        unsigned long ulAudioBlockInterval = 0;

        // Cmd variables

        bool bCmdMode = true;
        bool bCmdEntered = false;

        CmdInfo_def  cmdInfo;

        cmdInfo.cmdHeader.m_nCmdCnt = 1;
        cmdInfo.cmdHeader.m_nNumCmds = 99;
        cmdInfo.cmdHeader.m_nCmdDataSize = sizeof(NetCmdHeader_def);
        cmdInfo.cmdHeader.m_nCmdEntrySize = sizeof(NetCmdData_def);

        // *************************

        LogInfo("Processing input & commandds...");

        auto sTmp = StrUtils::toLower(g_globals.m_AppConfigMgr.m_sTestType);

        if (sTmp == "cmd")
        {
            bCmdMode = true;

            nHeaderLen = sizeof(NetCmdHeader_def);
            nDataLen = sizeof(NetCmdData_def);
            
            netOutput.setDataType("NetCmd");
 
            std::cout << "Mode: 'NetCmd' - Press 'C' to enter a Cmd, or press 'E' to exit. \n";
        }
        else if (sTmp == "audio")
        {
            bCmdMode = false;
            
            nHeaderLen = sizeof(NetAudioHeader_def);

            netOutput.setDataType("NetAudio");

            // load the input data file to get the number of channels

            m_pInputFile =
                CAudioFileIO::openFileTypeByExt
                (
                    g_globals.m_AppConfigMgr.m_sAudioFile,
                    eFileIoMode_def::eFileIoMode_input
                );
            if (m_pInputFile == nullptr)
            {
                LibLogError("[audioFileIn] failed to open audio input file: {}", g_globals.m_AppConfigMgr.m_sAudioFile);
                return -10;
            }

            eFileType = getAudioFileType(g_globals.m_AppConfigMgr.m_sAudioFile);
            
            m_pInputFile->setLoopingRead(true);

            m_pInputFile->setIoBlockSize(nBlockSize);

            if (eFileType == eAudioFileType_def::eFileType_raw)
            { 
                // if 'raw' file, set default sample rate & num chls

                m_pInputFile->setSampleRate(nSampleRate);

                m_pInputFile->setNumChannels(nNumChannels);
            }
            else if (eFileType == eAudioFileType_def::eFileType_wav)
            {
                // get sample rate and num chls from file

                nSampleRate = ((CWavFileIO *) m_pInputFile.get())->getSampleRate();

                nNumChannels = ((CWavFileIO* )m_pInputFile.get())->getNumChannels();
            }
            else
            {
                LibLogError("[audioFileIn] unsupported audio input file type: {}", g_globals.m_AppConfigMgr.m_sAudioFile);
                return -11;
            }

            // alloc the audio buffer (for reading audio from file)

            nDataLen = ((nBlockSize * nNumChannels * sizeof(int16_t)) + sizeof(NetAudioHeader_def));

            pAudioBuffer = (int16_t *) calloc(1, nDataLen);

            if (pAudioBuffer == nullptr)
            {
                LibLogError("[audioFileIn] unable to alloc audio data buffer");
                return -12;
            }

            // set the audio header info

            NetAudioHeader_def* pNetAudioHeader = (NetAudioHeader_def*) pAudioBuffer;

            pNetAudioHeader->m_nNumChls         = nNumChannels;
            pNetAudioHeader->m_nFrameRate       = nSampleRate;
            pNetAudioHeader->m_nSampleSize      = sizeof(int16_t);
            pNetAudioHeader->m_nAudioDataSize   = (nDataLen - sizeof(NetAudioHeader_def));

            // calculate: current time, audio block interval, and next audio start time

            ulAudioBlockInterval = (long) (((float) nBlockSize) / ((float) (nSampleRate)) * 1000.0f);

            lastTimeStamp = std::chrono::high_resolution_clock::now();

            nextSampleTime = (lastTimeStamp + std::chrono::high_resolution_clock::duration(10));

            std::cout << "Mode: 'NetAudio' - Press 'E' to exit. \n";
        }
        else
        {
            LogError("Invalid testType: {} - App terminating.", sTmp);
            return -15;
        }

        netOutput.setName("NetTxTest");

#if (SET_TCP_MULTI_SESSION == true)
        netOutput.setMultiIoSession(true);
#endif

        netOutput.initialize(nHeaderLen + nDataLen);

        status = netOutput.start();

        if (status == false)
        {
            LogError
            (
                "network output server not started - port:{} - App terminating.",
                nPort
            );

            break;
        }
        
        if (bDisplayOpenStatus == true)
        {
            LogInfo
            (
                "network output server started - port:{}",
                nPort
            );

            bDisplayOpenStatus = false;
            bDisplayWaitingStatus = true;
        }

        while (true)
        { 
            // Loop once per secing to check for exit key and app commands

            try
            { 
                status = _kbhit();
                if (status) 
                {
                    cKeyPress = (char) _getch();

                    if (cKeyPress == 'e' || cKeyPress == 'E')
                    {
                        // exit key ('e') has been pressed

                        LogInfo("App exit ket pressed. App will terminate.");

                        g_globals.m_bExitApp = true;
                        break;
                    }

                    if ((bCmdMode == true) && (cKeyPress == 'c' || cKeyPress == 'C'))
                    {
                        // Cmd entry key ('c') has been pressed...

                        std::string sTmp = "";

                        int nCmdNum, nCmdValue;

                        std::cout << "Enter a Cmd number: ";
                        std::getline(std::cin, sTmp);

                        if (sTmp != "")
                        { 
                            nCmdNum = stringToInt(sTmp);

                            sTmp = "";

                            std::cout << "Enter a Cmd value: ";
                            std::getline(std::cin, sTmp);

                            if (sTmp != "")
                            { 
                                nCmdValue = stringToInt(sTmp);

                                cmdInfo.cmdData.m_nCmdID = (uint16_t) nCmdNum;
                                cmdInfo.cmdData.m_CmdValue = (int32_t) nCmdValue;
                                cmdInfo.cmdHeader.m_nNumCmds = (nCmdNum + 1);

                                bCmdEntered = true;
                            }
                        }
                    }
                }
            }
            catch (...)
            {
                LogError("unknown eception while processing keyboard input");
            }

            // Try to read audio data from net

            if (netOutput.isRunning() == true)
            {
#if (SET_TCP_MULTI_SESSION == true)
                auto numSessions = netOutput.numActiveSessions();
                if (numSessions > 0)
                {
                    if (numSessions != nLastNumActiveSessions)
                    {
                        LogInfo("Num activateSessions: {}", numSessions);

                        nLastNumActiveSessions = numSessions;
                    }
                }
#endif
                if (bCmdMode == true)
                { 
                    // 'NetCmd' mode ...

                    if (bCmdEntered == true)
                    {
                        // if a new Cmd has been entered, send it

                        try
                        { 
                            // send the Cmd info

                            LogInfo("Sending 'NetCmd' - cmdNum: {}, cmdValue: {}", cmdInfo.cmdData.m_nCmdID, cmdInfo.cmdData.m_CmdValue);

                            netOutput.writeOutputData(&cmdInfo, sizeof(cmdInfo));

                            netOutput.sendOutput();

                            bCmdEntered = false;
                        }
                        catch (...)
                        {
                            LogError("uknown exception while sending Cmd data");
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(25));
                    }
                    else
                    {
                        // if NO new Cmd... delay for 200ms

                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    }
                }
                else
                {
                    // 'NetAudio' mode ...

                    // read the audio block (N channels * nBlockSize) into the buffer,
                    // but start reading at bufferStart + audioHeaderSize.

                    if (m_pInputFile->readBlock((pAudioBuffer + sizeof(NetAudioHeader_def)), nBlockSize) == false)
                    {
                        LibLogWarning("[audioFileIn] problem reading file: {} - (EOF ?)", g_globals.m_AppConfigMgr.m_sAudioFile);
                        g_globals.m_bExitApp = true;
                        break;
                    }

                    // wait until start time for next audio block

                    std::this_thread::sleep_until(nextSampleTime);

                    // calculate the start time of the next audio block

                    lastTimeStamp = std::chrono::high_resolution_clock::now();

                    nextSampleTime = (lastTimeStamp + std::chrono::milliseconds(ulAudioBlockInterval));

                    // send the audio block

                    netOutput.writeOutputData(pAudioBuffer, nDataLen);

                    netOutput.sendOutput();
                }
            }
            else
            {
                LogWarning("Net server not running");

                std::this_thread::sleep_for(std::chrono::milliseconds(800));
            }
        }

        if (netOutput.isRunning() == true)
        {
            netOutput.stop();
        }
    }
    while (g_globals.m_bExitApp == false);

    LogInfo("App exiting.");

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
