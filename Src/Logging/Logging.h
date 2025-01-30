/// 
/// \file   Logging.h
/// 
///         Logging header file
///


#ifndef APP_LOGGING_H
#define APP_LOGGING_H


#ifdef DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif

#ifdef WINDOWS
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT 
#endif

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"


#include <string>
#include <cstdarg>
#include <filesystem>



#define MAX_LOG_FILE_SIZE           ((size_t) (100000 * 1024))
#define MAX_LOG_FILES               ((size_t) 5)

#define ROTATE_LOGS_ON_STARTUP      true
#define USE_DAILY_LOG_ROTATION      true

#define LOG_ROTATION_HOUR           0
#define LOG_ROTATION_MINUTE         0


enum eLogLevel
{
    LogLevel_useDefault = -1,
    LogLevel_none = 0,
    LogLevel_fatal = 1,
    LogLevel_error = 2,
    LogLevel_warning = 3,
    LogLevel_info = 4,
    LogLevel_debug = 5,
    LogLevel_trace = 6
};


#define LogCritical(...)        SPDLOG_CRITICAL(__VA_ARGS__)
#define LogError(...)           SPDLOG_ERROR(__VA_ARGS__)
#define LogWarning(...)         SPDLOG_WARN(__VA_ARGS__)
#define LogInfo(...)            SPDLOG_INFO(__VA_ARGS__)
#define LogDebug(...)           SPDLOG_DEBUG(__VA_ARGS__)
#define LogTrace(...)           SPDLOG_TRACE(__VA_ARGS__)



class CLogger
{
    std::string         m_sLogName;
    std::string         m_sLogFile;
    std::string         m_sLogDir;

    eLogLevel           m_nConsoleLogLevel;
    eLogLevel           m_nFileLogLevel;

    unsigned int        m_nMaxLogFileSize;
    unsigned int        m_nMaxNumLogFiles;

    std::shared_ptr <spdlog::logger>                            m_mainLogger;
    std::shared_ptr <spdlog::sinks::stdout_color_sink_mt>       m_consoleSink;
    std::shared_ptr <spdlog::sinks::rotating_file_sink_mt>      m_fileSink;

    spdlog::level::level_enum   getSpdlogLevel(int level)
    {
        return ((spdlog::level::level_enum) ((int) spdlog::level::off - level));
    }

    void clearOldLogger()
    {
        try
        {
            spdlog::drop_all();
        }
        catch (...)
        {
        }

        try
        {
            spdlog::shutdown();
        }
        catch (...)
        {
        }
    }

    bool initializeMainLogger()
    {
        clearOldLogger();

        try
        {
            m_mainLogger =
                std::make_shared<spdlog::logger>(m_sLogName);
        }
        catch (...)
        {
            m_mainLogger = nullptr;
        }

        if (m_mainLogger == nullptr)
        {
            SPDLOG_ERROR("Failed to create mainLogger");
            return false;
        }

        try
        {
            spdlog::initialize_logger(m_mainLogger);

            //spdlog::register_logger(m_mainLogger);

            spdlog::set_default_logger(m_mainLogger);
        }
        catch (...)
        {
            SPDLOG_WARN("logger already registered");
        }

        return true;
    }

    bool initializeConsoleSink()
    {
        if (m_mainLogger == nullptr)
        {
            // main logger not yet created
            // so just exit this function

            return false;
        }

        try
        {
            m_consoleSink =
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        }
        catch (...)
        {
            m_consoleSink = nullptr;
        }

        if (m_consoleSink == nullptr)
        {
            SPDLOG_ERROR("Failed to create console log sink");
            return false;
        }

        m_mainLogger->sinks().push_back(m_consoleSink);

        return true;
    }

