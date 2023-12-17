#include "OHttpServer.h"

#include "../common/CommonTools.h"
#include "../common/Public.h"
#include "../core/OServer.h"

#include <vector>
#include <thread>

#ifdef WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <netdb.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <fcntl.h>
#endif


//URL��󳤶�
#define URL_MAX_LENGTH		65536

static void SplitStringToVector(std::string str, std::string split, std::vector<std::string>& vec_str)
{
	//��ʼ���ָ�����λ��
	size_t pos = std::string::npos;
	//��ʼ����ʼλ��
	size_t start = 0;

	//������黺��
	vec_str.clear();

	do
	{
		//���ݷָ��������ַ���
		pos = str.find(split, start);

		if (pos != std::string::npos)
		{
			//���ҵ��ָ�����λ�ã�������ӷָ���������ַ������黺�棬���¼���start��λ��
			vec_str.push_back(str.substr(start, pos - start));
			start = pos + split.length();
		}
		else
		{
			//�Ҳ����ָ�����λ�ã�ֱ�����ʣ����ַ���
			vec_str.push_back(str.substr(start));
		}
	} while (pos != std::string::npos); //����Ƿ��ܹ����ҵ��ָ�����λ��
}

static std::string& StringLeftTrim(std::string& s) {
	//ɾ����ߵĿո��ַ�
	s.erase(0, s.find_first_not_of(" "));
	return s;
}

static std::string& StringRightTrim(std::string& s) {
	//ɾ���ұߵĿո��ַ�
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}

static std::string& StringTrim(std::string& s) {
	//ɾ���������ߵĿո��ַ�
	return StringLeftTrim(StringRightTrim(s));
}



OHttpHandlerBase::OHttpHandlerBase()
{
}

OHttpHandlerBase::~OHttpHandlerBase()
{
}

std::string OHttpHandlerBase::GetUrlPath()
{
	return "";
}

bool OHttpHandlerBase::DoGet(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond)
{
	return false;
}

bool OHttpHandlerBase::DoPost(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond)
{
	return false;
}

bool OHttpHandlerBase::DoHead(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond)
{
	
	return false;
}

bool OHttpHandlerBase::DoPut(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond)
{
	return false;
}

bool OHttpHandlerBase::DoDelete(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond)
{
	return false;
}

bool OHttpHandlerBase::DoOptions(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond)
{
	return false;
}

bool OHttpHandlerBase::DoTrace(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond)
{
	return false;
}

bool OHttpHandlerBase::DoConnect(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond)
{
	return false;
}

bool OHttpHandlerBase::DoPatch(const std::map<std::string, std::string>& mapParam, const std::map<std::string, std::string>& mapHeader, const std::string& request, std::string& respond)
{
	return false;
}

OHttpServer::OHttpServer()
{
	#ifdef WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	#endif

	//��ʼ��
	m_socket = -1;
}


OHttpServer::~OHttpServer()
{
	Stop();

	#ifdef WIN32
	WSACleanup();
	#endif // WIN32
}

int OHttpServer::Start(unsigned short port, unsigned int timeout, const char* content_type, int listeners, int processors)
{
	//�ͷž���Դ
	Stop();

	//�����˿�
	m_socket = BindSocket(port);

	if (m_socket < 0)
	{
		printf("BindSocket fail ==> m_socket[%d]\n", (int)m_socket);
		return -1;
	}

	int ret = 0;

	//����������
	std::unique_lock<std::mutex> locker(m_mutex);

	for (int i = 0; i < listeners; i++)
	{
		OHttpListener* httpListener = new OHttpListener;
		if (NULL == httpListener)
		{
			return -2;
		}

		ret = httpListener->Start(m_socket, timeout, content_type, processors);
		if (0 != ret)
		{
			return -3;
		}

		m_httpListenerList.push_back(httpListener);
	}

	return 0;
}

int OHttpServer::Stop()
{
	std::unique_lock<std::mutex> locker(m_mutex);

	//ɾ��������
	for (std::list<OHttpListener*>::iterator iter = m_httpListenerList.begin(); iter != m_httpListenerList.end(); ++iter)
	{
		OHttpListener* httpListener = *iter;
		if (NULL == httpListener)
		{
			continue;
		}

		httpListener->Stop();
		delete httpListener;
		httpListener = NULL;
	}

	m_httpListenerList.clear();

	//�ͷ��׽�����Դ
	FreeSocket(m_socket);
	m_socket = -1;

	return 0;
}

