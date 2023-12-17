#include "FmsLog.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#ifdef WIN32
#include <sys/timeb.h>
#include <time.h>
#include <Shlwapi.h>
#include <process.h>
#include <direct.h>
#include <fstream> 
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <dirent.h>
#endif

#include "Public.h"

using namespace __service_log__;

#ifdef WIN32
#else
#define _access(path,mode)		access(path,mode)
#define _mkdir(path)			mkdir(path,0777)
#endif
#define LOG_MSG_BUF_SIZE		(1024 * 100)

#ifdef WIN32
#define DEFAULT_MAX_LOG_CAPACITY (50 * 1024 * 1024) //100G (KB为单位)
#define DEFAULT_FILE_CAPACITY   (50 * 1024)			 // 50M (KB为单位)
#define DEFAULT_LOG_KEEPDAYS     90					 // 7天

#define QUEUE_MAX_SIZE 10000						 //队列缓存大小
#else
#define DEFAULT_MAX_LOG_CAPACITY (10 * 1024)		 //10M (KB为单位)
#define DEFAULT_FILE_CAPACITY   (1024)				 // 1M (KB为单位)
#define DEFAULT_LOG_KEEPDAYS     7					 // 7天

#define QUEUE_MAX_SIZE 500							 //队列缓存大小
#endif

FmsLog::FmsLog(const char *szModule, const char* szLogPath)
	:m_pFile(NULL)
    ,m_nCapacity(DEFAULT_MAX_LOG_CAPACITY)
	,m_nTotalSize(0)
	,m_nFlushFlag(1)
	,m_nCompress(1)
	,m_eLogLevel(LOGLEVEL_DEBUG)
	,m_hasMillilSecond(1)
	,m_CompressFolder("")
	, m_nStopFlag(0)
	, m_keepDays(DEFAULT_LOG_KEEPDAYS)
	, m_nSyncProc(1)
{
	CAutoLock::InitLock(m_Lock);

	m_module = szModule == NULL ? "" : szModule;
	m_path   = szLogPath == NULL ? "./log" : szLogPath;

	time_t curr_time = time(NULL);
	struct tm *pt = NULL;

	#ifdef WIN32
	pt = localtime(&curr_time);
	#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
	#endif

	char szDate[64] = {};
	snprintf(szDate, 64, "%04d-%02d-%02d",
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday);

	std::string strPath = m_path;
	strPath += "/";
	strPath += szDate;

	if (_access(m_path.c_str(), 0))
	{
		if (_mkdir(m_path.c_str()))
		{
			printf("create log path fail");
			return;
		}
	}

	if (_access(strPath.c_str(),0))
	{
		if (_mkdir(strPath.c_str()))
		{
			printf("create log path fail");
			return;
		}	
	}

	char szFile[128] = {};
	snprintf(szFile, 64, "%s_%04d-%02d-%02dT%02d-%02d-%02d.log",
		m_module.c_str(),
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec);

	strPath += "/";
	strPath += szFile;

	m_pFile = fopen(strPath.c_str(),"w");

	if (!m_pFile)
	{
		printf("create log file fail");
	}

	m_stCurDate = *pt;

	#ifdef WIN32
	unsigned nThreadID;
	m_hCompressThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, this, 0, &nThreadID);
	#else
	pthread_create(&m_hCompressThread, NULL, ThreadFunc, this);
	#endif

#ifdef WIN32
	unsigned procThreadID;
	m_hSyncProcThread = (HANDLE)_beginthreadex(NULL, 0, SyncProcThread, this, 0, &procThreadID);
#else
	pthread_create(&m_hSyncProcThread, NULL, SyncProcThread, this);
#endif

	m_atomicQueue = new atomic_queue::AtomicQueueB2<std::string, std::allocator<std::string>>(QUEUE_MAX_SIZE);
}

FmsLog::~FmsLog()
{
	CAutoLock::LOCK(m_Lock);

	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}

	CAutoLock::UNLOCK(m_Lock);
	CAutoLock::UnInitLock(m_Lock);

	m_nCompress = 0;

	#ifdef WIN32
	CloseHandle(m_hCompressThread);
	#else
	void* pRet = NULL;
	pthread_join(m_hCompressThread,&pRet);
	#endif

	m_nSyncProc = 0;
	m_procEvent.Reset();

