#include <stdio.h>
#include <time.h>
#include <iostream>
#include <sstream>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/thread.h>

#ifdef WIN32
#else
#include <arpa/inet.h>
#endif // WIN32


#include "OTcpServer.h"
#include "Public.h"
#include "CommonTools.h"

#define BUFFER_SIZE 1024


void server_conn_readcb(struct bufferevent* bev, void* context)
{
	struct evbuffer* input = bufferevent_get_input(bev);
	size_t sz = evbuffer_get_length(input);

	//打印字符串
#if 0
	if (sz > 0)
	{
		char msg[BUFFER_SIZE] = { '\0' };
		bufferevent_read(bev, msg, sz);
		printf("%s\n", msg);
	}
#endif

	unsigned char msg[BUFFER_SIZE] = { 0 };
	bufferevent_read(bev, msg, sz);

	//std::string str_msg = CommonTools::Hex2String(msg, sz);

	OTcpServer* pServer = (OTcpServer*)context;

	if (NULL == pServer) 
	{ 
		return; 
	}

	pServer->OnMsgCallback(bev, msg, (int)sz);
}

void server_conn_writecb(struct bufferevent* bev, void* context)
{
	//sleep_ms(1000);

	//测试字符串
#if 0
	static int msg_num = 1;
	char reply_msg[1000] = { '\0' };
	char str[1024] = "I receive a message from server ";
	memcpy(reply_msg, str, strlen(str));
	sprintf(reply_msg + strlen(str), "%d", msg_num);
	bufferevent_write(bev, reply_msg, strlen(reply_msg));
	msg_num++;
#endif

#if 0
	//测试16进制
	char test[] = { 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01, 0x04, 0x00, 0x02, 0x00, 0x2a };

	int ret = bufferevent_write((bufferevent*)bev, test, sizeof(char) * 12);
#endif
}

void server_conn_eventcb(struct bufferevent* bev, short events, void* context)
{
	if (events & BEV_EVENT_EOF)
	{
		printf("Connection closed\n");
	}
	else if (events & BEV_EVENT_ERROR)
	{
		printf("Got an error on the connection: %s\n", strerror(errno));
	}

	OTcpServer* pServer = (OTcpServer*)context;
	if (NULL == pServer) { return; }

	//删除下行操作句柄
	pServer->DeleteClient(bev);

	//释放资源
	bufferevent_free(bev);
}

void listener_cb(struct evconnlistener* listener, evutil_socket_t fd,
	struct sockaddr* sa, int socklen, void* context)
{
	//打印客户端信息
	struct sockaddr_in* client = (sockaddr_in*)sa;
	std::ostringstream oss;
	oss.str("");
	oss << inet_ntoa(client->sin_addr) << ":" << ntohs(client->sin_port);
	std::cout << "connect new client => " << oss.str().c_str() << std::endl;

	OTcpServer* pServer = (OTcpServer*)context;
	if (NULL == pServer) { return; }

	struct event_base* base = (struct event_base*)pServer->GetBase();
	struct bufferevent* bev = NULL;

	//创建socket连接
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev)
	{
		printf("Could not create a bufferevent\n");
		event_base_loopbreak(base);
		return;
	}

	//设置回调
	bufferevent_setcb(bev, server_conn_readcb, server_conn_writecb, server_conn_eventcb, pServer);
	bufferevent_enable(bev, EV_READ | EV_WRITE);

	//添加下行操作句柄
	pServer->AddClient(bev, oss.str());
}

OTcpServer::OTcpServer(unsigned short port)
{
	m_base = NULL;
	m_listener = NULL;
	m_isConnecting = false;

	m_callback = NULL;
	m_context = NULL;

	m_port = port;

	m_isRunning = false;
}

OTcpServer::~OTcpServer()
{
	Stop();
}

void OTcpServer::GlobalInit()
{
#ifdef WIN32
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif
}

void OTcpServer::GlobalDelInit()
{
#ifdef WIN32
	WSACleanup();
#endif // WIN32
}

void* OTcpServer::DispatchThread(void* arg)
{
	OTcpServer* obj = (OTcpServer*)arg;

	if (NULL == obj)
	{
		return nullptr;
	}

	while (obj->GetRunStatus())
	{
		obj->Dispatch();

		obj->AutoSleep(5, 200);
	}

	return nullptr;
}

bool OTcpServer::GetRunStatus()
{
	return m_isRunning;
}

void OTcpServer::SetRunStatus(bool flag)
{
	m_isRunning = flag;
}

bool OTcpServer::Start()
{
	//设置多线程模式
#ifdef WIN32
	evthread_use_windows_threads();
#else
	evthread_use_pthreads();
#endif

	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(m_port);

	m_base = event_base_new();
	if (!m_base)
	{
		printf("Could not initialize libevent\n");
		return false;
	}

	//绑定监听
	m_listener = evconnlistener_new_bind(m_base, listener_cb, (void*)this,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
		(struct sockaddr*)&sin,
		sizeof(sin));

	if (!m_listener)
	{
		printf("Could not create a listener\n");
		return false;
	}

	//循环等待事件并且通知事件的发生
	bool result = StartDispatchThread();

	if (false == result)
	{
		printf("StartDispatchThread failed\n");
	}

	return result;
}

void OTcpServer::Stop()
{
	StopDispatchThread();

	sleep_ms(200);

	if (m_listener)
	{
		evconnlistener_free(m_listener);
		m_listener = NULL;
	}

	if (m_base)
	{
		event_base_free((event_base*)m_base);
		m_base = NULL;
	}
}

int OTcpServer::Dispatch()
{
	return event_base_dispatch((event_base*)m_base);
}

bool OTcpServer::SendMsg(void* handle, const void* msg, int len)
{
	if (0 == len || NULL == handle)
	{
		return false;
	}

	std::unique_lock<std::mutex> locker(m_mutex);

	std::map<void*, std::string>::iterator it = m_clientMap.find(handle);

	if (it == m_clientMap.end())
	{
		return false;
	}

	int ret = bufferevent_write((bufferevent*)it->first, msg, len);

	if (0 != ret)
	{
		return false;
	}

	return true;
}

void OTcpServer::OnMsgCallback(void* handle, unsigned char* msg, int len)
{
	if (NULL == m_callback)
	{
		return;
	}

	m_callback(handle, msg, len, m_context);
}

void OTcpServer::SetMsgCallback(MSG_CALLBACK callback, void* context)
{
	std::unique_lock<std::mutex> locker(m_mutex);

	m_context = context;
	m_callback = callback;
}

void OTcpServer::AutoSleep(int times, int circle_ms)
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

bool OTcpServer::StartDispatchThread()
{
	SetRunStatus(true);

	int nResult = m_dispatchThread.Start(OTcpServer::DispatchThread, this);

	if (0 != nResult)
	{

		return false;
	}

	return true;
}

void OTcpServer::StopDispatchThread()
{
	SetRunStatus(false);

	m_dispatchThread.Kill(0);
}

struct event_base* OTcpServer::GetBase()
{
	return m_base;
}

void OTcpServer::AddClient(void* handle, const std::string& url)
{
	std::unique_lock<std::mutex> locker(m_mutex);

	m_clientMap.insert(std::pair<void*, std::string>(handle, url));
}

void OTcpServer::DeleteClient(void* handle)
{
	std::unique_lock<std::mutex> locker(m_mutex);

	std::map<void*, std::string>::iterator it = m_clientMap.find(handle);

	if (it == m_clientMap.end())
	{
		return;
	}

	printf("DeleteClient ==> %s\n", it->second.c_str());
	m_clientMap.erase(it);
}
