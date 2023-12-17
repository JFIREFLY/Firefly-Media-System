#ifndef __RPC_HANDLE__
#define __RPC_HANDLE__
#include "ORpcBaseHandle.h"
#include "OStreamChannel.h"

#define MAX_CHANNEL 15

template <class AsyncService, class request, class response>
class ORpcServiceHandle : public ORpcBaseHandle
{
public:
	ORpcServiceHandle(grpc::ServerBuilder* builder, int max_channel = MAX_CHANNEL);
	~ORpcServiceHandle();

	int Start();
	void Stop();

	void Write(void* channel, void* rsp);

private:
	bool GetRunStatus();
	void SetRunStatus(bool flag);

	bool StartLoopThread();
	void StopLoopThread();
	static void* LoopThread(void* arg, void* service, grpc::ServerCompletionQueue* cq);

	bool StartCheckThread();
	void StopCheckThread();
	static void* CheckThread(void* arg, void* service, grpc::ServerCompletionQueue* cq);

	std::list<OStreamChannel<AsyncService, request, response>*>* GetChannelList();
	void AutoSleep(int times /*= 100*/, int circle_ms /*= 10*/);

private:
	AsyncService m_service;
	std::unique_ptr<grpc::ServerCompletionQueue> m_scq;
	std::list<OStreamChannel<AsyncService, request, response>*> m_channel_list;

private:
	bool m_isRunning;
	std::mutex m_mutex;
	int m_maxChannel;
	std::thread* m_loopThread;
	std::thread* m_checkThread;
};



template <class AsyncService, class request, class response>
void ORpcServiceHandle<AsyncService, request, response>::Write(void* channel, void* rsp)
{
	if (NULL == channel || rsp == NULL)
	{
		return;
	}

	for (typename std::list<OStreamChannel<AsyncService, request, response>*>::iterator it = m_channel_list.begin(); it != m_channel_list.end(); ++it)
	{
		if ((*it) != channel)
		{
			continue;
		}

		((OStreamChannel<AsyncService, request, response>*)channel)->Write((response*)rsp);
	}
}

template <class AsyncService, class request, class response>
void ORpcServiceHandle<AsyncService, request, response>::AutoSleep(int times /*= 100*/, int circle_ms /*= 10*/)
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

		#ifdef WIN32
		Sleep(circle_ms);
		#else
		usleep(circle_ms*1000)
		#endif // WIN32

		++ticks;

	} while (GetRunStatus());
}

template <class AsyncService, class request, class response>
std::list<OStreamChannel<AsyncService, request, response>*>* ORpcServiceHandle<AsyncService, request, response>::GetChannelList()
{
	return &m_channel_list;
}

template <class AsyncService, class request, class response>
void* ORpcServiceHandle<AsyncService, request, response>::CheckThread(void* arg, void* service, grpc::ServerCompletionQueue* cq)
{
	ORpcServiceHandle* obj = (ORpcServiceHandle*)arg;
	if (NULL == obj)
	{
		return NULL;
	}


	std::list<OStreamChannel<AsyncService, request, response>*>* list_ptr = obj->GetChannelList();
	if (NULL == list_ptr)
	{
		return NULL;
	}

	while (obj->GetRunStatus())
	{
		int cnt = 0;

		//释放连接端口的句柄
		for (typename std::list<OStreamChannel<AsyncService, request, response>*>::iterator it = list_ptr->begin(); it != list_ptr->end(); )
		{		
			if (OStreamChannel<AsyncService, request, response>::E_CHANNEL_STATUS_DISCONNECT == (*it)->GetChannelStatus())
			{
				delete (*it);
				list_ptr->erase(it++);
				++cnt;
			}
			else
			{
				++it;
			}
		}

		//补齐通道数量
		for (int i = 0; i < cnt; ++i)
		{
			OStreamChannel<AsyncService, request, response>* channel = new OStreamChannel<AsyncService, request, response>((AsyncService*)service, cq, obj);

			if (NULL == channel)
			{
				continue;
			}

			list_ptr->push_back(channel);
		}

		obj->AutoSleep(2, 500);
	}

	return NULL;
}