#ifdef WIN32
	CloseHandle(m_hSyncProcThread);
#else
	void* pRet2 = NULL;
	pthread_join(m_hSyncProcThread, &pRet2);
#endif

	if (m_atomicQueue)
	{
		delete m_atomicQueue;
		m_atomicQueue = NULL;
	}
}

void FmsLog::flush()
{
	CAutoLock autoLock(m_Lock);

	if (m_pFile) fflush(m_pFile);
}

static std::string GetLevelName(EnumLogLevel eLevel)
{
	std::string pszLevel = "";

	switch (eLevel)//自动处理标准级别的
	{
	case LOGLEVEL_FATAL:
		pszLevel = "FATAL";
		break;

	case LOGLEVEL_ERROR:
		pszLevel = "ERROR";
		break;

	case LOGLEVEL_WARN:
		pszLevel = "WARN";
		break;

	case LOGLEVEL_INFO:
		pszLevel = "INFO";
		break;

	case LOGLEVEL_DEBUG:
		pszLevel = "DEBUG";
		break;

	case LOGLEVEL_TRACE:
		pszLevel = "TRACE";
		break;

	default: //由上层自定义
		break;
	}

	return pszLevel;
}

void FmsLog::print(EnumLogLevel level, const char *format, ...)
{
	if (level > m_eLogLevel)
	{
		return;
	}

	CAutoLock autoLock(m_Lock);
	if (!m_pFile)
	{
		return;
	}

	Backup();

	time_t curr_time = time(NULL);
	struct tm *pt = NULL;

	#ifdef WIN32
	pt = localtime(&curr_time);
	#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
	#endif

	char szInfo[256] = {};
	std::string pszLevel = GetLevelName(level);

	snprintf(szInfo, 256, "%04d-%02d-%02d %02d:%02d:%02d.%03d [%s] %-6s",
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec, GetMillisecond(),m_module.c_str(), pszLevel.c_str());

	va_list ap;
	va_start(ap, format);

	char* buffer = new char[LOG_MSG_BUF_SIZE];
	if (NULL == buffer)
	{
		return;
	}
	memset(buffer, 0, LOG_MSG_BUF_SIZE);

	vsnprintf(buffer, LOG_MSG_BUF_SIZE - 1, format, ap);
	buffer[LOG_MSG_BUF_SIZE - 1] = 0;

	m_nTotalSize += fprintf(m_pFile, "%s  %s\n", szInfo, buffer);

	va_end(ap);

	if (m_nFlushFlag) fflush(m_pFile);

	delete[] buffer;
	buffer = NULL;
}

void FmsLog::printstr(EnumLogLevel level, const char* str)
{
	if (level > m_eLogLevel)
	{
		return;
	}

	time_t curr_time = time(NULL);
	struct tm* pt = NULL;

	#ifdef WIN32
	pt = localtime(&curr_time);
	#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
	#endif

	char szInfo[256] = {};
	std::string pszLevel = GetLevelName(level);

	snprintf(szInfo, 256, "%04d-%02d-%02d %02d:%02d:%02d.%03d [%s] %-6s",
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec, GetMillisecond(), m_module.c_str(), pszLevel.c_str());


	std::string buffer = szInfo;
	buffer += " ";
	buffer += str;
	buffer += "\n";

	do 
	{
		//异步处理
		if (m_nSyncProc)
		{		
			PushBack(buffer);

			//设置消息处理事件
			m_procEvent.Set();
		}
		else
		{
			print(buffer.c_str());
		}

	} while (false);
}

void FmsLog::print(EnumLogLevel level, int ID, const char *format, ...)
{
	if (level > m_eLogLevel)
	{
		return;
	}

	CAutoLock autoLock(m_Lock);

	if (!m_pFile)
	{
		return;
	}

	Backup();

	time_t curr_time = time(NULL);
	struct tm *pt = NULL;

	#ifdef WIN32
	pt = localtime(&curr_time);
	#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
	#endif

	char szInfo[256] = {};
	std::string pszLevel = GetLevelName(level);

	snprintf(szInfo, 256, "%04d-%02d-%02d %02d:%02d:%02d.%03d [%s] %-6s [%08X]",
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec, GetMillisecond(), m_module.c_str(), pszLevel.c_str(),ID);

	va_list ap;
	va_start(ap, format);

	char* buffer = new char[LOG_MSG_BUF_SIZE];
	if (NULL == buffer)
	{
		return;
	}
	memset(buffer, 0, LOG_MSG_BUF_SIZE);

	vsnprintf(buffer, LOG_MSG_BUF_SIZE - 1, format, ap);
	buffer[LOG_MSG_BUF_SIZE - 1] = 0;

	m_nTotalSize += fprintf(m_pFile, "%s  %s\n", szInfo, buffer);

	va_end(ap);

	if (m_nFlushFlag) fflush(m_pFile);

	delete[] buffer;
	buffer = NULL;
}