int OHttpServer::Add(OHttpHandlerBase* http_handler)
{
	if (nullptr == http_handler || http_handler->GetUrlPath().length() == 0)
		return -1;

	int ret = 0;

	//���URL��������Դ
	std::unique_lock<std::mutex> locker(m_urlHandlerMapMutex);
	m_urlHandlerMap.insert(std::pair<std::string, OHttpHandlerBase*>(http_handler->GetUrlPath(), http_handler));

	//ע��URL
	for (std::list<OHttpListener*>::iterator iter = m_httpListenerList.begin(); iter != m_httpListenerList.end(); ++iter)
	{
		OHttpListener* httpListener = *iter;
		if (NULL == httpListener)
		{
			continue;
		}

		ret = httpListener->Add(http_handler);
		if (0 != ret)
		{
			break;
		}
	}

	return ret;
}

int OHttpServer::Delete(const char* urlpath)
{
	if (nullptr == urlpath)
	{
		return -1;
	}

	int ret = 0;

	//ע��URL
	std::unique_lock<std::mutex> locker(m_urlHandlerMapMutex);

	OHttpListener* httpListener = NULL;

	for (std::list<OHttpListener*>::iterator iter = m_httpListenerList.begin(); iter != m_httpListenerList.end(); ++iter)
	{
		httpListener = *iter;
		if (NULL == httpListener)
		{
			continue;
		}

		ret = httpListener->Delete(urlpath);
		if (0 != ret)
		{
			return ret;
		}
	}

	//�ͷ�URL��������Դ
	std::map<std::string, OHttpHandlerBase*>::iterator iter = m_urlHandlerMap.find(urlpath);

	if (iter == m_urlHandlerMap.end())
	{
		return ret;
	}

	if (iter->second)
	{
		delete iter->second;
	}

	m_urlHandlerMap.erase(iter);

	return 0;
}

void* OHttpServer::GetHttpHandle(const char* url)
{
	if (nullptr == url)
	{
		return NULL;
	}

	//�ͷ�URL��������Դ
	std::map<std::string, OHttpHandlerBase*>::iterator iter = m_urlHandlerMap.find(url);

	if (iter == m_urlHandlerMap.end())
	{
		return NULL;
	}

	return iter->second;
}

int64_t OHttpServer::BindSocket(unsigned short port)
{
	if (port < 0)
	{
		return -1;
	}

	//���������
	#if 0
	const int backlog = SOMAXCONN;
	#else
	const int backlog = 0x7fffffff;
	#endif

	sockaddr_in srvaddr;
	memset(&srvaddr, 0, sizeof(srvaddr));

	//����socket
	#ifdef WIN32
	int64_t sock = socket(AF_INET, SOCK_STREAM, 0);
	#else
	int64_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	#endif

	if (sock < 0)
	{
		return -2;
	}

	//����SO_REUSEADDR�����õ�ַ����
	int optval = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));

	//�������������
	SetNonblocking(sock);

	//��IP��ַ�Ͷ˿�
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(port);
	srvaddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (sockaddr*)(&srvaddr), sizeof(sockaddr)) == -1)
	{
		return -3;
	}

	//��������
	if (listen(sock, backlog) == -1)
	{
		return -4;
	}

	return sock;
}

void OHttpServer::FreeSocket(int64_t socket)
{
	if (socket < 0)
	{
		return;
	}

	#ifdef WIN32
	closesocket(socket);
	#else
	close(socket);
	#endif // WIN32
}

void OHttpServer::SetNonblocking(int64_t sock)
{
	#ifdef WIN32
	unsigned long ul = 1;
	ioctlsocket(sock, FIONBIO, (unsigned long*)&ul);
	#else
	fcntl(sock, F_SETFL, O_NONBLOCK);
	#endif
}

