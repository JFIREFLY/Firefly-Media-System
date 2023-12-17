#include "OLogger.h"
#include "FmsLog.h"

#include <stdarg.h>
#include <iostream>

using namespace __service_log__;

#define LOG_BUFFER_SIZE		102400

OLogger::OLogger()
	: m_level(LL_OFF), m_ptrLog(nullptr)
{
	
}

OLogger::~OLogger()
{
	if (m_ptrLog)
	{
		delete (FmsLog*)m_ptrLog;
		m_ptrLog = nullptr;
	}
}

bool OLogger::Init(std::string modulename, std::string filename, LogLevel level)
{
	m_level = level;

	m_ptrLog = new FmsLog(modulename.c_str(), filename.c_str());

	if (m_ptrLog == nullptr)
		return false;

	switch (m_level)
	{
	case LL_OFF:
		((FmsLog*)m_ptrLog)->setLogLevel(LOGLEVEL_CLOSED);
		break;
	case LL_FATAL:
		((FmsLog*)m_ptrLog)->setLogLevel(LOGLEVEL_FATAL);
		break;
	case LL_ERROR:
		((FmsLog*)m_ptrLog)->setLogLevel(LOGLEVEL_ERROR);
		break;
	case LL_WARN:
		((FmsLog*)m_ptrLog)->setLogLevel(LOGLEVEL_WARN);
		break;
	case LL_INFO:
		((FmsLog*)m_ptrLog)->setLogLevel(LOGLEVEL_INFO);
		break;
	case LL_DEBUG:
		((FmsLog*)m_ptrLog)->setLogLevel(LOGLEVEL_DEBUG);
		break;
	case LL_TRACE:
		((FmsLog*)m_ptrLog)->setLogLevel(LOGLEVEL_TRACE);
		break;
	case LL_ALL:
		((FmsLog*)m_ptrLog)->setLogLevel(LOGLEVEL_TRACE);
		break;
	default:
		break;
	}

	return true;
}

void OLogger::Fatal(const char * fmt, ...)
{
	//std::unique_lock<std::mutex> locker(m_lockLogger);

	if (LL_FATAL > m_level)
	{
		return;
	}

	va_list ap;
	va_start(ap, fmt);

	char* buf = new char[LOG_BUFFER_SIZE];
	if (NULL == buf) { return; }

	memset(buf, 0, LOG_BUFFER_SIZE);
	vsnprintf(buf, LOG_BUFFER_SIZE, fmt, ap);

	if (m_ptrLog)
	{
		((FmsLog*)m_ptrLog)->printstr(LOGLEVEL_FATAL, (const char*)buf);
	}

	if (m_level) printf("[FATAL] %s\r\n", (const char*)buf);

	va_end(ap);

	delete[] buf;
}

void OLogger::Error(const char * fmt, ...)
{
	//std::unique_lock<std::mutex> locker(m_lockLogger);

	if (LL_ERROR > m_level)
	{
		return;
	}

	va_list ap;
	va_start(ap, fmt);

	char* buf = new char[LOG_BUFFER_SIZE];
	if (NULL == buf) { return; }

	memset(buf, 0, LOG_BUFFER_SIZE);
	vsnprintf(buf, LOG_BUFFER_SIZE, fmt, ap);

	if (m_ptrLog)
	{
		((FmsLog*)m_ptrLog)->printstr(LOGLEVEL_ERROR, (const char*)buf);
	}

	printf("[ERROR] %s\r\n", (const char*)buf);

	va_end(ap);

	delete[] buf;
}

void OLogger::Warn(const char * fmt, ...)
{
	//std::unique_lock<std::mutex> locker(m_lockLogger);

	if (LL_WARN > m_level)
	{
		return;
	}

	va_list ap;
	va_start(ap, fmt);

	char* buf = new char[LOG_BUFFER_SIZE];
	if (NULL == buf){ return; }

	memset(buf, 0, LOG_BUFFER_SIZE);
	vsnprintf(buf, LOG_BUFFER_SIZE, fmt, ap);

	if (m_ptrLog)
	{
		((FmsLog *)m_ptrLog)->printstr(LOGLEVEL_WARN, (const char*)buf);
	}

	printf("[WARN] %s\r\n", (const char*)buf);

	va_end(ap);

	delete[] buf;
}