void FmsLog::print(EnumLogLevel level, unsigned int qualifier, unsigned int ID, const char *format, ...)
{
	if (level > m_eLogLevel)
	{
		return;
	}

	CAutoLock autoLock(m_Lock);

	if (!m_pFile)
	{
		return;
	}

	Backup();

	time_t curr_time = time(NULL);
	struct tm *pt = NULL;

	#ifdef WIN32
	pt = localtime(&curr_time);
	#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
	#endif

	char szInfo[256] = {};
	std::string pszLevel = GetLevelName(level);

	snprintf(szInfo, 256, "%04d-%02d-%02d %02d:%02d:%02d.%03d [%s] %-6s <%08X>[%08X]",
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec, GetMillisecond(), m_module.c_str(), pszLevel.c_str(), qualifier,ID);

	va_list ap;
	va_start(ap, format);

	char* buffer = new char[LOG_MSG_BUF_SIZE];
	if (NULL == buffer)
	{
		return;
	}
	memset(buffer, 0, LOG_MSG_BUF_SIZE);

	vsnprintf(buffer, LOG_MSG_BUF_SIZE - 1, format, ap);
	buffer[LOG_MSG_BUF_SIZE - 1] = 0;

	m_nTotalSize += fprintf(m_pFile, "%s  %s\n", szInfo, buffer);

	va_end(ap);

	if (m_nFlushFlag) fflush(m_pFile);

	delete[] buffer;
	buffer = NULL;
}

void FmsLog::printnt(EnumLogLevel level, const char *format, ...)
{
	if (level > m_eLogLevel)
	{
		return;
	}

	CAutoLock autoLock(m_Lock);

	if (!m_pFile)
	{
		return;
	}

	Backup();

	char szInfo[128] = {};
	std::string pszLevel = GetLevelName(level);

	snprintf(szInfo, 128, "[%s] %-6s", m_module.c_str(), pszLevel.c_str());

	va_list ap;
	va_start(ap, format);

	char* buffer = new char[LOG_MSG_BUF_SIZE];
	if (NULL == buffer)
	{
		return;
	}
	memset(buffer, 0, LOG_MSG_BUF_SIZE);

	vsnprintf(buffer, LOG_MSG_BUF_SIZE - 1, format, ap);
	buffer[LOG_MSG_BUF_SIZE - 1] = 0;

	m_nTotalSize += fprintf(m_pFile, "%s  %s\n", szInfo, buffer);

	va_end(ap);

	if (m_nFlushFlag) fflush(m_pFile);

	delete[] buffer;
	buffer = NULL;
}

void FmsLog::printb(EnumLogLevel level, const char *title,const unsigned char *buf, int len)
{
	if (!buf || len <= 0)
	{
		return;
	}

	if (level > m_eLogLevel)
	{
		return;
	}

	CAutoLock autoLock(m_Lock);

	if (!m_pFile)
	{
		return;
	}

	Backup();

	time_t curr_time = time(NULL);
	struct tm *pt = NULL;

	#ifdef WIN32
	pt = localtime(&curr_time);
	#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
	#endif

	char szInfo[256] = {};
	std::string pszLevel = GetLevelName(level);

	snprintf(szInfo, 256, "%04d-%02d-%02d %02d:%02d:%02d.%03d [%s] %-6s %s",
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec, GetMillisecond(), m_module.c_str(), pszLevel.c_str(), title);

	m_nTotalSize += fprintf(m_pFile, "%s\n", szInfo);

	printb(buf, len);
}

