#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "Config.h"
#include "../common/OLogger.h"

using namespace __service_log__;

class OResource
{
public:
	OResource();
	virtual ~OResource();

	OConfig* GetConfig();
	OLogger* GetLogger();


private:
	OConfig m_config;
	OLogger m_logger; 
};

#endif // _RESOURCE_H_
