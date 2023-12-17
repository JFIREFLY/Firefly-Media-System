#ifndef _SERVER_H_
#define _SERVER_H_

#include <map>
#include <mutex>
#include <string>

#include "OResource.h"
#include "IService.h"
#include "../common/OEvent.h"


class OServer
{
public:
	static OServer * GetInstance();

public:
	OServer();
	virtual ~OServer();

	bool Init(std::string config_file);
	void Run();
	void Deinit();

	bool GetRunStatus();
	void SetRunStatus(bool flag);

	OResource * Ref();
	IService * GetService(std::string name);

	void Loop();
	void BreakLoop();

private:
	bool AddService(std::string name, IService * service);
	bool RemoveService(std::string name);
	bool RemoveAllService();

	bool StartServiceThread();
	bool StopServiceThread();

	bool InitLogger(); 


	//轮询事件
	void SetLoopEvent();
	void WaitLoopEvent(unsigned int ms = 0);
	void ResetLoopEvent();

private:
	static OServer * m_instance;

	OResource m_resource;

	bool m_isRunning;
	std::mutex m_lockRunStatus;

	std::map<std::string, IService *> m_mapService;
	std::mutex m_lockService;

	//轮询事件
	OEvent m_loopEvent;

};

#endif // _PARKLOT_SERVER_H_