void FmsLog::printb(EnumLogLevel level, unsigned int ID, const char *title, const unsigned char *buf, int len)
{
	if (!buf || len <= 0)
	{
		return;
	}

	if (level > m_eLogLevel)
	{
		return;
	}

	CAutoLock autoLock(m_Lock);

	if (!m_pFile)
	{
		return;
	}

	Backup();

	time_t curr_time = time(NULL);
	struct tm *pt = NULL;

	#ifdef WIN32
	pt = localtime(&curr_time);
	#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
	#endif

	char szInfo[256] = {};
	std::string pszLevel = GetLevelName(level);

	snprintf(szInfo, 256, "%04d-%02d-%02d %02d:%02d:%02d.%03d [%s] %-6s [%08X] %s",
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec, GetMillisecond(), m_module.c_str(), pszLevel.c_str(),ID,title);

	m_nTotalSize += fprintf(m_pFile, "%s\n", szInfo);

	printb(buf, len);
}

void FmsLog::printbnt(EnumLogLevel level, const unsigned char *buf, int len)
{
	if (!buf || len <= 0)
	{
		return;
	}

	if (level > m_eLogLevel)
	{
		return;
	}

	CAutoLock autoLock(m_Lock);

	if (!m_pFile)
	{
		return;
	}

	Backup();

	char szInfo[256] = {};
	std::string pszLevel = GetLevelName(level);

	snprintf(szInfo, 256, "[%s] %-6s", m_module.c_str(), pszLevel.c_str());

	m_nTotalSize += fprintf(m_pFile, "%s\n", szInfo);

	printb(buf, len);
}

void FmsLog::vprint(EnumLogLevel level, const char *format, va_list argptr)
{
	if (level > m_eLogLevel)
	{
		return;
	}

	CAutoLock autoLock(m_Lock);
	if (!m_pFile)
	{
		return;
	}

	Backup();

	time_t curr_time = time(NULL);
	struct tm *pt = NULL;

	#ifdef WIN32
	pt = localtime(&curr_time);
	#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
	#endif

	char szInfo[256] = {};
	std::string pszLevel = GetLevelName(level);

	snprintf(szInfo, 256, "%04d-%02d-%02d %02d:%02d:%02d.%03d [%s] %-6s",
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec, GetMillisecond(), m_module.c_str(), pszLevel.c_str());

	char* buffer = new char[LOG_MSG_BUF_SIZE];
	if (NULL == buffer)
	{
		return;
	}
	memset(buffer, 0, LOG_MSG_BUF_SIZE);

	vsnprintf(buffer, LOG_MSG_BUF_SIZE - 1, format, argptr);
	buffer[LOG_MSG_BUF_SIZE - 1] = 0;

	m_nTotalSize += fprintf(m_pFile, "%s  %s\n", szInfo, buffer);

	if (m_nFlushFlag) fflush(m_pFile);

	delete[] buffer;
	buffer = NULL;
}

void FmsLog::vprint(EnumLogLevel level, int ID, const char *format, va_list argptr)
{
	if (level > m_eLogLevel)
	{
		return;
	}

	CAutoLock autoLock(m_Lock);

	if (!m_pFile)
	{
		return;
	}

	Backup();

	time_t curr_time = time(NULL);
	struct tm *pt = NULL;

	#ifdef WIN32
	pt = localtime(&curr_time);
	#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
	#endif

	char szInfo[256] = {};
	std::string pszLevel = GetLevelName(level);

	snprintf(szInfo, 256, "%04d-%02d-%02d %02d:%02d:%02d.%03d [%s] %-6s [%08X]",
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec, GetMillisecond(), m_module.c_str(), pszLevel.c_str(),ID);

	char* buffer = new char[LOG_MSG_BUF_SIZE];
	if (NULL == buffer)
	{
		return;
	}
	memset(buffer, 0, LOG_MSG_BUF_SIZE);

	vsnprintf(buffer, LOG_MSG_BUF_SIZE - 1, format, argptr);
	buffer[LOG_MSG_BUF_SIZE - 1] = 0;

	m_nTotalSize += fprintf(m_pFile, "%s  %s\n", szInfo, buffer);

	if (m_nFlushFlag) fflush(m_pFile);

	delete[] buffer;
	buffer = NULL;
}

