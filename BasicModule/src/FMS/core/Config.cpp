#include "Config.h"
#include "ConfigFile.h"
#include "OServer.h"

#include "../common/CommonTools.h"
#include "../common/CommonDef.h"

#include <fstream>
#include <string.h>
#include <sstream>
#ifdef WIN32
#include <windows.h>
#endif // WIN32

#ifdef WIN32
#define DEFAULT_MAX_LOG_CAPACITY (1024 * 1024) //1G (KB为单位)
#define DEFAULT_LOG_KEEPDAYS     7					 // 7天
#else
#define DEFAULT_MAX_LOG_CAPACITY (10 * 1024)		 //10M (KB为单位)
#define DEFAULT_LOG_KEEPDAYS     7					 // 7天
#endif

OConfig::OConfig()
{
	m_logPath = "./log";
	m_logLevel = "INFO";
	m_logKeepDays = DEFAULT_LOG_KEEPDAYS;
	m_logCapacity = DEFAULT_MAX_LOG_CAPACITY;

	m_httpPort = 9990;

	m_httpListeners = 4;
	m_httpProcessors = 64;
}

OConfig::~OConfig()
{
}

bool OConfig::Load(const char * filename)
{
	bool ret = false;
	CConfigFile conf;
	std::string value;

	if (!conf.Load(filename))
		return false;

	value = conf.Get("LOG_PATH");
	if (value != "")
		m_logPath = value;

	value = conf.Get("LOG_LEVEL");
	if (value != "")
	{
		m_logLevel = value;
	}

	value = conf.Get("LOG_KEEPDAYS");
	if (value != "")
	{
		m_logKeepDays = (unsigned int)strtoul(value.c_str(), nullptr, 10);
	}

	value = conf.Get("LOG_CAPACITY");
	if (value != "")
	{
		m_logCapacity = (unsigned int)strtoul(value.c_str(), nullptr, 10) * 1024;
	}

	value = conf.Get("HTTP_PORT");
	if (value != "")
		m_httpPort = (unsigned short)strtoul(value.c_str(), nullptr, 10);

	value = conf.Get("HTTP_LISTENERS");
	if (value != "")
		m_httpListeners = (unsigned short)strtoul(value.c_str(), nullptr, 10);

	value = conf.Get("HTTP_PROCESSORS");
	if (value != "")
		m_httpProcessors = (unsigned short)strtoul(value.c_str(), nullptr, 10);


	ret = true;

	return ret;
}


std::string OConfig::GetLogPath()
{
	return m_logPath;
}

std::string OConfig::GetLogLevel()
{
	return m_logLevel;
}

unsigned int OConfig::GetLogKeepDays()
{
	return m_logKeepDays;
}

unsigned long OConfig::GetLogCapacity()
{
	return m_logCapacity;
}

unsigned short OConfig::GetHttpPort()
{
	return m_httpPort;
}

unsigned short OConfig::GetHttpListeners()
{
	return m_httpListeners;
}

unsigned short OConfig::GetHttpProcessors()
{
	return m_httpProcessors;
}
