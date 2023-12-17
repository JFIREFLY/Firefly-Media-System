#include "ConfigFile.h"

#include <fstream>
#include <iostream>
#include <string.h>

CConfigFile::CConfigFile()
{
}


CConfigFile::~CConfigFile()
{
}

bool CConfigFile::Load(const char * filename)
{
	std::ifstream ifile(filename);
	if (ifile.is_open())
	{
		if (!ifile)
			return false;

		std::string text;

		while (std::getline(ifile, text))
		{
			if (text.substr(0, 1) == "#")
				continue;

			std::size_t found = text.find('=');

			if (found == std::string::npos)
				continue;

			std::string key = text.substr(0, found);
			std::string value = text.substr(found + 1, text.length() - found - 1);

			key = StringTrim(key);
			value = StringTrim(value);

			std::pair<std::map<std::string, std::string>::iterator, bool> ret = m_mapConfig.insert(std::pair<std::string, std::string>(key, value));

			if (ret.second == false)
			{
				std::cout << "warn: config item missing [" << key.c_str() << "=" << value.c_str() << "]" << std::endl;
			}
		}

		ifile.close();
		ifile.clear();
	}

	return true;
}

std::string CConfigFile::Get(const char * name)
{
	if (name == nullptr || strlen(name) == 0)
		return "";

	std::string _name = name;

	std::map<std::string, std::string>::iterator iter = m_mapConfig.find(_name);

	if (iter == m_mapConfig.end())
		return "";

	return iter->second;
}

std::string& CConfigFile::StringTrim(std::string &str)
{
	if (str.empty()) {
		return str;
	}

	str.erase(0, str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") + 1);
	str.erase(str.find_last_not_of("\r") + 1);
	str.erase(str.find_last_not_of("\n") + 1);

	return str;
}