OHttpProcessor::OHttpProcessor()
{
	m_isIdle = true;
	m_isRunning = false;
	m_event_base = NULL;
	m_evhttp = NULL;
	m_dispatch_thread = NULL;
	m_procQueue = new XQueue(NORMAL_TYPE);
	m_procThread = NULL;
}

OHttpProcessor::~OHttpProcessor()
{
	Stop();

	if (m_procQueue)
	{
		delete m_procQueue;
		m_procQueue = NULL;
	}
}

int OHttpProcessor::Start()
{
	if (false == StartProcThread())
	{
		return -1;
	}
	return 0;
}

int OHttpProcessor::Stop()
{
	StopProcThread();

	return 0;
}

int OHttpProcessor::Add(OHttpHandlerBase* http_handler)
{
	if (nullptr == http_handler || http_handler->GetUrlPath().length() == 0)
		return -1;

	//ע��URL
	std::unique_lock<std::mutex> locker(m_urlHandlerMapMutex);

	std::map<std::string, OHttpHandlerBase*>::iterator iter_handler = m_urlHandlerMap.find(http_handler->GetUrlPath().c_str());

	if (iter_handler != m_urlHandlerMap.end())
		return -2;

	std::pair<std::map<std::string, OHttpHandlerBase*>::iterator, bool> ret =
		m_urlHandlerMap.insert(std::pair<std::string, OHttpHandlerBase*>(http_handler->GetUrlPath().c_str(), http_handler));

	if (false == ret.second)
		return -3;

	return 0;
}

int OHttpProcessor::Delete(const char* urlpath)
{
	if (nullptr == urlpath)
	{
		return -1;
	}

	//ע��URL
	std::unique_lock<std::mutex> locker(m_urlHandlerMapMutex);

	std::map<std::string, OHttpHandlerBase*>::iterator iter_handler = m_urlHandlerMap.find(urlpath);
	if (iter_handler == m_urlHandlerMap.end())
	{
		return -2;
	}

	m_urlHandlerMap.erase(iter_handler);

	return 0;
}

OHttpHandlerBase* OHttpProcessor::GetUrlHandler(const std::string& url)
{
	//std::unique_lock<std::mutex> locker(m_urlHandlerMapMutex);

	std::map<std::string, OHttpHandlerBase*>::iterator iter = m_urlHandlerMap.find(url);

	if (iter != m_urlHandlerMap.end())
	{
		return iter->second;
	}

	return NULL;
}

bool OHttpProcessor::GetRunStatus()
{
	return m_isRunning;
}

void OHttpProcessor::SetRunStatus(bool flag)
{
	std::unique_lock<std::mutex> locker(m_mutex);
	m_isRunning = flag;
}

bool OHttpProcessor::GetIdleStatus()
{
	return m_isIdle;
}

void OHttpProcessor::SetIdleStatus(bool flag)
{
	std::unique_lock<std::mutex> locker(m_isIdleMutex);
	m_isIdle = flag;
}

XQueue* OHttpProcessor::GetProcQueque()
{
	return m_procQueue;
}

