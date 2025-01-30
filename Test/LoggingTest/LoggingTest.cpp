//******************************************************************
// LoggingTest.cpp : Defines the entry point for the application.
//

#include "LoggingTest.h"


int main()
{
	LogInfo("Starting 'LoggingTest' application");

	CLogger		logger;

	LogInfo("Initializing logging config");

	logger.Init("LoggingTest", LogLevel_info, "LoggingTest_log.txt");

	LogDebug("Log init succeeded");

	LogInfo("Application log has been initialized.  Updating log level to: debug ");

	logger.setLogLevel(LogLevel_debug);

	LogDebug("Log level has been set to: debug");

	LogWarning("This is a test warning message");

	LogError("This is a test error message");

	LogInfo("Exiting application");

	return 0;
}
