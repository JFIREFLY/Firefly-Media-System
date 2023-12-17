#include "OThread.h"

#ifdef WIN32
#pragma comment(lib, "pthreadVC2.lib")
#else
#include <signal.h>
#endif // WIN32

namespace CrossPlatform {

	OThread::OThread()
		: m_ret(-1)
	{

	}

	OThread::~OThread()
	{
		Cancel();
	}

	int OThread::GetStatus()
	{
		return m_ret;
	}

	int OThread::Start(ThreadFunction start, void *arg)
	{
		m_ret = pthread_create(&m_thread, NULL, start, arg);
		return m_ret;
	}

	int OThread::Cancel()
	{
		if (m_ret == -1)
			return 0;

		return pthread_cancel(m_thread);
	}

	int OThread::Join()
	{
		if (m_ret == -1)
			return 0;

		return pthread_join(m_thread, NULL);
	}

	int OThread::Detach()
	{
		if (m_ret == -1)
			return 0;

		return pthread_detach(m_thread);
	}

	int OThread::Kill(int sig)
	{
		if (m_ret == -1)
			return 0;

		return pthread_kill(m_thread, sig);
	}

}