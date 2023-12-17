#ifndef __XQUEUE__
#define __XQUEUE__

#include <stdlib.h>
#include <deque>

#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

#define NORMAL_TYPE   0
#define OBJECT_TYPE   1
#define PACKET_TYPE   2


class XQueue
{
public:
	XQueue(int type);
	~XQueue();

private:
	#ifdef WIN32
	  CRITICAL_SECTION	mutex;
	#else
		pthread_mutex_t	mutex; 
	#endif
	std::deque<void*>	queue;
	int	dwtype;
		
public:
	void	PushBack(void *obj);
	void* PopFront();
	void	Clear();
	int	Size();
};


#endif
