
#include "lavos/log.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>

using namespace lavos;


void lavos::LogF(LogLevel level, const char *file, int line, const char *function, const char *fmt, ...)
{
	const char *level_str = nullptr;
	const char *ansi_str = nullptr;
	switch(level)
	{
		case LogLevel::Debug:
			level_str = "D";
			ansi_str = "\x1b[36m";
			break;
		case LogLevel::Info:
			level_str = "I";
			ansi_str = "";
			break;
		case LogLevel::Warning:
			level_str = "W";
			ansi_str = "\x1b[33m";
			break;
		case LogLevel::Error:
			level_str = "E";
			ansi_str = "\x1b[31m";
			break;
	}

	const char *filename = strrchr(file, '/');
	if(!filename)
		filename = file;
	else
		filename++;

	printf("%s[%s %16.16s:%-4d - %-20.20s] ", ansi_str, level_str, filename, line, function);
	va_list ap;
	va_start (ap, fmt);
	vprintf(fmt, ap);
	va_end (ap);
	printf("\x1b[0m\n");
}