void FmsLog::vprint(EnumLogLevel level, unsigned int guid, unsigned int ID, const char *format, va_list argptr)
{
	if (level > m_eLogLevel)
	{
		return;
	}

	CAutoLock autoLock(m_Lock);

	if (!m_pFile)
	{
		return;
	}

	Backup();

	time_t curr_time = time(NULL);
	struct tm *pt = NULL;

	#ifdef WIN32
	pt = localtime(&curr_time);
	#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
	#endif

	char szInfo[256] = {};
	std::string pszLevel = GetLevelName(level);

	snprintf(szInfo, 256, "%04d-%02d-%02d %02d:%02d:%02d.%03d [%s] %-6s <%08X>[%08X]",
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec, GetMillisecond(), m_module.c_str(), pszLevel.c_str(), guid,ID);

	char* buffer = new char[LOG_MSG_BUF_SIZE];
	if (NULL == buffer)
	{
		return;
	}
	memset(buffer, 0, LOG_MSG_BUF_SIZE);

	vsnprintf(buffer, LOG_MSG_BUF_SIZE - 1, format, argptr);
	buffer[LOG_MSG_BUF_SIZE - 1] = 0;

	m_nTotalSize += fprintf(m_pFile, "%s  %s\n", szInfo, buffer);

	if (m_nFlushFlag) fflush(m_pFile);

	delete[] buffer;
	buffer = NULL;
}

void FmsLog::printb(const unsigned char *buf, int len)
{
	char msg[512] = { 0 };
	char *pStr = msg;
	int i = 0;

	for (i = 0; i < len; i++)
	{
		if (i % 32 == 0)
		{
			pStr += sprintf(pStr, "%08x: ", buf[i]);
		}

		pStr += sprintf(pStr, "%02x ", buf[i]);
		if (((i + 1) % 32) == 0)
		{
			strcat(msg, "\n");
			m_nTotalSize += fprintf(m_pFile, "%s", msg);
			strcpy(msg, "");
			pStr = msg;
		}
	}

	if (i % 32)
	{
		strcat(msg, "\n");
		m_nTotalSize += fprintf(m_pFile, "%s\n", msg);
	}
	
	if (m_nFlushFlag) fflush(m_pFile);
}

void FmsLog::Backup()
{
	time_t curr_time = time(NULL);
	struct tm *pt = NULL;
	#ifdef WIN32
	pt = localtime(&curr_time);
	#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
	#endif
	
	if ((m_nTotalSize && (m_stCurDate.tm_year != pt->tm_year
		|| m_stCurDate.tm_mon != pt->tm_mon
		|| m_stCurDate.tm_mday != pt->tm_mday)) || ((m_nTotalSize / 1024) >= (DEFAULT_FILE_CAPACITY > m_nCapacity ? m_nCapacity : DEFAULT_FILE_CAPACITY)))
	{
		char szDate[64] = {};
		snprintf(szDate, 64, "%04d-%02d-%02d",
			pt->tm_year + 1900,
			pt->tm_mon + 1, pt->tm_mday);

		std::string strPath = m_path;
		strPath += "/";
		strPath += szDate;

		if (_access(strPath.c_str(), 0))
		{
			if (_mkdir(strPath.c_str()))
			{
				printf("create log path fail");
				return;
			}
		}

		char szFile[128] = {};
		snprintf(szFile, 64, "%s_%04d-%02d-%02dT%02d-%02d-%02d.log",
			m_module.c_str(),
			pt->tm_year + 1900,
			pt->tm_mon + 1, pt->tm_mday,
			pt->tm_hour, pt->tm_min, pt->tm_sec);

		strPath += "/";
		strPath += szFile;

		if (m_pFile)
		{
			fclose(m_pFile);
		}
		
		m_pFile = fopen(strPath.c_str(),"w");
		m_nTotalSize = 0;

		if (m_stCurDate.tm_year != pt->tm_year
			|| m_stCurDate.tm_mon != pt->tm_mon
			|| m_stCurDate.tm_mday != pt->tm_mday)
		{
			snprintf(szDate, 64, "%04d-%02d-%02d",
				m_stCurDate.tm_year + 1900,
				m_stCurDate.tm_mon + 1, m_stCurDate.tm_mday);
			m_CompressFolder = szDate;
		}

		m_stCurDate = *pt;
	}
}

int FmsLog::GetMillisecond()
{
	if (m_hasMillilSecond)
	{
		#ifdef WIN32
		_timeb time_with_millisecond;
		_ftime(&time_with_millisecond);
		return time_with_millisecond.millitm;
		#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return tv.tv_usec / 1000;
		#endif
	}
	else
	{
		return 0;
	}
}

