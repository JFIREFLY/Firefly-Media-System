#include "OResource.h"



OResource::OResource()
{
}


OResource::~OResource()
{
}

OConfig * OResource::GetConfig()
{
	return &m_config;
}

OLogger * OResource::GetLogger()
{
	return &m_logger;
}