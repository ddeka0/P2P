/* code for logging */
#include <bits/stdc++.h>
#include <cstdio>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>
#include "logging.h"

#ifdef  COLOURED_LOGS		//Enable from Makefile
#define STYLE_RED	"\x1B[31m"
#define STYLE_BOLD	"\033[1m"
#define STYLE_RESET	"\033[0m"
#else
#define STYLE_RED	""
#define STYLE_BOLD	""
#define STYLE_RESET	""
#endif  /*COLOURED_LOGS*/

#define gettid() syscall(SYS_gettid)

/* main logging function, DONT USE DIRECTLY, use wrappers */
void platformLog(int logLevel, const char* file, int line,
		 const char* func, const char *logMsg, ...)
{
	va_list arg;
	time_t timer;
	struct tm* timeInfo;
	char timeBuff[20];
	char logLevelChars[] = "LMH"; /*Low, Mid & High*/

	if(logLevel > 4) 
		return;
	
	/* 

	if(logLevel == 4) {
		std::ofstream logFile;
		logFile.open("ranLog.txt");
		logFile << getpid() <<" "<<timeBuff <<" "<<file<<" "<<line<<" "<<func;
		
		va_start (arg, logMsg);
		vfprintf (logFile, logMsg, arg);
		va_end (arg);
	
	}else if(logLevel == 5) {

	} 
	
	*/

	time(&timer);
	timeInfo = localtime(&timer);
	strftime(timeBuff, 20, "%H:%M:%S", timeInfo);

	printf("%s%s%c  PID:%d  %s\t%s:%d %s:   ",
			logLevel >= 1 ? STYLE_BOLD : "",
			logLevel == 2 ? STYLE_RED  : "",
			logLevelChars[logLevel], gettid(), timeBuff,
			file, line, func);
	va_start (arg, logMsg);
	vfprintf (stdout, logMsg, arg);
	va_end (arg);
	printf("%s\n", STYLE_RESET);
}