#ifdef WIN32
unsigned __stdcall FmsLog::ThreadFunc(void* arg)
#else
void* FmsLog::ThreadFunc(void* arg)
#endif
{
	FmsLog *pLog = (FmsLog*)arg;

	pLog->Compress();

	return 0;
}

#ifdef WIN32
unsigned __stdcall FmsLog::SyncProcThread(void* arg)
#else
void* FmsLog::SyncProcThread(void* arg)
#endif
{
	FmsLog* pLog = (FmsLog*)arg;

	pLog->SyncProc();

	return 0;
}

void FmsLog::MySleep(int nMsec)
{
	#ifdef WIN32
	Sleep(nMsec);
	#else
	usleep(nMsec * 1000);
	#endif
}

int FmsLog::removeDir(std::string dirPath)
#ifdef WIN32
{
	struct _finddata_t fb;   //find the storage structure of the same properties file.
	std::string path;
	intptr_t    handle;
	int   noFile;            // the tag for the system's hidden files

	noFile = 0;
	handle = 0;

	path = dirPath + "/*";
	handle = _findfirst(path.c_str(), &fb);

	if (handle > -1)
	{
		while (0 == _findnext(handle, &fb))
		{
			noFile = strcmp(fb.name, "..");

			if (0 != noFile)
			{
				path.clear();
				path = dirPath + "/" + fb.name;
				if (fb.attrib == 16)
				{
					removeDir(path);
				}
				else
				{
					remove(path.c_str());
				}
			}
		}

		_findclose(handle);
	}

	_rmdir(dirPath.c_str());

	return 0;
}
#else
{
	if (dirPath.empty())
	{
		return -1;
	}

	DIR *hDir = opendir(dirPath.c_str());
	if (!hDir)
	{
		return -1;
	}

	struct dirent *pstInfo;
	char TempDir[1024] = { 0 };
	struct stat FileBuf;

	while ((pstInfo = readdir(hDir)) != NULL)
	{
		if (strcmp(pstInfo->d_name, ".") && strcmp(pstInfo->d_name, ".."))
		{
			snprintf(TempDir, 1023, "%s/%s", dirPath.c_str(), pstInfo->d_name);

			if (stat(TempDir, &FileBuf) == 0)
			{
				//判断文件是否是目录文件
				if (S_ISDIR(FileBuf.st_mode))
				{
					removeDir(TempDir);
				}
				//判断文件是否是普通文件
				else if (S_ISREG(FileBuf.st_mode))
				{
					remove(TempDir);
				}
			}
			else
			{
				return -1;
			}
		}
	}

	closedir(hDir);
	rmdir(dirPath.c_str());

	return 0;
}
#endif

std::string FmsLog::GetZipCmdLine(const std::string& strFromFileFullName, const std::string& strToFileFullNameWithoutZipExt)
{
	char cmdLine[256] = { 0 };

#ifdef WIN32
	sprintf_s(cmdLine, sizeof(cmdLine), "7z.exe a \"%s.zip\" \"%s\"", strToFileFullNameWithoutZipExt.c_str(), strFromFileFullName.c_str());
#endif // WIN32

	return cmdLine;
}

std::string FmsLog::GetTarCmdLine(const std::string& strFromFileFullName, const std::string& strFolder,const std::string& strRelativePath)
{
	char cmdLine[256] = { 0 };

#ifndef WIN32
	snprintf(cmdLine, sizeof(cmdLine), "tar -cvzf %s.tar.gz -C %s %s", strFromFileFullName.c_str(), strRelativePath.c_str(), strFolder.c_str());
#endif // WIN32

	return cmdLine;
}