void OLogger::Info(const char * fmt, ...)
{
	//std::unique_lock<std::mutex> locker(m_lockLogger);

	if (LL_INFO > m_level)
	{
		return;
	}

	va_list ap;
	va_start(ap, fmt);

	char* buf = new char[LOG_BUFFER_SIZE];
	if (NULL == buf) { return; }

	memset(buf, 0, LOG_BUFFER_SIZE);
	vsnprintf(buf, LOG_BUFFER_SIZE, fmt, ap);

	if (m_ptrLog)
	{
		((FmsLog*)m_ptrLog)->printstr(LOGLEVEL_INFO, (const char*)buf);
	}

	printf("[INFO] %s\r\n", (const char*)buf);

	va_end(ap);

	delete[] buf;
}

void OLogger::Debug(const char * fmt, ...)
{
	//std::unique_lock<std::mutex> locker(m_lockLogger);

	if (LL_DEBUG > m_level)
	{
		return;
	}

	va_list ap;
	va_start(ap, fmt);

	char* buf = new char[LOG_BUFFER_SIZE];
	if (NULL == buf) { return; }

	memset(buf, 0, LOG_BUFFER_SIZE);
	vsnprintf(buf, LOG_BUFFER_SIZE, fmt, ap);

	if (m_ptrLog)
	{
		((FmsLog*)m_ptrLog)->printstr(LOGLEVEL_DEBUG, (const char*)buf);
	}

	printf("[DEBUG] %s\r\n", (const char*)buf);

	va_end(ap);

	delete[] buf;
}

void OLogger::Trace(const char * fmt, ...)
{
	//std::unique_lock<std::mutex> locker(m_lockLogger);

	if (LL_TRACE > m_level)
	{
		return;
	}

	va_list ap;
	va_start(ap, fmt);

	char* buf = new char[LOG_BUFFER_SIZE];
	if (NULL == buf) { return; }

	memset(buf, 0, LOG_BUFFER_SIZE);
	vsnprintf(buf, LOG_BUFFER_SIZE, fmt, ap);

	if (m_ptrLog)
	{
		((FmsLog*)m_ptrLog)->printstr(LOGLEVEL_TRACE, (const char*)buf);
	}

	printf("[TRACE] %s\r\n", (const char*)buf);

	va_end(ap);

	delete[] buf;
}

int OLogger::GetLogLevel()
{
	return m_level;
}

void OLogger::SetLogLevel(int level)
{
	std::unique_lock<std::mutex> locker(m_lockLogger);

	m_level = (LogLevel)level;

	((FmsLog*)m_ptrLog)->setLogLevel((EnumLogLevel)level);
}

void OLogger::SetMaxCapacity(unsigned long nCapacity)
{
	std::unique_lock<std::mutex> locker(m_lockLogger);

	((FmsLog*)m_ptrLog)->setMaxCapacity(nCapacity);
}

void __service_log__::OLogger::Print(int level, const char* str)
{
	if (level > m_level)
	{
		return;
	}

	((FmsLog*)m_ptrLog)->printstrExt((EnumLogLevel)level, str);

	if (LOGLEVEL_FATAL == level)
	{
		printf("[FATAL] %s\r\n", (const char*)str);
	}
	else if (LOGLEVEL_ERROR == level)
	{
		printf("[ERROR] %s\r\n", (const char*)str);
	}
	else if (LOGLEVEL_WARN == level)
	{
		printf("[WARN] %s\r\n", (const char*)str);
	}
	else if (LOGLEVEL_INFO == level)
	{
		printf("[INFO] %s\r\n", (const char*)str);
	}
	else if (LOGLEVEL_DEBUG == level)
	{
		printf("[DEBUG] %s\r\n", (const char*)str);
	}
	else if (LOGLEVEL_TRACE == level)
	{
		printf("[TRACE] %s\r\n", (const char*)str);
	}
}

void __service_log__::OLogger::SetKeepDays(int days)
{
	std::unique_lock<std::mutex> locker(m_lockLogger);

	((FmsLog*)m_ptrLog)->setKeepDays(days);
}
