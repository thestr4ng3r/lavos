
#include "lavos/log.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>

using namespace lavos;


void lavos::LogF(LogLevel level, const char *file, int line, const char *function, const char *fmt, ...)
{
	const char *level_str = nullptr;
	switch(level)
	{
		case LogLevel::Debug:
			level_str = "D";
			break;
		case LogLevel::Info:
			level_str = "I";
			break;
		case LogLevel::Warning:
			level_str = "W";
			break;
		case LogLevel::Error:
			level_str = "E";
			break;
	}

	const char *filename = strrchr(file, '/');
	if(!filename)
		filename = file;
	else
		filename++;

	printf("[%s %16.16s:%-4d - %-20.20s] ", level_str, filename, line, function);
	va_list ap;
	va_start (ap, fmt);
	vprintf(fmt, ap);
	va_end (ap);
	printf("\n");
}