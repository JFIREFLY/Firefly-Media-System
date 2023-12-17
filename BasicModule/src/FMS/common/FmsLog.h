#ifndef __FMS_LOG_H__
#define __FMS_LOG_H__

#include <string>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <map>
#include <list>
#include <mutex>

#include "XQueue.h"
#include "OEvent.h"
#include "atomic_queue.h"

#ifdef WIN32
#ifdef __WINSOCK2__
#include <winsock2.h>
#else
#include <winsock.h>
#endif
#include <cassert>
#include <io.h>
#else
#include <unistd.h>
#include <assert.h>
#endif

namespace __service_log__
{
#ifdef WIN32
#define LOG_MUTEX CRITICAL_SECTION
#define THREAD_HANDLE HANDLE
#else
#define LOG_MUTEX pthread_mutex_t
#define THREAD_HANDLE  pthread_t
#endif


#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
# define TRUE 0x1
# define FALSE 0
#endif

	typedef enum
	{
		LOGLEVEL_CLOSED = 0,
		LOGLEVEL_FATAL = 1,
		LOGLEVEL_ERROR = 2,
		LOGLEVEL_WARN = 3,
		LOGLEVEL_INFO = 4,
		LOGLEVEL_DEBUG = 5,
		LOGLEVEL_TRACE = 6,
	}EnumLogLevel;

	class CAutoLock
	{
	public:
		CAutoLock(LOG_MUTEX& lock) :m_lock(lock) { LOCK(m_lock); }
		~CAutoLock() { UNLOCK(m_lock); }

		static void InitLock(LOG_MUTEX& lock)
		{
			#ifdef WIN32
			InitializeCriticalSection(&lock);
			#else
			pthread_mutex_init(&lock, NULL);
			#endif // WIN32
		}

		static void UnInitLock(LOG_MUTEX& lock)
		{
			#ifdef WIN32
			DeleteCriticalSection(&lock);
			#else
			pthread_mutex_destroy(&lock);
			#endif 
		}

		static void LOCK(LOG_MUTEX& lock)
		{
			#ifdef WIN32
			EnterCriticalSection(&lock);
			#else
			pthread_mutex_lock(&lock);
			#endif // WIN32
		}

		static void UNLOCK(LOG_MUTEX& lock)
		{
			#ifdef WIN32
			LeaveCriticalSection(&lock);
			#else
			pthread_mutex_unlock(&lock);
			#endif 
		}

	private:
		LOG_MUTEX& m_lock;
		};

	class FmsLog
	{
	public:
		//�ռ��ļ���:ModuleName_yy-MM-DDTHH:MI:SS.log
		//Ĭ���ڵ�ǰ·���������ռ�
		FmsLog(const char* szModule, const char* szLogPath = NULL);

		~FmsLog();

	public:
		//�����Ƿ�����ˢ����־��־,0=������ˢ�£�1=����ˢ�£�ȱʡ��
		void setFlushFlag(int flag) { m_nFlushFlag = flag; }

		//�����Ƿ��ӡ����,Ĭ�ϴ�ӡ
		void setWithMillisecond(int flag) { m_hasMillilSecond = flag; }

		//������־�ļ��������(��λKB),��־�ļ���С�����������ֵ���Զ����ݲ��������ļ�������־��� 
		//Ĭ��Ϊ50M
		void setMaxCapacity(unsigned long nCapacity) { m_nCapacity = nCapacity; }

		//������־����ȼ�
		//���ڻ���ڸõȼ����ռǽ��ᱻ��������ڸõȼ����ռǽ��ᱻ����
		//Ĭ�ϵȼ�ΪLOGLEVEL_DEBUG,��Ĭ�ϻ���������ռ�
		void setLogLevel(EnumLogLevel eLevel) { m_eLogLevel = eLevel; }

		//�����Ƿ������Զ�ѹ������ǰһ����ռ��ļ�
		//Ĭ��Ϊ����
		void setAutoCompress(int flag) { m_nCompress = flag; }

		//������־�ļ�����ʱ��(��λ��),��־�ļ��������ڳ���ʱ����ɾ��
		//Ĭ��Ϊ7��
		void setKeepDays(int day) { m_keepDays = day; }

		//�ֶ�ˢ����־��� 
		void flush();

	public:
		//��ͨ��־����ͷ��ӡʱ�䡢ģ����
		//format:YY-MM-DD HH:MI:SS:MS [Module] Level .......
		void print(EnumLogLevel level, const char* format, ...);

