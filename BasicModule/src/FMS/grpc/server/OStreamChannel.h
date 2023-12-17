#ifndef __RPC_SERVER_TEMPLATE__
#define __RPC_SERVER_TEMPLATE__

#include "RpcServerTemplate.h"
#include "RpcBusinessTemplate.h"

using TagType = std::function<void(bool)>;

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

template <class AsyncService, class request, class response>
class OStreamChannel : public ICallback
{
public:
	typedef enum CHANNEL_STATUS
	{
		E_CHANNEL_STATUS_UNCONNECT = 0,
		E_CHANNEL_STATUS_CONNECTED = 1,
		E_CHANNEL_STATUS_DISCONNECT = 2,

		E_CHANNEL_STATUS_UNDEFINE
	};

public:
	OStreamChannel(AsyncService* service, grpc::ServerCompletionQueue* cq, void* context);
	~OStreamChannel();

	void OnConnected(bool ok);
	void OnReadDone(bool ok);
	void OnWriteDone(bool ok);
	void OnDisconnect(bool ok);

	int GetChannelStatus();
	void SetChannelStatus(int status);

	void Write(response* data);

private:
	grpc::ServerCompletionQueue* m_scq_ptr;
	AsyncService* m_service_ptr;
	grpc::ServerContext m_ctx;
	grpc::ServerAsyncReaderWriter<response, request> m_stream;
	request m_buffer;
	std::mutex m_mutex;
	int m_channelStatus;
	void* m_context;

	//函数指针
	TagType m_connectedFunc;	//连接服务端时触发
	TagType m_readDoneFunc;		//读到新消息时触发
	TagType m_writeDoneFunc;	//发送一帧消息成功后触发
	TagType m_disconnectFunc;	//stream断开时触发
};

template <class AsyncService, class request, class response>
void OStreamChannel<AsyncService, request, response>::Write(response* data)
{
	if (data == NULL)
	{
		return;
	}

	m_stream.Write(*data, &m_writeDoneFunc);
}

template <class AsyncService, class request, class response>
void OStreamChannel<AsyncService, request, response>::SetChannelStatus(int status)
{
	std::unique_lock<std::mutex> locker(m_mutex);

	m_channelStatus = status;
}

template <class AsyncService, class request, class response>
int OStreamChannel<AsyncService, request, response>::GetChannelStatus()
{
	return m_channelStatus;
}

template <class AsyncService, class request, class response>
void OStreamChannel<AsyncService, request, response>::OnDisconnect(bool ok)
{
	printf("断开连接[%d]\n", ok);
	SetChannelStatus(E_CHANNEL_STATUS_DISCONNECT);
}

template <class AsyncService, class request, class response>
void OStreamChannel<AsyncService, request, response>::OnWriteDone(bool ok)
{
	printf("write[%d]\n", ok);
}

template <class AsyncService, class request, class response>
void OStreamChannel<AsyncService, request, response>::OnReadDone(bool ok)
{
	if (ok)
	{
		BusinessTemplate::MsgCallbackTemplate(this, &m_buffer, m_context);
	}

	if (GetChannelStatus() == E_CHANNEL_STATUS_CONNECTED)
	{
		m_stream.Read(&m_buffer, &m_readDoneFunc);
	}
}

template <class AsyncService, class request, class response>
void OStreamChannel<AsyncService, request, response>::OnConnected(bool ok)
{
	if (ok)
	{
		printf("新的连接 连接成功[%d]\n", ok);

		SetChannelStatus(E_CHANNEL_STATUS_CONNECTED);

		m_stream.Read(&m_buffer, &m_readDoneFunc);
	}
	else
	{
		printf("新的连接 连接失败[%d]\n", ok);
	}

}

template <class AsyncService, class request, class response>
OStreamChannel<AsyncService, request, response>::OStreamChannel(AsyncService* service, grpc::ServerCompletionQueue* cq, void* context) : m_service_ptr(service), m_scq_ptr(cq), m_stream(&m_ctx), m_context(context)
{
	m_channelStatus = E_CHANNEL_STATUS_UNCONNECT;

	//使用std::bind绑定对象和类对象函数得到一个函数指针
	m_connectedFunc = std::bind(&OStreamChannel::OnConnected, this, std::placeholders::_1);
	m_readDoneFunc = std::bind(&OStreamChannel::OnReadDone, this, std::placeholders::_1);
	m_writeDoneFunc = std::bind(&OStreamChannel::OnWriteDone, this, std::placeholders::_1);
	m_disconnectFunc = std::bind(&OStreamChannel::OnDisconnect, this, std::placeholders::_1);

	//设置离线事件函数
	m_ctx.AsyncNotifyWhenDone(&m_disconnectFunc);

	//设置连接事件函数
	ServerTemplate::RequestTemplate<AsyncService>(m_service_ptr, &m_ctx, &m_stream, m_scq_ptr, m_scq_ptr, &m_connectedFunc);
}


template <class AsyncService, class request, class response>
OStreamChannel<AsyncService, request, response>::~OStreamChannel()
{
}

#endif