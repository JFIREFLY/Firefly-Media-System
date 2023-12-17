#ifndef _HTTP_SERVICE_H_
#define _HTTP_SERVICE_H_
#include "../core/IService.h"
#include "../common/OThread.h"

#include <mutex>

class OHttpService : public IService
{
public:
	OHttpService();
	virtual ~OHttpService();

	virtual int Start();
	virtual int Stop();

	void Register();
	void UnRegister();

	void SetRunStatus(bool flag);
	bool GetRunStatus();
	void* GetHttpServer();

	void AutoSleep(int times, int circle_ms);

private:
	void* m_httpServer;

	std::string m_httpIp;
	unsigned short m_httpPort;

	unsigned short m_httpListeners;
	unsigned short m_httpProcessors;

private:
	std::mutex m_mutex;
	bool m_isRunning;
};

#endif // !_HTTP_SERVICE_H_