		//��ͨ��־����ͷ��ӡʱ�䡢ģ������ֱ�Ӵ�ӡ�ַ���
		//format:YY-MM-DD HH:MI:SS:MS [Module] Level .......
		void printstr(EnumLogLevel level, const char* str);
		void print(const char* str);

		//��ͨ��־����ͷ��ӡʱ��
		//format:YY-MM-DD HH:MI:SS:MS Level
		void printstrExt(EnumLogLevel level, const char* str);

		//��ͨ��־����ͷ��ӡʱ�䡢ģ������ID
		void print(EnumLogLevel level, int ID, const char* format, ...);

		//��ͨ��־����ͷ��ӡʱ�䡢ID��qualifier
		//format: MMDD HH:MI:SS <qualifier>[ID]
		void print(EnumLogLevel level, unsigned int qualifier, unsigned int ID, const char* format, ...);

		//��ͨ��־����ͷ����ӡʱ��
		void printnt(EnumLogLevel level, const char* format, ...);

	public:
		//��������־����ͷ��ӡʱ��
		void printb(EnumLogLevel level, const char* title, const unsigned char* buf, int len);

		//��������־����ͷ��ӡʱ��
		void printb(EnumLogLevel level, unsigned int ID, const char* title, const unsigned char* buf, int len);

		//��������־����ͷ����ӡʱ��
		void printbnt(EnumLogLevel level, const unsigned char* buf, int len);

	public:
		//���Դ��ݱ䳤����ָ����д�ӡ,��ͷ��ӡʱ��
		void vprint(EnumLogLevel level, const char* format, va_list argptr);

		//���Դ��ݱ䳤����ָ����д�ӡ,��ͷ��ӡʱ���ID
		void vprint(EnumLogLevel level, int ID, const char* format, va_list argptr);

		//format:<guid>[ID]content...
		void vprint(EnumLogLevel level, unsigned int guid, unsigned int ID, const char* format, va_list argptr);

	private:
		#ifdef WIN32
		static unsigned __stdcall ThreadFunc(void* arg);
		#else
		static void* ThreadFunc(void* arg);
		#endif

		#ifdef WIN32
		static unsigned __stdcall SyncProcThread(void* arg);
		#else
		static void* SyncProcThread(void* arg);
		#endif

		void printb(const unsigned char* buf, int len);

		//����
		void Backup();

		//ѹ��
		void Compress();

		//ɾ������������־
		void DelFile();

		int GetMillisecond();

		int removeDir(std::string dirPath);

		void CompressFile(const std::string& strFileFullName, const std::string& strBackupFullPath);
		void CompressFile(const std::string& strRelativePath, std::string& strFileFullName, const std::string& strBackupFullPath);

		std::string GetZipCmdLine(const std::string& strFromFileFullName, const std::string& strToFileFullNameWithoutZipExt);
		std::string GetTarCmdLine(const std::string& strFromFileFullName, const std::string& strFolder, const std::string& strRelativePath);

		//��ȡ�ļ�Ŀ¼��С
		uint64_t GetFolderSize(std::string path, std::map<int64_t, std::string>& mapFileName);

		//�첽д��־
		void SyncProc();

		bool PushBack(std::string& str);
		void PopFront(std::list<std::string>& list);


		void MySleep(int nMsec);

	private:
		FILE* m_pFile;
		LOG_MUTEX     m_Lock;
		std::string	  m_module;
		std::string   m_path;
		std::string   m_CompressFolder;
		int           m_nFlushFlag;
		int           m_eLogLevel;
		int           m_hasMillilSecond;
		int           m_nCompress;
		int			  m_keepDays;
		unsigned long m_nCapacity;
		unsigned long m_nTotalSize;
		struct tm     m_stCurDate;
		THREAD_HANDLE m_hCompressThread;
		bool          m_nStopFlag;

		int           m_nSyncProc;
		std::list<std::string> m_procQueue;
		OEvent		  m_procEvent;
		THREAD_HANDLE m_hSyncProcThread;
		std::mutex    m_mutex;

		atomic_queue::AtomicQueueB2<std::string, std::allocator<std::string>>* m_atomicQueue;

	private:
		FmsLog(const FmsLog& rhs);
		FmsLog& operator=(const FmsLog& rhs);
	};
}

#endif //__FMS_LOG_H__