void FmsLog::CompressFile(const std::string& strFileFullName, const std::string& strBackupFullPath)
{
	#ifdef WIN32
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	ZeroMemory(&pi, sizeof(pi));

	char toFileNameWithoutZipExt[256] = { 0 };

	sprintf_s(toFileNameWithoutZipExt, sizeof(toFileNameWithoutZipExt), "%s", strBackupFullPath.c_str());

	std::string cmdLine = GetZipCmdLine(strFileFullName, toFileNameWithoutZipExt);
	printf_s("cmdline = %s\n", cmdLine.c_str());

	if (!CreateProcess(NULL, (char*)cmdLine.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		return;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD dwExitCode = -1;

	if (!GetExitCodeProcess(pi.hProcess, &dwExitCode))
	{
		dwExitCode = -2;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	if (0 == dwExitCode)
	{
		removeDir(strFileFullName.c_str());
	}
	else
	{
		printf("compress fail");
	}
	#endif
}

void FmsLog::CompressFile(const std::string& strRelativePath, std::string& strFileFullName, const std::string& strBackupFullPath)
{
#ifndef WIN32
	char toFileNameWithoutZipExt[256] = { 0 };

	snprintf(toFileNameWithoutZipExt, sizeof(toFileNameWithoutZipExt), "%s", strBackupFullPath.c_str());

	std::string cmdLine = GetTarCmdLine(toFileNameWithoutZipExt, strFileFullName, strRelativePath);
	printf("cmdline = %s\n", cmdLine.c_str());

	if (system(cmdLine.c_str()) != 0)
	{
		return;
	}

	removeDir(strBackupFullPath.c_str());
#endif // !WIN32
}

void FmsLog::Compress()
{
	while (m_nCompress)
	{
		if (!m_CompressFolder.empty())
		{
			std::string strCompressPath = m_path;
			strCompressPath += "/";
			strCompressPath += m_CompressFolder;

			#ifdef WIN32
			//CompressFile(strCompressPath, strCompressPath);
			#else
			CompressFile(m_path, m_CompressFolder, strCompressPath);
			#endif

			m_CompressFolder = "";
		}

		//删除超过容量的文件
		DelFile();

		MySleep(1000);
	}
}

void FmsLog::printstrExt(EnumLogLevel level, const char* str)
{
	if (level > m_eLogLevel)
	{
		return;
	}

	//日志超过最大缓存则不输出
	//if (m_nStopFlag)
	//{
	//	return;
	//}

	time_t curr_time = time(NULL);
	struct tm* pt = NULL;

	#ifdef WIN32
	pt = localtime(&curr_time);
	#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
	#endif

	char szInfo[256] = {};
	std::string pszLevel = GetLevelName(level);

	snprintf(szInfo, 256, "%04d-%02d-%02d %02d:%02d:%02d.%03d %-6s",
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec, GetMillisecond(), pszLevel.c_str());

	std::string buffer = szInfo;
	buffer += " ";
	buffer += str;
	buffer += "\n";

	do
	{
		//异步处理
		if (m_nSyncProc)
		{
			PushBack(buffer);

			//设置消息处理事件
			m_procEvent.Set();
		}
		else
		{
			print(buffer.c_str());
		}

	} while (false);
}

void FmsLog::print(const char* str)
{
	if (NULL == str)
	{
		return;
	}

	if (!m_pFile)
	{
		return;
	}

	CAutoLock autoLock(m_Lock);

	Backup();

	m_nTotalSize += fprintf(m_pFile, "%s", str);

	if (m_nFlushFlag) fflush(m_pFile);
}

void __service_log__::FmsLog::DelFile()
{
	//检测日志文件是否超过大小
	std::map<int64_t, std::string> mapFileName;

	m_nStopFlag = ((GetFolderSize(m_path, mapFileName) / 1024) >= m_nCapacity) ? true : false;

	if (m_nStopFlag)
	{
		//删除日志
		if (mapFileName.size())
		{
			remove(mapFileName.begin()->second.c_str());
		}
	}
}

void FmsLog::PopFront(std::list<std::string>& list)
{
#if 0
	std::unique_lock<std::mutex> lock(m_mutex);

	list = m_procQueue;
	m_procQueue.clear();
#else
	if (NULL == m_atomicQueue)
	{
		return;
	}

	int num = m_atomicQueue->was_size();
	num = num > 500 ? 500 : num;

	for (int i = 0; i < num; ++i)
	{
		list.push_back(m_atomicQueue->pop());
	}

#endif
}

bool FmsLog::PushBack(std::string& str)
{
#if 0
	if (m_procQueue.size() >= QUEUE_MAX_SIZE)
	{
		return false;
	}

	std::unique_lock<std::mutex> lock(m_mutex);

	m_procQueue.push_back(str);

	return true;
#else
	if (NULL == m_atomicQueue)
	{
		return false;
	}

	if (m_atomicQueue->was_size() >= QUEUE_MAX_SIZE - 1)
	{
		return false;
	}

	m_atomicQueue->push(str);

	return true;
#endif
}

void FmsLog::SyncProc()
{
	unsigned int tick = 0;
	unsigned int pagesize = 101;

	while (m_nSyncProc)
	{
		//批量取出队列中所有数据
		std::list<std::string> list;
		PopFront(list);

		if (0 == list.size())
		{
			m_procEvent.Wait();
			m_procEvent.Reset();
			continue;
		}

		//分页上报
		tick = 0;
		std::string msg;

		auto timeNow = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
		int64_t time_ms = timeNow.count();

		for (std::list<std::string>::iterator it = list.begin(); it != list.end();)
		{
			++tick;

			if (0 == (tick / pagesize))
			{
				msg += *it;
				++it;
			}
			else
			{
				//中间分页
				// 
				print(msg.c_str());
	
				tick = 0;
				msg.clear();
			}
		}

		//最后一页
		if (!msg.empty())
		{
			print(msg.c_str());
		}

		auto timeNow2 = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
		time_ms = timeNow2.count() - time_ms;

		msg = "log tick num = " + std::to_string(list.size()) + " time_ms = " + std::to_string(time_ms) + "\n";
		print(msg.c_str());

		#ifdef WIN32
		sleep_ms(10);
		#else
		sleep_ms(100);
		#endif
	}
}

uint64_t FmsLog::GetFolderSize(std::string path, std::map<int64_t, std::string>& mapFileName)//递归
{
#ifdef WIN32
	//文件句柄
	intptr_t hFile = 0;
	std::vector<std::string> files;

	//文件信息
	struct _finddata_t fileinfo;
	std::string p;
	uint64_t filesize = 0;
	int i = 0;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))//判断是否是文件夹 
			{
				//如果是目录,递归查找并累积size
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) 
				{
					uint64_t dirSize = GetFolderSize(p.assign(path).append("\\").append(fileinfo.name), mapFileName);

					if (!dirSize)
					{
						_rmdir(p.assign(path).append("\\").append(fileinfo.name).c_str());
					}

					filesize += dirSize;
				}
			}
			else
			{
				//检测时间是否过期
				time_t curr_time = time(NULL);
				struct tm* pt = NULL;
				pt = localtime(&curr_time);

				//删除过期日志
				if ((curr_time - fileinfo.time_create) >= ((int64_t)m_keepDays * 24 * 3600))
				{
					remove((path + "\\" + fileinfo.name).c_str());
				}
				else
				{
					//累加size
					filesize += fileinfo.size;
					mapFileName.insert(std::make_pair(fileinfo.time_create, (path + "\\" + fileinfo.name)));
				}
			}


		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}

	return filesize;

