#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_
#include <string>
#include <map>
#include <mutex>
#include "OThread.h"


class OTcpServer
{
public:
	typedef void(*MSG_CALLBACK)(void* handle, unsigned char* msg, int len, void* context);

public:
	OTcpServer(unsigned short port);
	~OTcpServer();

	static void GlobalInit();
	static void GlobalDelInit();

	static void* DispatchThread(void* arg);
	bool GetRunStatus();
	void SetRunStatus(bool flag);

	bool Start();
	void Stop();

	int Dispatch();
	bool SendMsg(void* handle, const void* msg, int len);

	struct event_base* GetBase();
	void AddClient(void* handle, const std::string& url);
	void DeleteClient(void* handle);

	void OnMsgCallback(void* handle, unsigned char* msg, int len);
	void SetMsgCallback(MSG_CALLBACK callback, void* context);

	void AutoSleep(int times = 100, int circle_ms = 10);

private:
	bool StartDispatchThread();
	void StopDispatchThread();

private:
	struct event_base* m_base;
	struct evconnlistener* m_listener;
	bool m_isConnecting;
	int m_port;
	std::map<void*, std::string> m_clientMap;

	std::mutex m_mutex;
	MSG_CALLBACK m_callback;
	void* m_context;
	bool m_isRunning;
	CrossPlatform::OThread m_dispatchThread;
};


#endif // !_TCP_SERVER_H_
