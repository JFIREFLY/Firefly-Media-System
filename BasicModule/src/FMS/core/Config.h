#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>

class OConfig
{
public:
	OConfig();
	virtual ~OConfig();

	bool Load(const char * filename);

	std::string GetLogPath();
	std::string GetLogLevel();
	unsigned int GetLogKeepDays();
	unsigned long GetLogCapacity();

	unsigned short GetHttpPort();
	unsigned short GetHttpListeners();
	unsigned short GetHttpProcessors();


private:

	std::string m_logPath;
	std::string m_logLevel;
	unsigned int m_logKeepDays;
	unsigned long m_logCapacity;

	unsigned short m_httpPort;
	unsigned short m_httpListeners;
	unsigned short m_httpProcessors;
};

#endif // _CONFIG_H_