#else
	DIR* dp;
	struct dirent* entry;
	struct stat statbuf;
	uint64_t totalSize = 0;

	if ((dp = opendir(path.c_str())) == NULL)
	{
		fprintf(stderr, "Cannot open dir: %s\n", path.c_str());
		return -1; //可能是个文件，或者目录不存在
	}

	//先加上自身目录的大小
	lstat(path.c_str(), &statbuf);
	totalSize += statbuf.st_size;

	while ((entry = readdir(dp)) != NULL)
	{
		char subdir[256];
		sprintf(subdir, "%s/%s", path.c_str(), entry->d_name);
		lstat(subdir, &statbuf);

		if (S_ISDIR(statbuf.st_mode))
		{
			if (strcmp(".", entry->d_name) == 0 ||
				strcmp("..", entry->d_name) == 0)
			{
				continue;
			}

			uint64_t subDirSize = GetFolderSize(subdir, mapFileName);

			if (!subDirSize)
			{
				rmdir(subdir);
			}

			totalSize += subDirSize;
		}
		else
		{
			//检测时间是否过期
			time_t curr_time = time(NULL);
			struct tm* pt = NULL;
			pt = localtime(&curr_time);

			//删除过期日志
			if ((curr_time - statbuf.st_ctime) >= ((int64_t)m_keepDays * 24 * 3600))
			{
				remove(subdir);
			}
			else
			{
				totalSize += statbuf.st_size;
				mapFileName.insert(std::make_pair(statbuf.st_ctime, subdir));
			}
		}
	}

	closedir(dp);
	return totalSize;
#endif // WIN32
}
