#ifndef  __LOGGING_H
#define  __LOGGING_H

#include <cstdio>
#include <stdarg.h>

/* just print short file name */
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifndef LOGGING_BITMASK
	#define LOGGING_BITMASK 7
#endif

#if LOGGING_BITMASK & 1
	#define lowLog(log, ...)	platformLog(0, __FILENAME__, __LINE__, __func__, log, ##__VA_ARGS__)
#else
	#define lowLog(log, ...)	{} while(0)
#endif

#if LOGGING_BITMASK & 2
	#define midLog(log, ...)	platformLog(1, __FILENAME__, __LINE__, __func__, log, ##__VA_ARGS__)
#else
	#define midLog(log, ...)	{} while(0)
#endif

#if LOGGING_BITMASK & 4
	#define higLog(log, ...)	platformLog(2, __FILENAME__, __LINE__, __func__, log, ##__VA_ARGS__)
#else
	#define higLog(log,  ...)	{} while(0)
#endif

#define fileLog(comp,log, ...)		platformLog(comp, __FILENAME__, __LINE__, __func__, log, ##__VA_ARGS__)


#define LOG_ENTRY		lowLog("%s", "ENTRY")
#define LOG_EXIT		lowLog("%s", "EXIT")

void platformLog(int logLevel, const char* file, const int line,
		 const char* func, const char *logMsg, ...);

#endif  /* __LOGGING_H */
