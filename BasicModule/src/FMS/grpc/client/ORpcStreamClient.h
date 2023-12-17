#ifndef __RPC_STREAM_CLIENT__
#define __RPC_STREAM_CLIENT__

#include <string>

#include "RpcClientTemplate.h"

#define RPC_CONNECTED 1
#define RPC_DISCONNECT 2

//使用说明
//代码实现：
//1.在RpcClientTemplate.h中，特例化对应PB结构的NewStubTemplate和AsyncTemplate模板函数，函数实现固定，参照变量推送模板

//用法:
//1.初始化服务端IP和端口
//2.设置事件回调和消息回调
//3.调用Open打开数据流通道


namespace RpcClient {

class ICallback
{
public:
	ICallback() {};
	~ICallback() {};

	virtual void OnConnected(bool ok) = 0;
	virtual void OnReadDone(bool ok) = 0; 
	virtual void OnWriteDone(bool ok) = 0;
	virtual void OnDisconnect(bool ok) = 0;
};

template <class stub, class request, class response>
class ORpcStreamClient : public ICallback
{
public:
	using TagType = std::function<void(bool)>;

	typedef void(*EVENT_CALLBACK)(int events, void* context);
	typedef void(*MSG_CALLBACK)(void* data, void* context);

	void OnConnected(bool ok);
	void OnReadDone(bool ok);
	void OnWriteDone(bool ok);
	void OnDisconnect(bool ok);

public:
	ORpcStreamClient(const char* ip, unsigned short port);
	~ORpcStreamClient();

	int Open();
	void Close();

	void Write(request& req);

	void OnMsgCallback(void* data);
	void SetMsgCallback(MSG_CALLBACK callback, void* context);

	void OnEventCallback(int events);
	void SetEventCallback(EVENT_CALLBACK callback, void* context);
private:
	bool GetRunStatus();
	void SetRunStatus(bool flag);

	bool GetConnectStatus();
	void SetConnectStatus(bool status);

	grpc::CompletionQueue* GetCompletionQueue();

	bool StartLoopThread();
	void StopLoopThread();
	static void* LoopThread(void* arg);

private:
	grpc::ClientContext* m_ctx;
	grpc::CompletionQueue* m_cq;
	std::unique_ptr<grpc::ClientAsyncReaderWriter<request, response>> m_stream_ptr;
	std::shared_ptr<grpc::Channel> m_channel_ptr;
	std::unique_ptr<stub> m_stub_ptr;
	grpc::Status m_status;
	response m_buffer;

	//函数指针
	TagType m_connectedFunc;	//连接服务端时触发
	TagType m_readDoneFunc;		//读到新消息时触发
	TagType m_writeDoneFunc;	//发送一帧消息成功后触发
	TagType m_disconnectFunc;	//stream断开时触发
private:
	std::string m_ip;
	unsigned short m_port;
	bool m_isRunning;
	bool m_isConnect;
	std::mutex m_mutex;
	std::thread* m_loopThread;