void OHttpProcessor::OnProc(evhttp_request* req, void* arg)
{
	if ((NULL == req) || (NULL == arg))
	{
		return;
	}

	OHttpListener* httpListener = (OHttpListener*)arg;

	//ͳ��������
	httpListener->SetReqConcurrencys(false);

	int retcode = HTTP_OK;
	std::string retreason = "OK";

	std::string request;
	std::string respond;

	std::map<std::string, std::string> mapParam;
	std::map<std::string, std::string> mapHeader;
	std::string str_url;

	do
	{
		//���ر���
		if (true == httpListener->GetOverloadStatus())
		{
			respond = "{\"code\": 501,\"msg\": \"overload, visits greater than 1024 rps\"}";

			retcode = HTTP_NOTIMPLEMENTED;
			retreason = "overload, visits greater than 1024 rps";
			break;
		}

		//��ȡ����
		evhttp_cmd_type cmd_type = evhttp_request_get_command(req);

		//��ȡURL
		const struct evhttp_uri* evhttp_uri = evhttp_request_get_evhttp_uri(req);

		str_url.resize(URL_MAX_LENGTH + 1, 0x00);
		evhttp_uri_join(const_cast<struct evhttp_uri*>(evhttp_uri), (char*)str_url.data(), URL_MAX_LENGTH);
		str_url = str_url.c_str();

		//����URL����
		if (EVHTTP_REQ_GET == cmd_type)
		{
			GetUrlParm(str_url, mapParam);
		}

		//��ȡ������
		OHttpHandlerBase* ptr = GetUrlHandler(str_url);
		if (NULL == ptr)
		{
			retcode = HTTP_NOTFOUND;
			retreason = "could not find content for uri";
			break;
		}

		//����HEADER����
		struct evkeyvalq* header = evhttp_request_get_input_headers(req);
		if (header)
		{
			struct evkeyval* kv = header->tqh_first;

			while (kv)
			{
				mapHeader.insert(std::pair<std::string, std::string>(kv->key, kv->value));
				kv = kv->next.tqe_next;
			}
		}

		//��ȡ�����IP
		if (NULL != req->remote_host)
		{
			mapHeader.insert(std::make_pair("remote_host", req->remote_host));
		}

		//��ȡ����˶˿�
		mapHeader.insert(std::make_pair("remote_port", std::to_string(req->remote_port)));

		//��ȡ�ύ����
		size_t post_size = 0;
		post_size = evbuffer_get_length(req->input_buffer);

		if (post_size > 0)
		{
			request.resize(post_size + 1, 0x00);
			memcpy((char*)request.data(), evbuffer_pullup(req->input_buffer, -1), post_size);
		}

		bool retval = false;

		//��������ִ��ͨ��
		switch (cmd_type)
		{
		case EVHTTP_REQ_GET:
			retval = ptr->DoGet(mapParam, mapHeader, request, respond);
			break;
		case EVHTTP_REQ_POST:
			retval = ptr->DoPost(mapParam, mapHeader, request, respond);
			break;
		case EVHTTP_REQ_HEAD:
			retval = ptr->DoHead(mapParam, mapHeader, request, respond);
			break;
		case EVHTTP_REQ_PUT:
			retval = ptr->DoPut(mapParam, mapHeader, request, respond);
			break;
		case EVHTTP_REQ_DELETE:
			retval = ptr->DoDelete(mapParam, mapHeader, request, respond);
			break;
		case EVHTTP_REQ_OPTIONS:
			retval = ptr->DoOptions(mapParam, mapHeader, request, respond);
			break;
		case EVHTTP_REQ_TRACE:
			retval = ptr->DoTrace(mapParam, mapHeader, request, respond);
			break;
		case EVHTTP_REQ_CONNECT:
			retval = ptr->DoConnect(mapParam, mapHeader, request, respond);
			break;
		case EVHTTP_REQ_PATCH:
			retval = ptr->DoPatch(mapParam, mapHeader, request, respond);
			break;
		default:
			retval = false;
			break;
		}

		if (false == retval)
		{
			retcode = HTTP_NOTFOUND;
			retreason = "could not find content for uri";
			break;
		}

		retcode = HTTP_OK;
		retreason = "OK";

	} while (false);


	//��Ӧ����
	struct evbuffer* retbuff = NULL;

	if (respond.length() > 0)
	{
		retbuff = evbuffer_new();

		if (NULL != retbuff)
		{
			#if 0
			evbuffer_add_printf(retbuff, respond.c_str());
			#endif

			evbuffer_add(retbuff, respond.data(), respond.length());
		}
	}

	//���ͻظ�
	evhttp_send_reply(req, retcode, retreason.c_str(), retbuff);

	if (NULL != retbuff)
	{
		evbuffer_free(retbuff);
		retbuff = NULL;
	}

	#if 0
	evhttp_send_reply_end(req);
	#endif
}

bool OHttpProcessor::StartProcThread()
{
	SetRunStatus(true);

	ResetProcEvent();

	m_procThread = new std::thread(ProcThread, this);

	if (m_procThread)
	{
		return true;
	}

	return false;
}

void OHttpProcessor::StopProcThread()
{
	SetRunStatus(false);

	SetProcEvent();

	if (m_procThread)
	{
		m_procThread->join();

		delete m_procThread;
		m_procThread = NULL;
	}
}

