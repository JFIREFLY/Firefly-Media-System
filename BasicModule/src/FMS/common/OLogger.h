#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <string>
#include <mutex>

namespace __service_log__
{
	class OLogger
	{
	public:
		typedef enum _tagLogLevel {
			LL_OFF = 0,
			LL_FATAL,
			LL_ERROR,
			LL_WARN,
			LL_INFO,
			LL_DEBUG,
			LL_TRACE,
			LL_ALL
		} LogLevel;

	public:
		OLogger();
		virtual ~OLogger();

		bool Init(std::string modulename, std::string filename, LogLevel level);

		void Fatal(const char* fmt, ...);
		void Error(const char* fmt, ...);
		void Warn(const char* fmt, ...);
		void Info(const char* fmt, ...);
		void Debug(const char* fmt, ...);
		void Trace(const char* fmt, ...);

		void Fatal(const char* fmt, va_list& ap);
		void Error(const char* fmt, va_list& ap);
		void Warn(const char* fmt, va_list& ap);
		void Info(const char* fmt, va_list& ap);
		void Debug(const char* fmt, va_list& ap);
		void Trace(const char* fmt, va_list& ap);

		void Print(int level, const char* str);

		int GetLogLevel();
		void SetLogLevel(int level);

		void SetMaxCapacity(unsigned long nCapacity);
		void SetKeepDays(int days);
	private:
		std::mutex	m_lockLogger;
		LogLevel m_level;
		void* m_ptrLog;

	};
}
#endif // _LOGGER_H_
