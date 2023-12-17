#ifndef _THREAD_H_
#define _THREAD_H_

#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC 1
#endif // !HAVE_STRUCT_TIMESPEC

#include <pthread.h>
#include <mutex>

#ifndef WIN32
#define PTW32_CDECL
#endif

/*
  void * thread (void * arg)
  {
      return NULL;
  }
*/

namespace CrossPlatform {

	// 定义线程函数
	typedef void* ThreadFunction(void *);

	class OThread
	{
	public:
		OThread();
		virtual ~OThread();

		int GetStatus();

		int Start(ThreadFunction start, void *arg);

		int Cancel();

		int Join();
		int Detach();

		int Kill(int sig);

	private:
		pthread_t m_thread;
		int m_ret;
	};

}

#endif // _THREAD_H_
