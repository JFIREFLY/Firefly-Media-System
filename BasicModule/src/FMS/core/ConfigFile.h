#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#include <string>
#include <map>

class CConfigFile
{
public:
	CConfigFile();
	virtual ~CConfigFile();

	bool Load(const char * filename);

	std::string Get(const char * name);

private:
	std::string& StringTrim(std::string &str);

private:
	std::map<std::string, std::string> m_mapConfig;
};

#endif // _CONFIG_FILE_H_