    bool initializeFileSink()
    {
        if (m_mainLogger == nullptr)
        {
            // main logger not yet created
            // so just exit this function

            return false;
        }

        if (m_sLogFile == "")
        {
            SPDLOG_ERROR("Tried to create logfile sink, but log filename not yet set");
        }

        std::string sLogFilePath = "";

        // if log dir is set...

        if (m_sLogDir == "")
        {
            sLogFilePath = m_sLogFile;
        }
        else
        {

            sLogFilePath = getLogFilePath(m_sLogDir);
        }

        SPDLOG_INFO("Opening log file: {}", sLogFilePath);

        if (m_fileSink != nullptr)
        {
            try
            {
                for (auto s = m_mainLogger->sinks().begin(); s != m_mainLogger->sinks().end(); s++)
                {
                    if ((*s) == nullptr)
                        continue;

                    m_mainLogger->sinks().erase(s);

                    break;
                }

                m_fileSink.reset();
            }
            catch (...)
            {
                return false;
            }
        }

        try
        {
            m_fileSink =
                std::make_shared<spdlog::sinks::rotating_file_sink_mt>
                (
                    sLogFilePath,
                    MAX_LOG_FILE_SIZE,
                    MAX_LOG_FILES,
                    ROTATE_LOGS_ON_STARTUP
                );
        }
        catch (...)
        {
            m_fileSink = nullptr;
        }

        if (m_fileSink == nullptr)
        {
            SPDLOG_ERROR("Failed to create file log sink");
            return false;
        }

        m_mainLogger->sinks().push_back(m_fileSink);

        return true;
    }

  public:

    CLogger
        (
            eLogLevel logLevel = LogLevel_warning, 
            const std::string &sLogFile = "", 
            const std::string &sLogDir = ""
        ) :
        m_nConsoleLogLevel(logLevel),
        m_nFileLogLevel(LogLevel_info),
        m_sLogFile(sLogFile),
        m_sLogDir(sLogDir)
    {
        m_nMaxLogFileSize =     0;      // use default
        m_nMaxNumLogFiles =     0;      // use default

        m_mainLogger = nullptr;
        m_consoleSink = nullptr;
        m_fileSink = nullptr;

        m_sLogName.clear();
    }

    ~CLogger()
    {
        if (m_fileSink != nullptr)
        {
            m_fileSink->flush();
        }

        clearOldLogger();
    }

    bool Init
        (
            const std::string &sLogName, 
            eLogLevel consoleLogLevel = LogLevel_useDefault, 
            const std::string& sLogFile = "", 
            const std::string& sLogDir = ""
        )
    {
        m_sLogName = sLogName;

        if (consoleLogLevel >= 0)
        {
            m_nConsoleLogLevel = (eLogLevel) consoleLogLevel;
        }

        auto pLogger = spdlog::get(m_sLogName);
        if (pLogger != nullptr)
        {
            spdlog::drop_all();
        }

        bool status = false;

        status = initializeMainLogger();

        if (status == false)
        {
            return false;
        }

        if (sLogFile != "")
        {
            m_sLogFile = sLogFile;

            if (sLogDir != "")
            {
                m_sLogDir = sLogDir;
            }

            status = initializeFileSink();
        }

        status = initializeConsoleSink();

        if (status == false)
        {
            return false;
        }

        setConsoleLogLevel(m_nConsoleLogLevel);

        return true;
    }

    void setConsoleLogLevel(eLogLevel logLevel)
    {
        if  (logLevel != LogLevel_useDefault)   // use default/current loglevel setting
        {
            m_nConsoleLogLevel = logLevel;
        }

        if (m_consoleSink != nullptr)
        {
            m_consoleSink->set_level(getSpdlogLevel(m_nConsoleLogLevel));
        }

        if (m_mainLogger == nullptr)
        {
            spdlog::set_level(getSpdlogLevel(m_nConsoleLogLevel));
        }
        else
        {
            m_mainLogger->set_level(getSpdlogLevel(m_nConsoleLogLevel));
        }
    }


