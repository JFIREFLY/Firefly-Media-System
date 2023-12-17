
#include "XQueue.h"
#include "Public.h"

XQueue::XQueue(int type)
{
#ifdef WIN32
	InitializeCriticalSection(&mutex);
#else
	pthread_mutex_init(&mutex, NULL);
#endif 
	dwtype = type;
}

XQueue::~XQueue()
{
	if(!queue.empty())
	{
		Clear();
	}

#ifdef WIN32
	DeleteCriticalSection(&mutex);
#else
	pthread_mutex_destroy(&mutex);
#endif	
}

void XQueue::PushBack(void *obj)
{
#ifdef WIN32
	EnterCriticalSection(&mutex);
#else
	pthread_mutex_lock(&mutex);
#endif

	queue.push_back(obj);
	
#ifdef WIN32
	LeaveCriticalSection(&mutex);
#else
	pthread_mutex_unlock(&mutex);
#endif
}

void* XQueue::PopFront()
{
#ifdef WIN32
	EnterCriticalSection(&mutex);
#else
	pthread_mutex_lock(&mutex);
#endif

	void *obj = NULL;
    
	if (!queue.empty())
	{
		obj = queue.front();
		queue.pop_front();
	}
	
#ifdef WIN32
	LeaveCriticalSection(&mutex);
#else
	pthread_mutex_unlock(&mutex);
#endif

	return obj;
}

void  XQueue::Clear()
{
#ifdef WIN32
	EnterCriticalSection(&mutex);
#else
	pthread_mutex_lock(&mutex);
#endif

	if(!queue.empty())
	{
		std::deque<void *>::iterator It = queue.begin();
		while(It < queue.end())
		{	
			if(dwtype == NORMAL_TYPE)
			{
				free(*It);
			}
			else if(dwtype == PACKET_TYPE)
			{
				FreePacket((PACKET_T*)*It);
			}
			else
			{
			}
			*It = NULL;
			It = queue.erase(It);
		}
	}
	
#ifdef WIN32
	LeaveCriticalSection(&mutex);
#else
	pthread_mutex_unlock(&mutex);
#endif
}

int  XQueue::Size()
{
    #ifdef WIN32
	EnterCriticalSection(&mutex);
#else
	pthread_mutex_lock(&mutex);
#endif

    int size = (int)queue.size();

    #ifdef WIN32
	LeaveCriticalSection(&mutex);
#else
	pthread_mutex_unlock(&mutex);
#endif

    return size;
}