void* OHttpProcessor::ProcThread(void* arg)
{
	OHttpProcessor* obj = (OHttpProcessor*)arg;
	if (NULL == obj)
	{
		return NULL;
	}

	XQueue* procQueue = obj->GetProcQueque();
	if (NULL == procQueue)
	{
		return NULL;
	}

	while (obj->GetRunStatus())
	{
		HTTPREQUEST* request = (HTTPREQUEST*)procQueue->PopFront();
		if (NULL == request)
		{
			//����״̬Ϊ����
			obj->SetIdleStatus(true);

			obj->WaitProcEvent();
			obj->ResetProcEvent();
			continue;
		}

		//����״̬Ϊռ��
		obj->SetIdleStatus(false);

		obj->OnProc(request->req, request->arg);

		//�ͷ���Դ
		free(request);
		request = NULL;
	}

	return NULL;
}

void OHttpProcessor::SetProcEvent()
{
	m_procEvent.Set();
}

void OHttpProcessor::WaitProcEvent(unsigned int ms)
{
	if (0 == ms)
	{
		m_procEvent.Wait();
	}
	else
	{
		m_procEvent.Wait(ms);
	}
}

void OHttpProcessor::ResetProcEvent()
{
	m_procEvent.Reset();
}

OEvent* OHttpProcessor::GetProcEvent()
{
	return &m_procEvent;
}

bool OHttpProcessor::GetUrlParm(std::string& url, std::map<std::string, std::string>& mapParam)
{
	std::size_t pos = url.find("?");

	if (std::string::npos == pos)
	{
		return false;
	}

	std::string sub_url = url.substr(pos + 1);
	url = url.substr(0, pos);

	std::vector<std::string> vecItems;
	SplitStringToVector(sub_url, "&", vecItems);

	for (std::vector<std::string>::iterator iter = vecItems.begin(); iter != vecItems.end(); iter++)
	{
		std::vector<std::string> vecSubItem;
		SplitStringToVector((*iter), "=", vecSubItem);

		if (vecSubItem.size() == 2)
		{
			mapParam.insert(std::pair<std::string, std::string>(StringTrim(vecSubItem[0]), StringTrim(vecSubItem[1])));
		}
	}

	return true;
}

OHttpListener::OHttpListener()
{
	m_isRunning = false;
	m_recvQueue = new XQueue(NORMAL_TYPE);
	m_recvThread = NULL;

	m_isRunning = false;
	m_isHighConcurrency = false;
	m_isOverload = false;

	m_reqConcurrencys = 0;
	m_protectThread = NULL;

	m_event_base = NULL;
	m_evhttp = NULL;
	m_dispatch_thread = NULL;
}

OHttpListener::~OHttpListener()
{
	Stop();

	if (m_recvQueue)
	{
		delete m_recvQueue;
		m_recvQueue = NULL;
	}
}

