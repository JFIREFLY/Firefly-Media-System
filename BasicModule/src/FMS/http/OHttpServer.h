#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_
#include "evhttp.h"
#include "event.h"
#include "event2/http.h"
#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include "event2/bufferevent_compat.h"
#include "event2/http_struct.h"
#include "event2/http_compat.h"
#include "event2/util.h"
#include "event2/listener.h"
#include "event2/thread.h"
#include "../common/XQueue.h"
#include "../common/OEvent.h"


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <map>
#include <list>
#include <mutex>
#include <thread>

//初始化：
//1. Add(new OHttpLoginHandler());
//2. Start

//反初始化
//1.Delete
//2.Stop

typedef struct tagHttpRequest
{
	struct evhttp_request* req;
	void* arg;
	tagHttpRequest()
	{
		req = NULL;
		arg = NULL;
	}
}HTTPREQUEST, *LP_HTTPREQUEST;

class OHttpHandlerBase
{
public:
	OHttpHandlerBase();
	virtual ~OHttpHandlerBase();

	virtual std::string GetUrlPath();

	virtual bool DoGet(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond);
	virtual bool DoPost(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond);
	virtual bool DoHead(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond);
	virtual bool DoPut(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond);
	virtual bool DoDelete(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond);
	virtual bool DoOptions(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond);
	virtual bool DoTrace(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond);
	virtual bool DoConnect(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond);
	virtual bool DoPatch(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond);
};

//HTTP处理器
class OHttpProcessor
{
public:
	OHttpProcessor();
	virtual ~OHttpProcessor();

	int Start();
	int Stop();

	int Add(OHttpHandlerBase* http_handler);
	int Delete(const char* urlpath);

public:
	OHttpHandlerBase* GetUrlHandler(const std::string& url);

	bool GetRunStatus();
	void SetRunStatus(bool flag);

	bool GetIdleStatus();
	void SetIdleStatus(bool flag);

	XQueue* GetProcQueque();
	void OnProc(struct evhttp_request* req, void* arg);

	bool StartProcThread();
	void StopProcThread();
	static void* ProcThread(void* arg);

	//消息处理事件
	void SetProcEvent();
	void WaitProcEvent(unsigned int ms = 0);
	void ResetProcEvent();
	OEvent* GetProcEvent();

private:
	static bool GetUrlParm(std::string& url, std::map<std::string, std::string>& mapParam);

private:
	bool m_isRunning;
	std::mutex m_mutex;

	std::mutex m_isIdleMutex;
	bool m_isIdle;

	struct event_base* m_event_base;
	struct evhttp* m_evhttp;
	std::thread* m_dispatch_thread;

	std::mutex m_urlHandlerMapMutex;
	std::map<std::string, OHttpHandlerBase*> m_urlHandlerMap;

	//处理工作线程
	XQueue* m_procQueue;
	OEvent m_procEvent;
	std::mutex m_procMutex;
	std::thread* m_procThread;
};

//HTTP监听器
class OHttpListener
{
public:
	OHttpListener();
	virtual ~OHttpListener();

	int Start(int64_t socket, unsigned int timeout = 60, const char* content_type = "text/json; charset=utf-8", int processors = 60);
	int Stop();

	int Add(OHttpHandlerBase* http_handler);
	int Delete(const char* urlpath);

public:
	bool GetRunStatus();
	void SetRunStatus(bool flag);

	XQueue* GetRecvQueque();
	void OnRecv(HTTPREQUEST* request);

	bool Dispatch(HTTPREQUEST* request);

	bool StartRecvThread();
	void StopRecvThread();
	static void* RecvThread(void* arg);

	//消息处理事件
	void SetRecvEvent();
	void WaitRecvEvent(unsigned int ms = 0);
	void ResetRecvEvent();
	OEvent* GetRecvEvent();

	void SetHighConcurrencyStatus(bool flag);
	bool GetHighConcurrencyStatus();

	void SetReqConcurrencys(bool reset);
	uint64_t GetReqConcurrencys();

	void SetOverloadStatus(bool flag);
	bool GetOverloadStatus();

	void AutoSleep(int times = 100, int circle_ms = 10);

private:
	static void MessageCallback(struct evhttp_request* req, void* arg);
	static void RespondOverLoadExceptionTips(evhttp_request* req);

	bool StartProtectThread();
	void StopProtectThread();
	static void* ProtectThread(void* arg);

private:
	std::mutex m_mutex;
	bool m_isRunning;

	std::mutex m_isHighConcurrencyMutex;
	bool m_isHighConcurrency;

	std::mutex m_isOverloadMutex;
	bool m_isOverload;

	//接收工作线程
	XQueue* m_recvQueue;
	OEvent m_recvEvent;
	std::mutex m_recvMutex;
	std::thread* m_recvThread;
	std::list<OHttpProcessor*> m_http_processor_list;

	//过载保护线程
	uint64_t m_reqConcurrencys;
	std::mutex m_reqConcurrencysMutex;
	std::thread* m_protectThread;

	struct event_base* m_event_base;
	struct evhttp* m_evhttp;
	std::thread* m_dispatch_thread;
};

//HTTP服务器
class OHttpServer
{
public:
	OHttpServer();
	virtual ~OHttpServer();

	int Start(unsigned short port, unsigned int timeout/*连接超时(秒)*/ = 180, const char* content_type = "text/json; charset=utf-8", int listeners/*监听器个数*/ = 4, int processors/*处理器个数*/ = 60);
	int Stop();

	int Add(OHttpHandlerBase* http_handler);
	int Delete(const char* urlpath);
	void* GetHttpHandle(const char* url);

private:
	int64_t BindSocket(unsigned short port);
	void FreeSocket(int64_t socket);
	void SetNonblocking(int64_t sock);

private:
	int64_t m_socket;
	std::mutex m_mutex;
	std::list<OHttpListener*> m_httpListenerList;

	std::mutex m_urlHandlerMapMutex;
	std::map<std::string, OHttpHandlerBase*> m_urlHandlerMap;
};

#endif // _HTTP_SERVER_H_