template <class AsyncService, class request, class response>
void ORpcServiceHandle<AsyncService, request, response>::StopCheckThread()
{
	SetRunStatus(false);

	if (m_checkThread)
	{
		m_checkThread->join();

		delete m_checkThread;
		m_checkThread = NULL;
	}
}

template <class AsyncService, class request, class response>
bool ORpcServiceHandle<AsyncService, request, response>::StartCheckThread()
{
	SetRunStatus(true);

	if (m_checkThread)
	{
		return true;
	}

	m_checkThread = new std::thread(CheckThread, this, &m_service, m_scq.get());

	if (m_checkThread)
	{
		return true;
	}

	return false;
}

template <class AsyncService, class request, class response>
void ORpcServiceHandle<AsyncService, request, response>::SetRunStatus(bool flag)
{
	std::unique_lock<std::mutex> locker(m_mutex);

	m_isRunning = flag;
}

template <class AsyncService, class request, class response>
bool ORpcServiceHandle<AsyncService, request, response>::GetRunStatus()
{
	return m_isRunning;
}

template <class AsyncService, class request, class response>
void* ORpcServiceHandle<AsyncService, request, response>::LoopThread(void* arg, void* service, grpc::ServerCompletionQueue* cq)
{
	ORpcServiceHandle* obj = (ORpcServiceHandle*)arg;
	if (NULL == obj)
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

template <class AsyncService, class request, class response>
void ORpcServiceHandle<AsyncService, request, response>::StopLoopThread()
{
	SetRunStatus(false);

	if (m_loopThread)
	{
		m_loopThread->join();

		delete m_loopThread;
		m_loopThread = NULL;
	}
}

template <class AsyncService, class request, class response>
bool ORpcServiceHandle<AsyncService, request, response>::StartLoopThread()
{
	SetRunStatus(true);

	if (m_loopThread)
	{
		return true;
	}

	m_loopThread = new std::thread(LoopThread, this, &m_service, m_scq.get());

	if (m_loopThread)
	{
		return true;
	}

	return false;
}

template <class AsyncService, class request, class response>
void ORpcServiceHandle<AsyncService, request, response>::Stop()
{
	StopCheckThread();
	StopLoopThread();

	//释放通道
	for (typename std::list<OStreamChannel<AsyncService, request, response>*>::iterator it = m_channel_list.begin(); it != m_channel_list.end(); ++it)
	{
		delete (*it);
		*it = NULL;
	}

	m_channel_list.clear();

	m_scq.reset();
}

template <class AsyncService, class request, class response>
int ORpcServiceHandle<AsyncService, request, response>::Start()
{
	if (NULL == m_scq.get())
	{
		return -1;
	}

	//创建通道
	for (int i = 0; i < m_maxChannel; ++i)
	{
		OStreamChannel<AsyncService, request, response>* channel = new OStreamChannel<AsyncService, request, response>(&m_service, m_scq.get(), this);

		if (NULL == channel)
		{
			continue;
		}

		m_channel_list.push_back(channel);
	}

	//启动事件监听
	if (false == StartLoopThread())
	{
		return -2;
	}

	//启动通道校验
	if (false == StartCheckThread())
	{
		return -3;
	}

	return 0;
}

template <class AsyncService, class request, class response>
ORpcServiceHandle<AsyncService, request, response>::~ORpcServiceHandle()
{
	Stop();
}

template <class AsyncService, class request, class response>
ORpcServiceHandle<AsyncService, request, response>::ORpcServiceHandle(grpc::ServerBuilder* builder, int max_channel)
{
	m_loopThread = NULL;
	m_checkThread = NULL;
	m_isRunning = false;
	m_maxChannel = max_channel;

	builder->RegisterService(&m_service);
	m_scq = builder->AddCompletionQueue();
}

#endif