    void setLogLevel(eLogLevel fileLogLevel, eLogLevel consoleLogLevel = LogLevel_useDefault)
    {
        if (consoleLogLevel != LogLevel_useDefault)   // use default/current loglevel setting
        {
            m_nConsoleLogLevel = consoleLogLevel;
        }

        if (fileLogLevel != LogLevel_useDefault)   // use default/current loglevel setting
        {
            m_nFileLogLevel = fileLogLevel;
        }

        if (m_fileSink != nullptr)
        {
            m_fileSink->set_level(getSpdlogLevel(m_nFileLogLevel));
#ifdef DEBUG
            //m_fileSink->set_pattern("[%H:%M:%S] [%f : %L] %v");
#else
            //m_fileSink->set_pattern("[%H:%M:%S] %v");
#endif
        }

        if (m_consoleSink != nullptr)
        {
            m_consoleSink->set_level(getSpdlogLevel(m_nConsoleLogLevel));
#ifdef DEBUG
            //m_consoleSink->set_pattern("[%H:%M:%S] [%f : %L] %v");
#else
            //m_consoleSink->set_pattern("[%H:%M:%S] %v");
#endif
        }

        int logLevel;

        if (m_nConsoleLogLevel > m_nFileLogLevel)
        {
            logLevel = m_nConsoleLogLevel;
        }
        else
        {
            logLevel = m_nFileLogLevel;
        }

        if (m_mainLogger == nullptr)
        {
            spdlog::set_level(getSpdlogLevel(logLevel));
        }
        else
        {
            m_mainLogger->set_level(getSpdlogLevel(logLevel));
        }
    }

    std::string getLogLevelString()
    {
        std::string sOut;

        switch (m_nFileLogLevel)
        {
            case eLogLevel::LogLevel_none:
                sOut = "None";
                break;

            case eLogLevel::LogLevel_fatal:
                sOut = "Critical";
                break;

            case eLogLevel::LogLevel_error:
                sOut = "Error";
                break;

            case eLogLevel::LogLevel_warning:
                sOut = "Warning";
                break;

            case eLogLevel::LogLevel_info:
                sOut = "Info";
                break;

            case eLogLevel::LogLevel_debug:
                sOut = "Debug";
                break;

            case eLogLevel::LogLevel_trace:
                sOut = "Trace";
                break;

            default:
                sOut = "Unknown";
                break;
        }

        return sOut;
    }

    std::string getLogFilePath(const std::string sDir)
    {
#ifdef WINDOWS

        char cFilePathSeperator = '\\';

#else

        char cFilePathSeperator = '/';

#endif

        // if log dir is set...

        std::string sLogFilePath = sDir;

        if (sLogFilePath.length() < 1)
        {
            sLogFilePath = ".";
        }

        int lastChar = (int)(sLogFilePath.length() - 1);

        if (sLogFilePath[lastChar] != cFilePathSeperator)
        {
            sLogFilePath.push_back(cFilePathSeperator);
        }

        // set full log file pth...

        sLogFilePath.append(m_sLogFile);

        std::filesystem::path filePath = m_sLogFile;

        if (filePath.extension().string() == "")
        {
            sLogFilePath.append("_log.txt");
        }

        return sLogFilePath;
    }

    bool setLogFile(const std::string &sFile = "", const std::string& sDir = "", eLogLevel fileLogLevel = LogLevel_useDefault)
    {
        // set the log file name

        if (sFile != "")
        { 
            m_sLogFile = sFile;
        }

        if (m_sLogFile == "")
        {
            if (m_sLogName != "")
            {
                m_sLogFile = (m_sLogName + "_log.txt");
            }
            else
            {
                return false;
            }
        }

        if (sDir != "")
        {
            m_sLogDir = sDir;
        }

        std::string sLogFilePath = "";

        // if log dir is set...

        if (m_sLogDir == "")
        {
            sLogFilePath = m_sLogFile;
        }
        else
        {
            
            sLogFilePath = getLogFilePath(sDir);
        } 

        SPDLOG_INFO("Opening log file: {}", sLogFilePath);

        bool status = initializeFileSink();

        if (status == false)
        {
            return false;
        }
        
        return true;
    }

    void Flush()
    {
        if (m_mainLogger == nullptr)
            return;

        m_mainLogger->flush();
    }
};


#endif // APP_LOGGING_H
