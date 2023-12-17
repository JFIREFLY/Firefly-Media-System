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

	//����ָ��
	TagType m_connectedFunc;	//���ӷ����ʱ����
	TagType m_readDoneFunc;		//��������Ϣʱ����
	TagType m_writeDoneFunc;	//����һ֡��Ϣ�ɹ��󴥷�
	TagType m_disconnectFunc;	//stream�Ͽ�ʱ����
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
	printf("�Ͽ�����[%d]\n", ok);
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
		printf("�µ����� ���ӳɹ�[%d]\n", ok);

		SetChannelStatus(E_CHANNEL_STATUS_CONNECTED);

		m_stream.Read(&m_buffer, &m_readDoneFunc);
	}
	else
	{
		printf("�µ����� ����ʧ��[%d]\n", ok);
	}

}

template <class AsyncService, class request, class response>
OStreamChannel<AsyncService, request, response>::OStreamChannel(AsyncService* service, grpc::ServerCompletionQueue* cq, void* context) : m_service_ptr(service), m_scq_ptr(cq), m_stream(&m_ctx), m_context(context)
{
	m_channelStatus = E_CHANNEL_STATUS_UNCONNECT;

	//ʹ��std::bind�󶨶������������õ�һ������ָ��
	m_connectedFunc = std::bind(&OStreamChannel::OnConnected, this, std::placeholders::_1);
	m_readDoneFunc = std::bind(&OStreamChannel::OnReadDone, this, std::placeholders::_1);
	m_writeDoneFunc = std::bind(&OStreamChannel::OnWriteDone, this, std::placeholders::_1);
	m_disconnectFunc = std::bind(&OStreamChannel::OnDisconnect, this, std::placeholders::_1);

	//���������¼�����
	m_ctx.AsyncNotifyWhenDone(&m_disconnectFunc);

	//���������¼�����
	ServerTemplate::RequestTemplate<AsyncService>(m_service_ptr, &m_ctx, &m_stream, m_scq_ptr, m_scq_ptr, &m_connectedFunc);
}


template <class AsyncService, class request, class response>
OStreamChannel<AsyncService, request, response>::~OStreamChannel()
{
}

#endif