	std::mutex m_eventMutex;
	std::mutex m_msgMutex;
	EVENT_CALLBACK m_eventCallback;
	MSG_CALLBACK m_msgCallback;
	void* m_msgCallbackContext;
	void* m_eventCallbackContext;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------        分割线             -----------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------

template <class stub, class request, class response>
ORpcStreamClient<stub, request, response>::ORpcStreamClient(const char* ip, unsigned short port)
{
	m_ip = ip;
	m_port = port; 
	m_isRunning = false;
	m_isConnect = false;
	m_loopThread = NULL;
	m_ctx = NULL;
	m_cq = NULL;

	//使用std::bind绑定对象和类对象函数得到一个函数指针
	m_connectedFunc = std::bind(&ORpcStreamClient::OnConnected, this, std::placeholders::_1);
	m_readDoneFunc = std::bind(&ORpcStreamClient::OnReadDone, this, std::placeholders::_1);
	m_writeDoneFunc = std::bind(&ORpcStreamClient::OnWriteDone, this, std::placeholders::_1);
	m_disconnectFunc = std::bind(&ORpcStreamClient::OnDisconnect, this, std::placeholders::_1);
}

template <class stub, class request, class response>
ORpcStreamClient<stub, request, response>::~ORpcStreamClient()
{
	Close();
}


template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::OnDisconnect(bool ok)
{
	SetConnectStatus(false);
	OnEventCallback(RPC_DISCONNECT);
}

template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::OnWriteDone(bool ok)
{
	printf("write done\n");
}

template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::OnReadDone(bool ok)
{
	if (ok)
	{
		OnMsgCallback(&m_buffer);
	}

	if (m_isConnect)
	{
		m_stream_ptr->Read(&m_buffer, &m_readDoneFunc);
	}
}

template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::OnConnected(bool ok)
{
	if (ok)
	{
		SetConnectStatus(true);

		m_stream_ptr->Read(&m_buffer, &m_readDoneFunc);
		OnEventCallback(RPC_CONNECTED);
	}
	else
	{
		SetConnectStatus(false);

		OnEventCallback(RPC_DISCONNECT);
	}
	
}

template <class stub, class request, class response>
int ORpcStreamClient<stub, request, response>::Open()
{
	if (NULL == m_ctx)
	{
		m_ctx = new grpc::ClientContext;
	}

	if (NULL == m_cq)
	{
		m_cq = new grpc::CompletionQueue;
	}

	if (NULL == m_ctx || NULL == m_cq)
	{
		return -1;
	}

	m_channel_ptr = grpc::CreateChannel(m_ip + ":" + std::to_string(m_port), grpc::InsecureChannelCredentials());
	if (NULL == m_channel_ptr.get())
	{
		return -2;
	}

	m_stub_ptr = ClientTemplate::NewStubTemplate<stub>(m_channel_ptr);
	if (NULL == m_stub_ptr.get())
	{
		return -3;
	}

	//发起grpc连接，不管是成功还是失败，cq.AsyncNext都会返回connectedFunc这个tag，成功时，tag对应的ok为true
	m_stream_ptr = ClientTemplate::AsyncTemplate<stub, request, response>(m_stub_ptr, m_ctx, m_cq, &m_connectedFunc);
	if (NULL == m_stream_ptr.get())
	{
		return -4;
	}

	//设置Finish的tag，stream断开时，CompletionQueue会返回一个tag，这个tag就是输入的disconnectFunc这个函数指针
	m_stream_ptr->Finish(&m_status, &m_disconnectFunc);

	return !StartLoopThread();
}

template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::Close()
{
	SetConnectStatus(false);
	StopLoopThread();

	if (m_cq)
	{
		m_cq->Shutdown();
		delete m_cq;
		m_cq = NULL;
	}

	m_stream_ptr.reset();
	m_stub_ptr.reset();
	m_channel_ptr.reset();

	if (m_ctx)
	{
		m_ctx->TryCancel();
		delete m_ctx;
		m_ctx = NULL;
	}

	OnEventCallback(RPC_DISCONNECT);
}

template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::Write(request& req)
{
	if (false == m_isConnect)
	{
		return;
	}

	if (NULL == m_channel_ptr.get() || NULL == m_stub_ptr.get() || NULL == m_stream_ptr.get())
	{
		return;
	}

	m_stream_ptr->Write(req, &m_writeDoneFunc);
}

template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::SetMsgCallback(MSG_CALLBACK callback, void* context)
{
	std::unique_lock<std::mutex> locker(m_mutex);

	m_msgCallbackContext = context;
	m_msgCallback = callback;
}

template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::OnMsgCallback(void* data)
{
	if (NULL == m_msgCallback)
	{
		return;
	}

	std::unique_lock<std::mutex> locker(m_msgMutex);

	m_msgCallback(data, m_msgCallbackContext);
}


template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::SetEventCallback(EVENT_CALLBACK callback, void* context)
{
	std::unique_lock<std::mutex> locker(m_eventMutex);

	m_eventCallbackContext = context;
	m_eventCallback = callback;
}

template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::OnEventCallback(int events)
{
	if (NULL == m_eventCallback)
	{
		return;
	}

	std::unique_lock<std::mutex> locker(m_eventMutex);

	m_eventCallback(events, m_eventCallbackContext);
}


template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::SetRunStatus(bool flag)
{
	std::unique_lock<std::mutex> locker(m_mutex);

	m_isRunning = flag;
}

template <class stub, class request, class response>
bool ORpcStreamClient<stub, request, response>::GetRunStatus()
{
	return m_isRunning;
}

template <class stub, class request, class response>
bool ORpcStreamClient<stub, request, response>::GetConnectStatus()
{
	return m_isConnect;
}

template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::SetConnectStatus(bool status)
{
	std::unique_lock<std::mutex> locker(m_eventMutex);

	m_isConnect = status;
}

template <class stub, class request, class response>
grpc::CompletionQueue* ORpcStreamClient<stub, request, response>::GetCompletionQueue()
{
	return m_cq;
}

template <class stub, class request, class response>
void* ORpcStreamClient<stub, request, response>::LoopThread(void* arg)
{
	ORpcStreamClient* obj = (ORpcStreamClient*)arg;
	if (NULL == obj)
	{
		return NULL;
	}

	grpc::CompletionQueue* cq = obj->GetCompletionQueue();
	if (NULL == cq)
	{
		return NULL;
	}

	while (obj->GetRunStatus())
	{
		void* tag = NULL;
		bool ok = false;

		//阻塞100毫秒，gpr_time_from_millis()函数的单位是毫秒,输入的是tag和ok的地址，cq->AsyncNext()会把结果写到地址对应的内存上
		grpc::CompletionQueue::NextStatus status = cq->AsyncNext(&tag, &ok, gpr_time_from_millis(100, GPR_TIMESPAN));

		if (status == grpc::CompletionQueue::NextStatus::GOT_EVENT) 
		{
			//grpc服务器有新的事件，强制转换tag从void * 到 std::function<void(bool)> *，即void *(bool) 函数指针
			TagType* functionPointer = reinterpret_cast<TagType*>(tag);

			if (functionPointer)
			{
				(*functionPointer)(ok);
			}		
		}
	}

	return NULL;
}

template <class stub, class request, class response>
void ORpcStreamClient<stub, request, response>::StopLoopThread()
{
	SetRunStatus(false);

	if (m_loopThread)
	{
		m_loopThread->join();

		delete m_loopThread;
		m_loopThread = NULL;
	}
}

template <class stub, class request, class response>
bool ORpcStreamClient<stub, request, response>::StartLoopThread()
{
	SetRunStatus(true);

	if (m_loopThread)
	{
		return true;
	}

	m_loopThread = new std::thread(LoopThread, this);

	if (m_loopThread)
	{
		return true;
	}

	return false;
}

}; //RpcCilent

#endif