void* DispatchThread(void* ptr_event_base, void* ptr_http_server)
{
	OHttpListener* httpListener = (OHttpListener*)ptr_http_server;
	struct event_base* event_base = (struct event_base*)ptr_event_base;

	while (httpListener->GetRunStatus())
	{
		#if 0
		event_base_dispatch(event_base);
		#endif // 0

		#if 0
		//Ч����ͬ�� event_base_dispatch(event_base);
		event_base_loop(event_base, 0);
		#endif // 0

		#ifdef WIN32
		event_base_loop(event_base, EVLOOP_NONBLOCK);
		this_thread::sleep_for(chrono::milliseconds(1));
		#else
		if (false == httpListener->GetHighConcurrencyStatus())
		{
			event_base_loop(event_base, EVLOOP_ONCE); //����ѭ���Ե���
		}
		else
		{
			//�߲���ģʽ
			event_base_loop(event_base, EVLOOP_NONBLOCK); //������ѭ���Ե���
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		#endif // WIN32
	}

	return NULL;
}

int OHttpListener::Start(int64_t socket, unsigned int timeout, const char* content_type, int processors)
{
	int ret = 0;

	//�ͷž���Դ
	Stop();

	//�����·���
	SetRunStatus(true);
	StartProtectThread();

	//����������
	for (int i = 0; i < processors; i++)
	{
		OHttpProcessor* http_processor = new OHttpProcessor;
		if (NULL == http_processor)
		{
			return -2;
		}

		ret = http_processor->Start();
		if (0 != ret)
		{
			return -3;
		}

		m_http_processor_list.push_back(http_processor);
	}

	//���ö��߳�ģʽ
	#ifdef WIN32
	evthread_use_windows_threads();
	#else
	evthread_use_pthreads();
	#endif

	m_event_base = event_base_new();
	if (NULL == m_event_base)
	{
		return -1;
	}

	ret = event_reinit(m_event_base);
	if (0 != ret)
	{
		return -2;
	}

	m_evhttp = evhttp_new(m_event_base);
	if (NULL == m_evhttp)
	{
		return -3;
	}

	ret = evhttp_accept_socket(m_evhttp, socket);
	if (0 != ret)
	{
		return -4;
	}

	//��������ʱʱ��(s)
	evhttp_set_timeout(m_evhttp, timeout);

	//��������
	if (nullptr != content_type)
	{
		evhttp_set_default_content_type(m_evhttp, content_type);
	}

	//֧�����
	evhttp_set_allowed_methods(m_evhttp, EVHTTP_REQ_GET | EVHTTP_REQ_POST | EVHTTP_REQ_HEAD | EVHTTP_REQ_PUT |
		EVHTTP_REQ_DELETE | EVHTTP_REQ_OPTIONS | EVHTTP_REQ_TRACE | EVHTTP_REQ_CONNECT | EVHTTP_REQ_PATCH);

	//����http����ص�����
	evhttp_set_gencb(m_evhttp, &OHttpListener::MessageCallback, this);

	//����Dispatch�߳�
	m_dispatch_thread = new std::thread(DispatchThread, m_event_base, this);
	if (NULL == m_dispatch_thread)
	{
		return -5;
	}

	//���������߳�
	if (false == StartRecvThread())
	{
		return -6;
	}

	return 0;
}

int OHttpListener::Stop()
{
	SetRunStatus(false);
	StopProtectThread();

	//��ֹ�¼�ѭ��
	if (m_event_base)
	{
		event_base_loopbreak(m_event_base);
	}

	//�ر��¼��ɷ��߳�
	if (m_dispatch_thread)
	{
		m_dispatch_thread->join();

		delete m_dispatch_thread;
		m_dispatch_thread = NULL;
	}

	//�رս����߳�
	StopRecvThread();

	//��ս��ն���
	if (m_recvQueue)
	{
		m_recvQueue->Clear();
	}

	//ɾ��������
	for (std::list<OHttpProcessor*>::iterator iter = m_http_processor_list.begin(); iter != m_http_processor_list.end(); ++iter)
	{
		OHttpProcessor* http_processor = *iter;
		if (NULL == http_processor)
		{
			continue;
		}

		http_processor->Stop();
		delete http_processor;
		http_processor = NULL;
	}

	m_http_processor_list.clear();

	//�ͷż������
	if (m_evhttp)
	{
		evhttp_free(m_evhttp);
		m_evhttp = NULL;
	}

	if (m_event_base)
	{
		event_base_free(m_event_base);
		m_event_base = NULL;
	}

	return 0;
}

int OHttpListener::Add(OHttpHandlerBase* http_handler)
{
	int ret = 0;

	//���URL��������Դ
	std::unique_lock<std::mutex> locker(m_mutex);

	for (std::list<OHttpProcessor*>::iterator iter = m_http_processor_list.begin(); iter != m_http_processor_list.end(); ++iter)
	{
		OHttpProcessor* http_processor = *iter;
		if (NULL == http_processor)
		{
			continue;
		}

		ret = http_processor->Add(http_handler);
		if (0 != ret)
		{
			break;
		}
	}

	return ret;
}

int OHttpListener::Delete(const char* urlpath)
{
	if (nullptr == urlpath)
	{
		return -1;
	}

	int ret = 0;

	//�ͷ�URL��������Դ
	std::unique_lock<std::mutex> locker(m_mutex);

	for (std::list<OHttpProcessor*>::iterator iter = m_http_processor_list.begin(); iter != m_http_processor_list.end(); ++iter)
	{
		OHttpProcessor* http_processor = *iter;
		if (NULL == http_processor)
		{
			continue;
		}

		ret = http_processor->Delete(urlpath);
		if (0 != ret)
		{
			break;
		}
	}

	return ret;
}

bool OHttpListener::GetRunStatus()
{
	return m_isRunning;
}

void OHttpListener::SetRunStatus(bool flag)
{
	std::unique_lock<std::mutex> locker(m_mutex);

	m_isRunning = flag;
}

XQueue* OHttpListener::GetRecvQueque()
{
	return m_recvQueue;
}

void OHttpListener::OnRecv(HTTPREQUEST* request)
{
	if (NULL == request)
	{
		return;
	}

	bool result = Dispatch(request);

	if (false == result)
	{
		//���ع����쳣��ʾ��ҵ�������̳߳���������,����ٲ�������,����鵼�¿��ٵĶ�д���������ҵ������
		RespondOverLoadExceptionTips(request->req);

		OServer::GetInstance()->Ref()->GetLogger()->Error("http service overload");
	}

	return;
}

bool OHttpListener::Dispatch(HTTPREQUEST* request)
{
	//Ѱ�ҿ��еĴ����������ɴ�������
	bool result = false;

	for (std::list<OHttpProcessor*>::iterator iter = m_http_processor_list.begin(); iter != m_http_processor_list.end(); ++iter)
	{
		OHttpProcessor* http_processor = *iter;
		if (NULL == http_processor)
		{
			continue;
		}

		if (false == http_processor->GetIdleStatus())
		{
			continue;
		}

		//����Ϊռ��״̬
		http_processor->SetIdleStatus(false);

		XQueue* procQueue = http_processor->GetProcQueque();
		if (NULL == procQueue)
		{
			continue;
		}

		procQueue->PushBack(request);

		//���������¼�
		http_processor->SetProcEvent();

		result = true;

		break;
	}

	return result;
}

bool OHttpListener::StartRecvThread()
{
	SetRunStatus(true);

	ResetRecvEvent();

	m_recvThread = new std::thread(RecvThread, this);

	if (m_recvThread)
	{
		return true;
	}

	return false;
}

void OHttpListener::StopRecvThread()
{
	SetRunStatus(false);

	SetRecvEvent();

	if (m_recvThread)
	{
		m_recvThread->join();

		delete m_recvThread;
		m_recvThread = NULL;
	}
}

void* OHttpListener::RecvThread(void* arg)
{
	OHttpListener* obj = (OHttpListener*)arg;
	if (NULL == obj)
	{
		return NULL;
	}

	XQueue* recvQueue = obj->GetRecvQueque();
	if (NULL == recvQueue)
	{
		return NULL;
	}

	while (obj->GetRunStatus())
	{
		HTTPREQUEST* request = (HTTPREQUEST*)recvQueue->PopFront();
		if (NULL == request)
		{
			obj->WaitRecvEvent();
			obj->ResetRecvEvent();
			continue;
		}

		obj->OnRecv(request);
	}

	return NULL;
}

void OHttpListener::SetRecvEvent()
{
	m_recvEvent.Set();
}

void OHttpListener::WaitRecvEvent(unsigned int ms)
{
	if (0 == ms)
	{
		m_recvEvent.Wait();
	}
	else
	{
		m_recvEvent.Wait(ms);
	}
}

void OHttpListener::ResetRecvEvent()
{
	m_recvEvent.Reset();
}

OEvent* OHttpListener::GetRecvEvent()
{
	return &m_recvEvent;;
}

void OHttpListener::SetHighConcurrencyStatus(bool flag)
{
	std::unique_lock<std::mutex> locker(m_isHighConcurrencyMutex);
	m_isHighConcurrency = flag;
}

bool OHttpListener::GetHighConcurrencyStatus()
{
	return m_isHighConcurrency;
}

void OHttpListener::SetReqConcurrencys(bool reset)
{
	std::unique_lock<std::mutex> locker(m_reqConcurrencysMutex);

	if (reset)
	{
		m_reqConcurrencys = 0;
	}
	else
	{
		++m_reqConcurrencys;
	}
}

uint64_t OHttpListener::GetReqConcurrencys()
{
	return m_reqConcurrencys;
}

void OHttpListener::SetOverloadStatus(bool flag)
{
	std::unique_lock<std::mutex> locker(m_isOverloadMutex);
	m_isOverload = flag;
}

bool OHttpListener::GetOverloadStatus()
{
	return m_isOverload;
}

void OHttpListener::AutoSleep(int times, int circle_ms)
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

//���ӶϿ�
static void on_evcnclose_cb(struct evhttp_connection* evcn, void* arg)
{
	printf("evcn close cb++++++++++++++++++++\n");
	struct evhttp_request* req = (struct evhttp_request*)arg;

}

//�������
static void req_on_complete_cb(struct evhttp_request* req, void* arg)
{
	printf("req_on_complete_cb+++++++++++++++++\n");
}

void OHttpListener::MessageCallback(evhttp_request* req, void* arg)
{
	#if 0
	pthread_t tid = pthread_self();
	printf("tid %llu\n", tid.p);
	#endif

	#if 0
	evhttp_request_set_on_complete_cb(req, req_on_complete_cb, NULL);
	evhttp_connection_set_closecb(evhttp_request_get_connection(req), on_evcnclose_cb, req);
	#endif

	OHttpListener* obj = (OHttpListener*)arg;
	if (NULL == obj)
	{
		return;
	}

	XQueue* recvQueue = obj->GetRecvQueque();
	if (NULL == recvQueue)
	{
		return;
	}

	HTTPREQUEST* request = (HTTPREQUEST*)malloc(sizeof(HTTPREQUEST));
	if (NULL == request)
	{
		return;
	}

	request->req = req;
	request->arg = arg;

	recvQueue->PushBack(request);

	//���ý����¼�
	obj->SetRecvEvent();
}

void OHttpListener::RespondOverLoadExceptionTips(evhttp_request* req)
{
	if (NULL == req)
	{
		return;
	}

	int retcode = HTTP_OK;
	std::string retreason = "OK";
	std::string respond;

	//HTTP���������ʾ
	respond = "{\"code\": 501,\"msg\": \"The HTTP Server processor thread pool has been consumed up. Please reduce concurrent requests and check for read/write requests or other business requests that may cause stuttering\"}";
	retcode = HTTP_NOTIMPLEMENTED;
	retreason = "The HTTP Server processor thread pool has been consumed up";

	//��Ӧ����
	struct evbuffer* retbuff = NULL;

	if (respond.length() > 0)
	{
		retbuff = evbuffer_new();

		if (NULL != retbuff)
		{
			#if 0
			evbuffer_add_printf(retbuff, respond.c_str());
			#endif

			evbuffer_add(retbuff, respond.data(), respond.length());
		}
	}

	//���ͻظ�
	evhttp_send_reply(req, retcode, retreason.c_str(), retbuff);

	if (NULL != retbuff)
	{
		evbuffer_free(retbuff);
		retbuff = NULL;
	}

	#if 0
	evhttp_send_reply_end(req);
	#endif
}

bool OHttpListener::StartProtectThread()
{
	SetRunStatus(true);

	m_protectThread = new std::thread(ProtectThread, this);

	if (m_protectThread)
	{
		return true;
	}

	return false;
}

void OHttpListener::StopProtectThread()
{
	SetRunStatus(false);

	if (m_protectThread)
	{
		m_protectThread->join();

		delete m_protectThread;
		m_protectThread = NULL;
	}
}

void* OHttpListener::ProtectThread(void* arg)
{
	OHttpListener* obj = (OHttpListener*)arg;
	if (NULL == obj)
	{
		return NULL;
	}

	uint64_t concurrencys = 0;

	while (obj->GetRunStatus())
	{
		//ÿ1000������һ��
		obj->AutoSleep(2, 500);

		//���ݲ��������Զ��л��߲���ģʽ
		concurrencys = obj->GetReqConcurrencys();
		if (concurrencys > 3)
		{
			obj->SetHighConcurrencyStatus(true);
		}
		else
		{
			obj->SetHighConcurrencyStatus(false);
		}

		if (concurrencys > 1024)
		{
			obj->SetOverloadStatus(true);
		}
		else
		{
			obj->SetOverloadStatus(false);
		}

		//��λ������ͳ��
		obj->SetReqConcurrencys(true);
	}

	return NULL;
}