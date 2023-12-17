#include "OServer.h"

#include "../common/Public.h"
#include "../common/stringconv.h"
#include "../common/OLogger.h"

#include "../app/OHttpService.h"

#ifdef WIN32
#include "../common/tinystr.h"
#endif // WIN32


#include <algorithm>


using namespace std;

OServer * OServer::m_instance = nullptr;

OServer * OServer::GetInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new OServer;
	}

	return m_instance;
}

OServer::OServer()
{
	m_isRunning = false;
}


OServer::~OServer()
{
	Deinit();
}

bool OServer::Init(std::string config_file)
{
	std::string curPath;

	#ifdef WIN32
	curPath = GetCurrentDirPath() + "\\";
	#else
	#endif // WIN32

	//���������ļ�
	if (false == Ref()->GetConfig()->Load((curPath + config_file).c_str()))
	{
		printf("%s Load config file[%s] failed!\n", __FUNCTION__, config_file.c_str());
		return false;
	}

	//��ʼ���ռ����
	if (false == InitLogger())
	{
		return false;
	}

	//��ʼ�����ݿ�
	/*��ʵ��*/

	//����HTTP�ӿڷ���
	if (!AddService("OHttpService", new OHttpService))
	{
		return false;
	}

	//����TCP�ӿڷ���
	/*��ʵ��*/

	//�����û���Ϣ�������
	/*��ʵ��*/

	//�������ݲɼ�����
	/*��ʵ��*/

	//�����ͻ��˹������
	/*��ʵ��*/


	return true;
}

void OServer::Run()
{
	SetRunStatus(true);

	//�����������
	if (!StartServiceThread())
	{
		Ref()->GetLogger()->Error("start service thread fail");
		return;
	}
}

void OServer::Deinit()
{
	//��ȫ�˳�
	SetRunStatus(false);
	SetLoopEvent();

	//��ʱ����
	sleep_ms(500);

	//�ر����л�������
	StopServiceThread();

	//��ʱ����
	sleep_ms(500);

	//�ͷ�������Դ
	RemoveAllService();
}

bool OServer::GetRunStatus()
{
	std::unique_lock<std::mutex> locker(m_lockRunStatus);
	return m_isRunning;
}

void OServer::SetRunStatus(bool flag)
{
	std::unique_lock<std::mutex> locker(m_lockRunStatus);
	m_isRunning = flag;
}

OResource * OServer::Ref()
{
	return &m_resource;
}

bool OServer::AddService(std::string name, IService * service)
{
	std::unique_lock<std::mutex> locker(m_lockService);

	if (name == "" || service == NULL)
		return false;

	std::pair<std::map<std::string, IService *>::iterator, bool> ret = m_mapService.insert(std::pair<std::string, IService *>(name, service));
	return ret.second;
}

bool OServer::RemoveService(std::string name)
{
	OServer::GetInstance()->Ref()->GetLogger()->Trace("%s:%d", __FUNCTION__, __LINE__);

	std::unique_lock<std::mutex> locker(m_lockService);

	std::map<std::string, IService *>::iterator iter = m_mapService.find(name);

	if (iter == m_mapService.end())
		return false;

	delete iter->second;

	m_mapService.erase(iter);

	return true;
}

bool OServer::RemoveAllService()
{
	OServer::GetInstance()->Ref()->GetLogger()->Trace("%s:%d", __FUNCTION__, __LINE__);

	std::unique_lock<std::mutex> locker(m_lockService);

	for (std::map<std::string, IService*>::iterator iter = m_mapService.begin(); iter != m_mapService.end(); iter++)
	{
		Ref()->GetLogger()->Info("%s service remove ...", iter->first.c_str());

		delete iter->second;
	}

	m_mapService.clear();

	return true;
}

IService * OServer::GetService(std::string name)
{
	//std::unique_lock<std::mutex> locker(m_lockService);

	std::map<std::string, IService *>::iterator iter = m_mapService.find(name);

	if (iter == m_mapService.end())
		return NULL;

	return iter->second;
}

void OServer::Loop()
{
	do
	{
		//��ʱ����
		sleep_ms(500);

		WaitLoopEvent();
		ResetLoopEvent();

	} while (GetRunStatus());
}

void OServer::BreakLoop()
{
	SetRunStatus(false);
	SetLoopEvent();
}

bool OServer::StartServiceThread()
{
	std::unique_lock<std::mutex> locker(m_lockService);

	for (std::map<std::string, IService *>::iterator iter = m_mapService.begin(); iter != m_mapService.end(); iter++)
	{
		int ret = iter->second->Start();

		if (ret != 0)
		{
			OServer::GetInstance()->Ref()->GetLogger()->Error("start app service fail [%s] ret[%d]", iter->first.c_str(), ret);
			return false;
		}
		else
		{
			OServer::GetInstance()->Ref()->GetLogger()->Error("start app service success [%s]", iter->first.c_str());

		}
	}

	return true;
}

bool OServer::StopServiceThread()
{
	OServer::GetInstance()->Ref()->GetLogger()->Trace("%s:%d", __FUNCTION__, __LINE__);

	std::unique_lock<std::mutex> locker(m_lockService);

	for (std::map<std::string, IService*>::iterator iter = m_mapService.begin(); iter != m_mapService.end(); iter++)
	{
		Ref()->GetLogger()->Info("%s service stop ...", iter->first.c_str());

		int ret = iter->second->Stop();

		if (ret != 0)
		{
			Ref()->GetLogger()->Error("%s service stop error [%d]", iter->first.c_str(), ret);
			return false;
		}
		else
		{
			Ref()->GetLogger()->Info("%s service stop success.", iter->first.c_str());
		}
	}

	return true;
}

bool OServer::InitLogger()
{
	printf("create fms service logger ......\n");

	std::string curPath;

#ifdef WIN32
	curPath = GetCurrentDirPath();
	curPath.append("\\");
#endif // WIN32
	curPath.append("log");

	// ������־ģ��
	OLogger::LogLevel level;
	std::string level_str = Ref()->GetConfig()->GetLogLevel();

	if (level_str == "FATAL")
		level = OLogger::LogLevel::LL_FATAL;
	else if (level_str == "ERROR")
		level = OLogger::LogLevel::LL_ERROR;
	else if (level_str == "WARN")
		level = OLogger::LogLevel::LL_WARN;
	else if (level_str == "INFO")
		level = OLogger::LogLevel::LL_INFO;
	else if (level_str == "DEBUG")
		level = OLogger::LogLevel::LL_DEBUG;
	else if (level_str == "TRACE")
		level = OLogger::LogLevel::LL_TRACE;
	else if (level_str == "ALL")
		level = OLogger::LogLevel::LL_ALL;
	else
		level = OLogger::LogLevel::LL_OFF;

	if (Ref()->GetLogger()->Init("service", curPath, level) == false)
	{
		printf("create service logger fail\n");
		return false;
	}

	printf("create fms service logger ok\n");

	return true;
}

void OServer::SetLoopEvent()
{
	m_loopEvent.Set();
}

void OServer::WaitLoopEvent(unsigned int ms)
{
	if (0 == ms)
	{
		m_loopEvent.Wait();
	}
	else
	{
		m_loopEvent.Wait(ms);
	}
}

void OServer::ResetLoopEvent()
{
	m_loopEvent.Reset();
}
