/// 
/// \file       main.cpp
/// 
///             The application "main" definition file
///             for the NetRxTest app.
///


#define _CRT_SECURE_NO_WARNINGS


#include <cstdio>
#include <filesystem>
#include <condition_variable>
#include <csignal>

#ifdef WINDOWS
#include <conio.h>
#else
#include "RDB-libs/Src/KbInput/KbInput.c"
#endif

#include "RDB-libs/Src/NetIO/CClientIO.h"
#include "RDB-libs/Src/FileIO/CAudioFileIO.h"

#include "AppGlobalsCls.h"

#include "AppUtils.h"


#include <ME-Common/Src/NetIoDataDefs.h>


// Set this option to 'true' to enable 
// multi-I/O TCP sessions

#define SET_TCP_MULTI_SESSION           true


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

    status = g_globals.appInit("NetRxTest", "1.0");
 
    if (status == false)
    {
        LogError("App / logger initialization failed");
        return false;
    }

    appInfo();

    LogInfo("Parsing application command line");

    if (g_globals.parseCommandLine(argc, argv) == false)
    {
        LogError("Error parsing command line arguments");
    }

    g_globals.setLoggerConfig();

    LogInfo("Log level set to: {}", g_globals.m_AppConfigMgr.m_nLogLevel);

    std::cout << "Press 'E' to exit. \n";

    g_globals.m_bExitApp = false;;

    std::unique_lock<std::mutex> exitLock(g_globals.m_appExitMutex);

    do
    {
        g_globals.m_logger.Flush();

        LogInfo("Initializing Network IO interface...");

        CNetworkIO::CTcpClient          netInput;

        status = netInput.allocBuffer();

        unsigned int nPort = (unsigned int) stringToUint(g_globals.m_AppConfigMgr.m_sNetPort);

        netInput.setUri(g_globals.m_AppConfigMgr.m_sNetPath);

        netInput.setPort(nPort);

        // audio file variables

        unsigned int nBlockSize = 1024;
        unsigned int nSampleRate = 48000;
        unsigned int nNumChannels = 0;

        std::shared_ptr<CAudioFileIO> m_pInputFile;

        eAudioFileType_def eFileType = eAudioFileType_def::eFileType_unknown;

        bool bAudioFileOpen = false;

        // Cmd processing variables

        char cKeyPress;

        // main data processing loop

        bool bDisplayOpenStatus = true;
        bool bDisplayWaitingStatus = true;

        LogInfo("Processing input & commandds...");

        auto nHeaderLen = sizeof(NetCmdHeader_def);

        while (true)
        { 
            // Loop once per secing to check for exit key and app commands

            try
            { 
                auto status = _kbhit();
                if (status) 
                {
                    cKeyPress = (char) _getch();

                    if (cKeyPress == 'e' || cKeyPress == 'E')
                    {
                        LogInfo("App exit ket pressed. App will terminate.");

                        g_globals.m_bExitApp = true;

                        //if (netInput.isConnected() == true)
                        //{
                        //    netInput.close();
                        //}
                    
                        break;
                    }
                }
            }
            catch (...)
            {
                LogError("unknown eception while processing keyboard input");
            }

#if (SET_TCP_MULTI_SESSION == false)
            // Try to read audio data from net

            status = netInput.open();

            if (status == true)
            {
                if (bDisplayOpenStatus == true)
                {
                    LogInfo
                    (
                        "network input connection established - uri:{}, port:{}",
                        g_globals.m_AppConfigMgr.m_sNetPath,
                        nPort
                    );

                    bDisplayOpenStatus = false;
                    bDisplayWaitingStatus = true;
                }
            }
            else
            {
                if (bDisplayWaitingStatus == true)
                {
                    LogWarning
                    (
                        "waiting for network input connection - uri:{}, port:{}",
                        g_globals.m_AppConfigMgr.m_sNetPath,
                        nPort
                    );

                    bDisplayWaitingStatus = false;
                    bDisplayOpenStatus = true;
                }
            }
#endif

            if (netInput.isConnected() == true)
            {
                auto readStatus = netInput.read();

                if (readStatus < 0)
                {
                    std::string sErr = netInput.getLastError();

                    if (sErr != "")
                    { 
                        LogError("error reading TCP data, code: {}, err: {} - closing connection", readStatus, sErr);
                    }
                    else
                    {
                        if (readStatus == -30)
                        {
                            LogInfo("received server 'exit' msg - closing connection");
                        }
                        else
                        {
                            LogError("error reading TCP data, code: {} - closing connection", readStatus);
                        }
                    }

                    netInput.close();

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                    // try to re-open the net interface

                    status = netInput.open();

                    if (status == true)
                    {
                        LogInfo
                        (
                            "network input connection established - uri:{}, port:{}",
                            g_globals.m_AppConfigMgr.m_sNetPath,
                            nPort
                        );

                        bDisplayOpenStatus = false;
                        bDisplayWaitingStatus = true;
                    }
                    else
                    {
                        LogWarning
                        (
                            "waiting for network input connection - uri:{}, port:{}",
                            g_globals.m_AppConfigMgr.m_sNetPath,
                            nPort
                        );

                        bDisplayWaitingStatus = false;
                        bDisplayOpenStatus = true;
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }

                int nDataLen = 0;
                
                if (readStatus > 0)
                { 
                    nDataLen = (int) (readStatus - nHeaderLen);
                }

                if (nDataLen > 0)
                {
                    auto pNetData = netInput.getDataPtr();

                    if (pNetData == nullptr)
                    {
                        netInput.releasePtr();
                        LogError("invalid net data ptr");
                        return false;
                    }

                    // Check what kind of data type we got

                    if (netInput.isDataType("NetAudio") == true)
                    {
                        // Get net Audio block info

                        NetAudioHeader_def* pNetAudioHeader = (NetAudioHeader_def*) pNetData;

                        nNumChannels =          pNetAudioHeader->m_nNumChls;
                        nSampleRate =           pNetAudioHeader->m_nFrameRate;
                        auto nAudioDataSize =   pNetAudioHeader->m_nAudioDataSize;

                        LogInfo("AudioMsg - Len={}, NumChls={}, FrameRate={}", nAudioDataSize, nNumChannels, nSampleRate);

                        if (bAudioFileOpen == false)
                        {
                            // load the input data file to get the number of channels

                            m_pInputFile =
                                CAudioFileIO::openFileTypeByExt
                                (
                                    g_globals.m_AppConfigMgr.m_sAudioFile,
                                    eFileIoMode_def::eFileIoMode_output,
                                    nNumChannels,
                                    nSampleRate,
                                    nBlockSize
                                );
                            if (m_pInputFile == nullptr)
                            {
                                LibLogError("[audioFileIn] failed to open audio input file: {}", g_globals.m_AppConfigMgr.m_sAudioFile);
                                return -10;
                            }

                            eFileType = getAudioFileType(g_globals.m_AppConfigMgr.m_sAudioFile);

                        }


                    }
                    else if (netInput.isDataType("NetCmd") == true)
                    {
                        unsigned int cmdCnt = 0;

                        // Get net Cmd block info

                        NetCmdHeader_def* pNetCmdHeader = (NetCmdHeader_def*) pNetData;

                        auto nNumCmds =         pNetCmdHeader->m_nNumCmds;
                        auto nCmdCnt =          pNetCmdHeader->m_nCmdCnt;
                        auto nCmdEntrySize =    pNetCmdHeader->m_nCmdEntrySize;
                        auto nCmdDataSize =     pNetCmdHeader->m_nCmdDataSize;

                        // Log input Cmds into

                        LogInfo("CmdMsg - Len={}, EntrySize={}, NumCmds={}, CmdCnt={}", nCmdDataSize, nCmdEntrySize, nNumCmds, nCmdCnt);

                        if (nCmdEntrySize != sizeof(NetCmdData_def))
                        {
                            LogWarning("EntrySize={} != sizeof(NetCmdData_def) ", nCmdEntrySize, sizeof(NetCmdData_def));
                        }

                        auto netHeaderSize = sizeof(NetCmdHeader_def);

                        CNetworkIO::DataBytePtr_def pNetMsgData = (pNetData + sizeof(NetCmdHeader_def));

                        NetCmdData_def *pNetCmdData;

                        for (auto cmdNum = 0; cmdNum < (int) nCmdCnt; cmdNum++)
                        {
                            pNetCmdData = (NetCmdData_def*) (pNetMsgData + (cmdNum * nCmdEntrySize));

                            auto nCmdNum = (pNetCmdData->m_nCmdID);

                            auto nCmdValue = (pNetCmdData->m_CmdValue);

                            LogInfo("Cmd - ID={}, Value={}", nCmdNum, nCmdValue);
                        }
                    }
                    else
                    {
                        std::string sType = netInput.getDataType();

                        LogError("Unknown net msg type={}, len={}", sType, readStatus);
                    }

                    netInput.releasePtr();
                }

#if (SET_TCP_MULTI_SESSION == false)
                netInput.close();
#else
            }
            else
            {
                netInput.setMultiIoSession(true);

                status = netInput.open();

                if (status == true)
                {
                    if (bDisplayOpenStatus == true)
                    {
                        LogInfo
                        (
                            "network input connection established - uri:{}, port:{}",
                            g_globals.m_AppConfigMgr.m_sNetPath,
                            nPort
                        );

                        bDisplayOpenStatus = false;
                        bDisplayWaitingStatus = true;
                    }
                }
                else
                {
                    if (bDisplayWaitingStatus == true)
                    {
                        LogWarning
                        (
                            "waiting for network input connection - uri:{}, port:{}",
                            g_globals.m_AppConfigMgr.m_sNetPath,
                            nPort
                        );

                        bDisplayWaitingStatus = false;
                        bDisplayOpenStatus = true;
                    }
                }
#endif
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if (netInput.isConnected() == true)
        {
            netInput.close();
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
