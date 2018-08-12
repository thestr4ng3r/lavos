
#ifndef LAVOS_LOG_H
#define LAVOS_LOG_H

namespace lavos
{

enum class LogLevel { Debug, Info, Warning, Error };

void LogF(LogLevel level, const char *file, int line, const char *function, const char *fmt, ...);

}

#define LAVOS_LOGF(level, fmt, ...)	{ lavos::LogF((level), __FILE__, __LINE__, __func__, fmt, __VA_ARGS__); }
#define LAVOS_LOG(level, str)		{ LAVOS_LOGF((level), "%s", str); }

#endif //LAVOS_LOG_H
