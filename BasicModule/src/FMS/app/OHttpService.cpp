#include "OHttpService.h"
#include "../http/OHttpServer.h"
#include "../core/OServer.h"
#include "../common/Public.h"
#include "../common/CommonTools.h"

OHttpService::OHttpService()
{
	m_isRunning = false;

	m_httpServer = new OHttpServer();

	m_httpPort = OServer::GetInstance()->Ref()->GetConfig()->GetHttpPort();

	m_httpListeners = OServer::GetInstance()->Ref()->GetConfig()->GetHttpListeners();
	m_httpProcessors = OServer::GetInstance()->Ref()->GetConfig()->GetHttpProcessors();
}

OHttpService::~OHttpService()
{
	if (NULL != m_httpServer)
	{
		delete m_httpServer;
		m_httpServer = NULL;
	}
}

int OHttpService::Start()
{
	#ifndef SINGLE_CPU_CORE_MODE
	if (((OHttpServer*)m_httpServer)->Start(m_httpPort, 180, "text/json; charset=utf-8", m_httpListeners, m_httpProcessors))
	{
		return 1;
	}
	#else
	if (((OHttpServer*)m_httpServer)->Start(m_httpPort, 180, "text/json; charset=utf-8", m_httpListeners, m_httpProcessors))
	{
		return 1;
	}
	#endif //WIN32

	Register();

	OServer::GetInstance()->Ref()->GetLogger()->Info("http listen port[%d] success", __FUNCTION__, __LINE__, m_httpPort);
	

	return 0;
}

int OHttpService::Stop()
{
	SetRunStatus(false);

	UnRegister();

	if (m_httpServer)
	{
		((OHttpServer*)m_httpServer)->Stop();
	}

	return 0;
}

void OHttpService::Register()
{
	if (NULL == m_httpServer)
	{
		return;
	}

	//×¢²á½Ó¿Ú
}

void OHttpService::UnRegister()
{
	if (NULL == m_httpServer)
	{
		return;
	}

	//
}

void OHttpService::SetRunStatus(bool flag)
{
	std::unique_lock<std::mutex> locker(m_mutex);
	m_isRunning = flag;
}

bool OHttpService::GetRunStatus()
{
	return m_isRunning;
}

void* OHttpService::GetHttpServer()
{
	return m_httpServer;
}

void OHttpService::AutoSleep(int times, int circle_ms)
{
	if (times < 0 || circle_ms < 0)
	{
		return;
	}

	int ticks = 0;

	do
	{
		if (times <= ticks)
		{
			break;
		}

		sleep_ms(circle_ms);

		++ticks;

	} while (GetRunStatus());
}
