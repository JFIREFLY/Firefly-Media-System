#pragma warning(disable:4996)
#include "CommonTools.h"
#include "Public.h"

#ifdef WIN32
#include <windows.h>
#include <strsafe.h>
#include <netlistmgr.h>
#include <atlbase.h>
#include <wincon.h>
#include <Nb30.h>
#include <sys/timeb.h>
//#include <WinSock2.h>
#include <Iphlpapi.h>
#include <IPTypes.h>
//#include <WS2tcpip.h>
//#include <stdlib.h>
//#include <atlconv.h>
//#include <atlstr.h>
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"netapi32.lib")
#else 
#include <uuid/uuid.h>
#include <sstream>
#include <iomanip>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>  
#include <netdb.h>  
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/statfs.h>
#endif
#include<time.h>
#include <memory.h>
#include<string>
#include <sstream>
#include <chrono>   
#include <vector>
#include <fstream>
#include <iostream>

#include<openssl/aes.h>
#include<openssl/rsa.h>
#include<openssl/pem.h>
#include<openssl/err.h>


using namespace std;
using namespace chrono;

#define GUID_LEN 64 


std::string CommonTools::GetGUID()
{
#ifdef WIN32
	char buffer[GUID_LEN] = { 0 };
	GUID guid;

	if (CoCreateGuid(&guid))
	{
		return "";
	}
	_snprintf(buffer, sizeof(buffer),
		"%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2],
		guid.Data4[3], guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	return buffer;
#else
	uuid_t uu;
	char buffer[GUID_LEN] = { 0 };
	uuid_generate(uu);

	sprintf(buffer,
	    "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		uu[1], uu[2], uu[3],
		uu[2], uu[5], uu[6],
		uu[7], uu[8], uu[9],
		uu[10], uu[11], uu[12],
		uu[13], uu[14], uu[15], uu[16]);

	return buffer;
#endif

}

unsigned short CommonTools::GetRandU16()
{
	unsigned short dst_u16 = 0;

	srand((unsigned int)GetLocalTimeStamp_ns());
	dst_u16 = rand() % 0xffff;

	dst_u16 = dst_u16 + (unsigned short)GetLocalTimeStamp_ns();

	return dst_u16;
}

std::string CommonTools::GetRandStr(int len)
{
	if (len > 20)
	{
		return "";
	}

	char str[20] = {0};
	srand((unsigned int)time(NULL));

	int i;
	for (i = 0; i < len; ++i)
	{
		switch ((rand() % 3))
		{
		case 1:
			str[i] = 'A' + rand() % 26;
			break;
		case 2:
			str[i] = 'a' + rand() % 26;
			break;
		default:
			str[i] = '0' + rand() % 10;
			break;
		}
	}
	str[++i] = '\0';

	std::string dst_str = str;

	return dst_str;
}

std::string CommonTools::GetGUID2()
{
	char buffer[GUID_LEN] = { 0 };

	int64_t timeStamp = GetLocalTimeStamp_ms();

	sprintf(buffer, "%013llu%s", timeStamp, GetRandStr(7).c_str());

	return buffer;
}

std::string CommonTools::GetGUID3()
{
#ifdef WIN32
	char buffer[GUID_LEN] = { 0 };
	GUID guid;

	if (CoCreateGuid(&guid))
	{
		return "";
	}
	_snprintf(buffer, sizeof(buffer),
		"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2],
		guid.Data4[3], guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	return buffer;
#else
	uuid_t uu;
	char buffer[GUID_LEN] = { 0 };
	uuid_generate(uu);

	sprintf(buffer,
		"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		uu[1], uu[2], uu[3],
		uu[2], uu[5], uu[6],
		uu[7], uu[8], uu[9],
		uu[10], uu[11], uu[12],
		uu[13], uu[14], uu[15], uu[16]);

	return buffer;
#endif

}

Times CommonTools::stamp_to_standard(unsigned int stampTime)
{
	time_t tick = (time_t)stampTime;
	struct tm tm;
	char s[100];
	Times standard;

	tm = *localtime(&tick);
	strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);

	standard.Year = atoi(s);
	standard.Mon = atoi(s + 5);
	standard.Day = atoi(s + 8);
	standard.Hour = atoi(s + 11);
	standard.Min = atoi(s + 14);
	standard.Second = atoi(s + 17);

	return standard;
}

int CommonTools::standard_to_stamp(const char* str_time)
{
	struct tm stm;
	memset(&stm, 0, sizeof(stm));
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

	sscanf(str_time, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
	stm.tm_year = year - 1900;
	stm.tm_mon = month - 1;
	stm.tm_mday = day;
	stm.tm_hour = hour;
	stm.tm_min = minute;
	stm.tm_sec = second;
	stm.tm_isdst = 0;

	return (int)mktime(&stm);
}

int Daysto(int year, int mon)
{
	int days, end;

	static const short lmos[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };
	static const short mos[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

	if (0 < year) {
		end = year - 1;
		days = end / 4 - end / 100 + (1900 + end) / 400 - 1900 / 400;
	}
	else if (year <= -4) {
		days = 1 + (4 - year) / 4;
	}
	else {
		days = 0;
	}
	return days + (year & 03 || year == 0 ? mos : lmos)[mon];
}

long long tmtoT(tm* t)
{
	long double dsecs;
	int mon, year, ymon;
	long long secs;

	ymon = t->tm_mon / 12;
	mon = t->tm_mon - ymon * 12;
	if (mon < 0)
		mon += 12, --ymon;
	if (ymon < 0 && t->tm_year < 70
		|| 0 < ymon && 2147483647 - ymon < t->tm_year)
		return (long long)0;

	year = t->tm_year + ymon;
	dsecs = 86400.0 * (Daysto(year, mon) - 1)
		+ 31536000.0 * year + 86400.0 * t->tm_mday;
	dsecs += 3600.0 * t->tm_hour + 60.0 * t->tm_min + (double)t->tm_sec;
	if (dsecs < 0.0)
		return (long long)(0);
	secs = (long long)dsecs - ((long long)(70 * 365 + 17) * 86400);

	return secs;
}

long long CommonTools::standard_to_stamp_ext(const char* str_time)
{
	struct tm stm;
	memset(&stm, 0, sizeof(stm));
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

	sscanf(str_time, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
	stm.tm_year = year - 1920;
	stm.tm_mon = month - 1;
	stm.tm_mday = day;
	stm.tm_hour = hour;
	stm.tm_min = minute;
	stm.tm_sec = second;
	stm.tm_isdst = 0;

	return tmtoT(&stm);
}


long long CommonTools::standard_to_stamp2(const char* str_time)
{
	struct tm stm;
	memset(&stm, 0, sizeof(stm));
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
	std::string buf = str_time;

	year = atoi(buf.substr(0, 4).c_str());
	month = atoi(buf.substr(4, 2).c_str());
	day = atoi(buf.substr(6, 2).c_str());

	stm.tm_year = year - 1920;
	stm.tm_mon = month - 1;
	stm.tm_mday = day;
	stm.tm_hour = hour;
	stm.tm_min = minute;
	stm.tm_sec = second;
	stm.tm_isdst = 0;

	return tmtoT(&stm);
}

long long CommonTools::standard_to_stamp3(const char* str_time)
{
	if (NULL == str_time)
	{
		return 0;
	}

	struct tm stm;
	memset(&stm, 0, sizeof(stm));
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

	std::string buf = str_time;

	if (14 != buf.size())
	{
		return 0;
	}

	year = atoi(buf.substr(0, 4).c_str());
	month = atoi(buf.substr(4, 2).c_str());
	day = atoi(buf.substr(6, 2).c_str());
	hour = atoi(buf.substr(8, 2).c_str());
	minute = atoi(buf.substr(10, 2).c_str());
	second = atoi(buf.substr(12, 2).c_str());

	stm.tm_year = year - 1900;
	stm.tm_mon = month - 1;
	stm.tm_mday = day;
	stm.tm_hour = hour;
	stm.tm_min = minute;
	stm.tm_sec = second;
	stm.tm_isdst = 0;

	return tmtoT(&stm);
}

long long CommonTools::standard_to_stamp4(const char* str_time)
{
	if (NULL == str_time)
	{
		return 0;
	}

	struct tm stm;
	memset(&stm, 0, sizeof(stm));
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

	sscanf(str_time, "%d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
	stm.tm_year = year - 1900;
	stm.tm_mon = month - 1;
	stm.tm_mday = day;
	stm.tm_hour = hour;
	stm.tm_min = minute;
	stm.tm_sec = second;
	stm.tm_isdst = 0;

	return tmtoT(&stm);
}

long long CommonTools::standard_to_stamp5(const char* str_time)
{
	struct tm stm;
	memset(&stm, 0, sizeof(stm));
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
	std::string buf = str_time;

	year = atoi(buf.substr(0, 4).c_str());
	month = atoi(buf.substr(4, 2).c_str());
	day = atoi(buf.substr(6, 2).c_str());

	stm.tm_year = year - 1900;
	stm.tm_mon = month - 1;
	stm.tm_mday = day;
	stm.tm_hour = hour;
	stm.tm_min = minute;
	stm.tm_sec = second;
	stm.tm_isdst = 0;

	return tmtoT(&stm);
}

long long self_mktime(unsigned int year, unsigned int mon,
	unsigned int day, unsigned int hour,
	unsigned int min, unsigned int sec)
{
	if (0 >= (int)(mon -= 2)) {    /* 1..12 -> 11,12,1..10 */
		mon += 12;      /* Puts Feb last since it has leap day */
		year -= 1;
	}

	return (((
		(long long)(year / 4 - year / 100 + year / 400 + 367 * mon / 12 + day) +
		year * 365 - 719499
		) * 24 + hour /* now have hours */
		) * 60 + min /* now have minutes */
		) * 60 + sec; /* finally seconds */
}

long long CommonTools::standard_to_stamp6(const char* str_time)
{
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
	std::string buf = str_time;

	year = atoi(buf.substr(0, 4).c_str());
	month = atoi(buf.substr(4, 2).c_str());
	day = atoi(buf.substr(6, 2).c_str());

	return self_mktime(year, month, day, hour, minute, second);
}

void CommonTools::StringToDatetime(std::string str, tm& tm_)
{
	char *cha = (char*)str.data();                                                
	int year=0, month=0, day=0, hour=0, minute=0, second=0;
	sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
	tm_.tm_year = year - 1900;               
	tm_.tm_mon = month - 1;                    
	tm_.tm_mday = day;                       
	tm_.tm_hour = hour;                      
	tm_.tm_min = minute;                    
	tm_.tm_sec = second;                    
	tm_.tm_isdst = 0;                                                              
}

void CommonTools::StringToDatetime2(std::string str, tm& tm_)
{
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

	year = atoi(str.substr(0, 4).c_str());
	month = atoi(str.substr(4, 2).c_str());
	day = atoi(str.substr(6, 2).c_str());
	hour = atoi(str.substr(9, 2).c_str());
	minute = atoi(str.substr(11, 2).c_str());
	second = atoi(str.substr(13, 2).c_str());

	tm_.tm_year = year - 1900;
	tm_.tm_mon = month - 1;
	tm_.tm_mday = day;
	tm_.tm_hour = hour;
	tm_.tm_min = minute;
	tm_.tm_sec = second;
	tm_.tm_isdst = 0;
}

void CommonTools::StringToDatetime3(std::string str, tm& tm_)
{
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

	hour = atoi(str.substr(0, 2).c_str());
	minute = atoi(str.substr(3, 2).c_str());
	second = atoi(str.substr(6, 2).c_str());

	tm_.tm_year = year;
	tm_.tm_mon = month;
	tm_.tm_mday = day;
	tm_.tm_hour = hour;
	tm_.tm_min = minute;
	tm_.tm_sec = second;
	tm_.tm_isdst = 0;
}

void CommonTools::StringToTimeForm1(std::string& str_src, std::string& str_dst)
{
	tm tm_;
	char* cha = (char*)str_src.data();
	int year = 0, month = 0, day = 0;

	sscanf(cha, "%d-%d-%d", &year, &month, &day);
	tm_.tm_year = year - 1900;
	tm_.tm_mon = month - 1;
	tm_.tm_mday = day;
	tm_.tm_hour = 23;
	tm_.tm_min = 59;
	tm_.tm_sec = 59;
	tm_.tm_isdst = 0;

	char s[100] = { 0 };
	strftime(s, sizeof(s), "%Y%m%d%H%M%S", &tm_);

	str_dst = s;
}

std::string CommonTools::GetLocalTimeForm1()
{
	char s[100] = { 0 };
	time_t tt = time(NULL);
	struct tm *ptm = localtime(&tt);
	strftime(s, sizeof(s), "%Y%m%d%H%M%S", ptm);

	return std::string(s);
}

std::string CommonTools::GetLocalTimeForm2()
{
	char s[100] = { 0 };
	time_t tt = time(NULL);
	struct tm *ptm = localtime(&tt);
	strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", ptm);

	return std::string(s);
}

std::string CommonTools::GetLocalTimeForm3()
{
	char s[100] = { 0 };
	time_t tt = time(NULL);
	struct tm *ptm = localtime(&tt);
	strftime(s, sizeof(s), "%Y/%m/%d %H:%M:%S", ptm);

	return std::string(s);
}

std::string CommonTools::GetLocalTimeForm4()
{
	char s[100] = { 0 };
	time_t tt = time(NULL);
	struct tm *ptm = localtime(&tt);
	strftime(s, sizeof(s), "%Y-%m-%d 00:00:00", ptm);

	return std::string(s);
}

std::string CommonTools::GetLocalTimeForm5()
{
	char s[100] = { 0 };
	time_t tt = time(NULL);
	struct tm *ptm = localtime(&tt);
	strftime(s, sizeof(s), "%Y-%m-%d 23:59:59", ptm);

	return std::string(s);
}

std::string CommonTools::GetLocalTimeForm6()
{
	char s[100] = { 0 };
	time_t tt = time(NULL);
	struct tm* ptm = localtime(&tt);
	strftime(s, sizeof(s), "%Y-%m-%d", ptm);

	return std::string(s);
}

std::string CommonTools::GetLocalTimeForm7()
{
	char s[100] = { 0 };
	time_t tt = time(NULL);
	struct tm* ptm = localtime(&tt);
	strftime(s, sizeof(s), "%Y%m%dT%H%M%SZ", ptm);

	return std::string(s);
}

std::string CommonTools::GetLocalTimeForm8()
{
	char s[100] = { 0 };
	time_t tt = time(NULL);
	struct tm* ptm = localtime(&tt);
	strftime(s, sizeof(s), "%Y%m%d", ptm);

	return std::string(s);
}

std::string CommonTools::GetLocalTimeForm9()
{
	char s[100] = { 0 };
	time_t tt = time(NULL);
	struct tm* ptm = localtime(&tt);
	strftime(s, sizeof(s), "%H%M%S", ptm);

	return std::string(s);
}

int64_t CommonTools::GetLocalTimeStamp_ns()
{
	auto timeNow = chrono::duration_cast<chrono::nanoseconds>(chrono::system_clock::now().time_since_epoch());
	return timeNow.count();
}

int64_t CommonTools::GetLocalTimeStamp_ms()
{
	auto timeNow = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch());
	return timeNow.count();
}

int64_t CommonTools::GetLocalTimeStamp_s()
{
	auto timeNow = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch());
	return timeNow.count();
}

int CommonTools::GetLocalMillisecond()
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

std::string CommonTools::GetLocalTimeForm10()
{
	time_t curr_time = time(NULL);
	struct tm* pt = NULL;

#ifdef WIN32
	pt = localtime(&curr_time);
#else
	struct tm t1;
	pt = localtime_r(&curr_time, &t1);
#endif

	char szInfo[256] = {};

	snprintf(szInfo, 256, "%04d/%02d/%02d %02d:%02d:%02d.%03d",
		pt->tm_year + 1900,
		pt->tm_mon + 1, pt->tm_mday,
		pt->tm_hour, pt->tm_min, pt->tm_sec, GetLocalMillisecond());

	return std::string(szInfo);
}

std::string CommonTools::Hex2String(const unsigned char * pData, size_t nSize)
{
	std::string str;
	char buf[3] = { 0 };

	for (size_t i = 0; i<nSize; i++)
	{
		std::snprintf(buf, 3, "%02x", *(pData + i));
		str += buf;
	}

	return str;
}

std::string CommonTools::String2Hex(std::string str, std::string separator)
{
	const std::string hex = "0123456789ABCDEF";
	std::stringstream ss;

	for (std::string::size_type i = 0; i < str.size(); ++i)
		ss << hex[(unsigned char)str[i] >> 4] << hex[(unsigned char)str[i] & 0xf] << separator;

	return ss.str();
}

std::string CommonTools::Dec2BinString(unsigned long long dec)
{
	int i = 0, j = 0, a[1000], bit[64];

	while (dec)
	{
		a[j] = dec % 2;
		dec /= 2;
		j++;
	}

	for (i = j - 1; i >= 0; i--) 
	{
		bit[i] = a[i];
	}

	if (j - 1 < 63) {
		for (int m = j; m < 64; m++) {
			bit[m] = 0;
		}
	}

	ostringstream oss;

	for (int i = 0; i < 64; i++) {
		oss << bit[i];
	}

	return oss.str();
}

std::string CommonTools::ToString(const int value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::string CommonTools::ToString(const double value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

double CommonTools::String2Double(const std::string& str)
{
	double value;
	std::istringstream iss(str.c_str());
	iss >> value;
	return value;
}

int CommonTools::String2Int(const std::string& str)
{
	int value;
	std::istringstream iss(str.c_str());
	iss >> value;
	return value;
}

int CommonTools::HexString2Int(const std::string& str)
{
	int n = (int)str.length();
	long long num = 0;

	for (int i = 0; i < n; ++i) 
	{
		if (str[i] >= '0' && str[i] <= '9') 
		{
			num += (str[i] - '0');
		}
		else if (str[i] >= 'a' && str[i] <= 'f') 
		{
			num += (str[i] - 'a' + 10);
		}
		else if (str[i] >= 'A' && str[i] <= 'F') 
		{
			num += (str[i] - 'A' + 10);
		}

		if (i < n - 1)
		{
			num *= 16;
		}
	}

	return (int)num;
}

std::string CommonTools::GetTimeString(const std::string& strDate, bool ignoreNowOnly)
{
	std::string strTime;

	do
	{
		if (true == ignoreNowOnly)
		{
			time_t timep;
			struct tm *tm1;
			time(&timep);
			tm1 = gmtime(&timep);

			struct tm tm2;
			String2Time(strDate, tm2);

			if (tm1->tm_year != tm2.tm_year ||
				tm1->tm_mon != tm2.tm_mon ||
				tm1->tm_mday != tm2.tm_mday)
			{
				return strDate;
			}
		}

		size_t nPos = strDate.find_first_of(" ");

		if (nPos == std::string::npos)
		{
			return "";
		}

		strTime = strDate.substr(nPos + 1);

	} while (false);

	return strTime;

}

void CommonTools::String2Time(std::string time_str, struct tm& tmbuf)
{
	sscanf(time_str.c_str(), "%04d-%02d-%02d %02d:%02d:%02d",
		&tmbuf.tm_year, &tmbuf.tm_mon, &tmbuf.tm_mday,
		&tmbuf.tm_hour, &tmbuf.tm_min, &tmbuf.tm_sec);

	tmbuf.tm_year -= 1900;
	tmbuf.tm_mon -= 1;
}

void CommonTools::String2TimeExt(std::string time_str, tm & tmbuf)
{
	sscanf(time_str.c_str(), "%04d%02d%02d%02d%02d%02d",
		&tmbuf.tm_year, &tmbuf.tm_mon, &tmbuf.tm_mday,
		&tmbuf.tm_hour, &tmbuf.tm_min, &tmbuf.tm_sec);

	tmbuf.tm_year -= 1900;
	tmbuf.tm_mon -= 1;
}

time_t CommonTools::String2Time(std::string time_str, bool standard)
{
	struct tm tmp;

	if (standard)
	{
		String2Time(time_str, tmp);
	}
	else
	{
		String2TimeExt(time_str, tmp);
	}

	return mktime(&tmp);
}

bool CommonTools::CalcTimeLength(std::string start, std::string last, std::string & str_day, std::string & str_hour, std::string & str_minute)
{
	time_t tmStart = String2Time(start);
	time_t tmLast = String2Time(last);

	if (tmStart > tmLast)
		return false;

	unsigned long seconds = (unsigned long)difftime(tmLast, tmStart);

	int day = seconds / 86400;
	seconds = seconds % 86400;

	int hour = seconds / 3600;
	seconds = seconds % 3600;

	int minute = seconds / 60;
	//int second = seconds % 60;

	std::ostringstream oss;
	oss.str("");

	if (day > 0)
	{
		oss << day;
		str_day = oss.str();
	}

	oss.str("");
	if (hour > 0 || day > 0)
	{
		oss << hour;
		str_hour = oss.str();
	}
	
	oss.str("");
	oss << minute;
	str_minute = oss.str();

	return true;
}

bool CommonTools::CalcTimeLengthExt(std::string start, std::string last, unsigned long & timelength, bool standard)
{
	if (start.empty() || last.empty())
	{
		return false;
	}

	time_t tmStart = String2Time(start, standard);
	time_t tmLast = String2Time(last, standard);

	if (tmStart > tmLast)
	{
		timelength = (unsigned long)difftime(tmStart, tmLast);
	}
	else
	{
		timelength = (unsigned long)difftime(tmLast, tmStart);
	}	

	return true;
}

static const uint8_t aucCRCHi[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40
};
static const uint8_t aucCRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
	0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
	0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
	0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
	0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
	0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
	0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
	0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
	0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
	0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
	0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
	0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
	0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
	0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
	0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
	0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
	0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
	0x41, 0x81, 0x80, 0x40
};

uint16_t CommonTools::usMBCRC16(uint8_t * pucFrame, uint8_t usLen)
{
	uint8_t ucCRCHi = 0xFF;
	uint8_t ucCRCLo = 0xFF;
	int iIndex;
	while (usLen--)
	{
		iIndex = ucCRCLo ^ *(pucFrame++);
		ucCRCLo = (uint8_t)(ucCRCHi ^ aucCRCHi[iIndex]);
		ucCRCHi = aucCRCLo[iIndex];
	}
	return (uint16_t)(ucCRCHi << 8 | ucCRCLo);
}

std::string CommonTools::DateToString(time_t time)
{
	tm* tm_ = localtime(&time);					   // 将time_t格式转换为tm结构体
	int year = tm_->tm_year + 1900;                // 临时变量，年，由于tm结构体存储的是从1900年开始的时间，所以临时变量int为tm_year加上1900        
	int month = tm_->tm_mon + 1;                   // 临时变量，月，由于tm结构体的月份存储范围为0-11，所以临时变量int为tm_mon加上1
	int day = tm_->tm_mday;                        // 临时变量，日

	char s[20] = { 0 };                             // 定义总日期时间char*变量。
	sprintf(s, "%04d/%02d/%02d", year, month, day); // 将年月日时分秒合并。
	std::string str(s);                             // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。

	return str;
}

int FastSecondToDate(const long long& unix_sec, struct tm* tm, int time_zone)
{
	static const int kHoursInDay = 24;
	static const int kMinutesInHour = 60;
	static const int kDaysFromUnixTime = 2472632;
	static const int kDaysFromYear = 153;
	static const int kMagicUnkonwnFirst = 146097;
	static const int kMagicUnkonwnSec = 1461;
	tm->tm_sec = unix_sec % kMinutesInHour;
	int i = (int)(unix_sec / kMinutesInHour);
	tm->tm_min = i % kMinutesInHour; //nn
	i /= kMinutesInHour;
	tm->tm_hour = (i + time_zone) % kHoursInDay; // hh
	tm->tm_mday = (i + time_zone) / kHoursInDay;
	int a = tm->tm_mday + kDaysFromUnixTime;
	int b = (a * 4 + 3) / kMagicUnkonwnFirst;
	int c = (-b * kMagicUnkonwnFirst) / 4 + a;
	int d = ((c * 4 + 3) / kMagicUnkonwnSec);
	int e = -d * kMagicUnkonwnSec;
	e = e / 4 + c;
	int m = (5 * e + 2) / kDaysFromYear;
	tm->tm_mday = -(kDaysFromYear * m + 2) / 5 + e + 1;
	tm->tm_mon = (-m / 10) * 12 + m + 2;
	tm->tm_year = b * 100 + d - 6700 + (m / 10);
	return 0;
}

std::string CommonTools::DateToStringExt(long long time)
{
	tm tm_;
	FastSecondToDate(time, &tm_, 0);			  // 将time_t格式转换为tm结构体
	int year = tm_.tm_year + 1920;                // 临时变量，年，由于tm结构体存储的是从1920年开始的时间，所以临时变量int为tm_year加上1920        
	int month = tm_.tm_mon + 1;                   // 临时变量，月，由于tm结构体的月份存储范围为0-11，所以临时变量int为tm_mon加上1
	int day = tm_.tm_mday;                        // 临时变量，日

	char s[20] = { 0 };                             // 定义总日期时间char*变量。
	sprintf(s, "%04d/%02d/%02d", year, month, day); // 将年月日时分秒合并。
	std::string str(s);                             // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。

	return str;
}


std::string CommonTools::DateToStringExt2(long long time)
{
	tm tm_;
	FastSecondToDate(time, &tm_, 0);			  // 将time_t格式转换为tm结构体
	int year = tm_.tm_year + 1920;                // 临时变量，年，由于tm结构体存储的是从1920年开始的时间，所以临时变量int为tm_year加上1920        
	int month = tm_.tm_mon + 1;                   // 临时变量，月，由于tm结构体的月份存储范围为0-11，所以临时变量int为tm_mon加上1
	int day = tm_.tm_mday;                        // 临时变量，日

	char s[20] = { 0 };                             // 定义总日期时间char*变量。
	sprintf(s, "%04d%02d%02d", year, month, day); // 将年月日时分秒合并。
	std::string str(s);                             // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。

	return str;
}

std::string CommonTools::DateToStringExt3(long long time)
{
	tm tm_;
	FastSecondToDate(time, &tm_, 0);			  // 将time_t格式转换为tm结构体
	int year = tm_.tm_year + 1900;                // 临时变量，年，由于tm结构体存储的是从1920年开始的时间，所以临时变量int为tm_year加上1920        
	int month = tm_.tm_mon + 1;                   // 临时变量，月，由于tm结构体的月份存储范围为0-11，所以临时变量int为tm_mon加上1
	int day = tm_.tm_mday;                        // 临时变量，日

	char s[20] = { 0 };                             // 定义总日期时间char*变量。
	sprintf(s, "%04d%02d%02d", year, month, day); // 将年月日时分秒合并。
	std::string str(s);                             // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。

	return str;
}

std::string CommonTools::DateTimeToString(long long time)
{
	tm tm_;
	FastSecondToDate(time, &tm_, 0);			  // 将time_t格式转换为tm结构体
	int year = tm_.tm_year + 1900;                // 临时变量，年，由于tm结构体存储的是从1900年开始的时间，所以临时变量int为tm_year加上1900        
	int month = tm_.tm_mon + 1;                   // 临时变量，月，由于tm结构体的月份存储范围为0-11，所以临时变量int为tm_mon加上1
	int day = tm_.tm_mday;                        // 临时变量，日
	int hour = tm_.tm_hour;
	int min = tm_.tm_min;
	int sec = tm_.tm_sec;

	char s[20] = { 0 };                             // 定义总日期时间char*变量。
	sprintf(s, "%04d%02d%02d%02d%02d%02d", year, month, day, hour, min, sec); 
	std::string str(s);                             // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。

	return str;
}

std::string CommonTools::DateTimeToStringExt(long long time)
{
	tm tm_;
	FastSecondToDate(time, &tm_, 0);			  // 将time_t格式转换为tm结构体
	int year = tm_.tm_year + 1900;                // 临时变量，年，由于tm结构体存储的是从1900年开始的时间，所以临时变量int为tm_year加上1900        
	int month = tm_.tm_mon + 1;                   // 临时变量，月，由于tm结构体的月份存储范围为0-11，所以临时变量int为tm_mon加上1
	int day = tm_.tm_mday;                        // 临时变量，日
	int hour = tm_.tm_hour;
	int min = tm_.tm_min;
	int sec = tm_.tm_sec;

	char s[20] = { 0 };                             // 定义总日期时间char*变量。
	sprintf(s, "%.4d%.2d%.2dT%.2d%.2d%.2dZ", year, month, day, hour, min, sec);
	std::string str(s);                             // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。

	return str;
}

std::string CommonTools::GetUtcTimeForm1()
{
	time_t  now = time(0);
	char* data = ctime(&now);
	char result[100] = { 0 };

	//转换UTC时间
	//  tm *gmtime(const time_t *time);  该函数返回一个指向 time 的指针，time 为 tm 结构，用协调世界时（UTC）也被称为格林尼治标准时间（GMT）表示。
	tm* gm = gmtime(&now);
	// char * asctime ( const struct tm * time );  该函数返回一个指向字符串的指针，字符串包含了 time 所指向结构中存储的信息，返回形式为：day month date hours:minutes:seconds year\n\0。
	data = asctime(gm);

	strftime(result, sizeof(result), "%Y%m%d%H%M%S", gm);
	std::string str(result);
	return str;
}

std::string CommonTools::GetUtcTimeForm2()
{
	time_t  now = time(0);
	char* data = ctime(&now);
	char result[100] = { 0 };

	//转换UTC时间
	//  tm *gmtime(const time_t *time);  该函数返回一个指向 time 的指针，time 为 tm 结构，用协调世界时（UTC）也被称为格林尼治标准时间（GMT）表示。
	tm* gm = gmtime(&now);
	// char * asctime ( const struct tm * time );  该函数返回一个指向字符串的指针，字符串包含了 time 所指向结构中存储的信息，返回形式为：day month date hours:minutes:seconds year\n\0。
	data = asctime(gm);

	strftime(result, sizeof(result), "%Y%m%dT%H%M%SZ", gm);
	std::string str(result);
	return str;
}

std::string CommonTools::GetUtcTimeForm3()
{
	time_t  now = time(0);
	char* data = ctime(&now);
	char result[100] = { 0 };

	//转换UTC时间
	//  tm *gmtime(const time_t *time);  该函数返回一个指向 time 的指针，time 为 tm 结构，用协调世界时（UTC）也被称为格林尼治标准时间（GMT）表示。
	tm* gm = gmtime(&now);
	// char * asctime ( const struct tm * time );  该函数返回一个指向字符串的指针，字符串包含了 time 所指向结构中存储的信息，返回形式为：day month date hours:minutes:seconds year\n\0。
	data = asctime(gm);

	strftime(result, sizeof(result), "%Y%m%d", gm);
	std::string str(result);
	return str;
}

std::string CommonTools::GetUtcTimeForm4()
{
	time_t  now = time(0);
	char* data = ctime(&now);
	char result[100] = { 0 };

	//转换UTC时间
	//  tm *gmtime(const time_t *time);  该函数返回一个指向 time 的指针，time 为 tm 结构，用协调世界时（UTC）也被称为格林尼治标准时间（GMT）表示。
	tm* gm = gmtime(&now);
	// char * asctime ( const struct tm * time );  该函数返回一个指向字符串的指针，字符串包含了 time 所指向结构中存储的信息，返回形式为：day month date hours:minutes:seconds year\n\0。
	data = asctime(gm);

	strftime(result, sizeof(result), "%H%M%S", gm);
	std::string str(result);
	return str;
}

//ConvertUtcToLocalTime(NULL,"20211213T105236Z");
//std::string CommonTools::ConvertUtcToLocalTime(struct tm* t2, const char* date)
//{
//	if (NULL == date)
//	{
//		return "";
//	}
//
//	struct tm t;
//	std::string str = date;
//
//	memset(&t, 0, sizeof(t));
//
//	t.tm_year = atoi(str.substr(0, 4).c_str()) - 1900;
//	t.tm_mon = atoi(str.substr(4, 2).c_str()) - 1;
//	t.tm_mday = atoi(str.substr(6, 2).c_str());
//	t.tm_hour = atoi(str.substr(9, 2).c_str());
//	t.tm_min = atoi(str.substr(11, 2).c_str());
//	t.tm_sec = atoi(str.substr(13, 2).c_str());
//
//	time_t tt = _mkgmtime64(&t);
//
//	if (tt != -1) 
//	{
//		if (t2 == NULL) 
//		{
//			t2 = &t;
//		}
//
//		*t2 = *localtime(&tt);
//
//		char ds[24] = { 0 };
//
//		memset(ds, 0, 24);
//
//		sprintf(ds, "%.4d%.2d%.2dT%.2d%.2d%.2dZ", t2->tm_year + 1900,
//			t2->tm_mon + 1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec);
//
//		return ds;
//	}
//
//	return "";
//}

bool CommonTools::IsChinese(const std::string& str)
{
	for (int i = 0;i < str.length();i++)
	{
		if (str[i] < 0) return true;

	}

	return false;

}

//获取指定长度字符串的合法长度，避免出现乱码,dst 目标字符串，len 指定长度  仅支持utf8编码字符串
int CommonTools::GetSubStrLen(const char* dst, int len)
{
	int j = 0;
	int n = 0;
	const char* p = NULL;

	int str_length = len;

	p = dst;

	while (n <= str_length)
	{
		if ((*p & 0XFC) == 0XFC)
		{
			j = 6;
		}
		else if ((*p & 0XF8) == 0XF8)
		{
			j = 5;
		}
		else if ((*p & 0XF0) == 0XF0)
		{
			j = 4;
		}
		else if ((*p & 0XE0) == 0XE0)
		{
			j = 3;
		}
		else if ((*p & 0XC0) == 0XC0)
		{
			j = 2;
		}
		else
		{
			j = 1;
		}

		if (n + j <= str_length)
		{
			n += j;
		}
		else
			break;

		p = p + j;
	}

	return n;
}

bool isChinese(char ch) 
{
	return (ch < 0) ? true : false;
}

int MoveEnglish(const char* input, int input_len) 
{
	int out_len = input_len;
	for (int i = 0; i < input_len; i++)
	{
		if (isChinese(input[i]) == false) 
		{
			out_len++;
		}
	}
	return (out_len > 0) ? out_len : 0;
}

int CommonTools::HalfChinese_GBK(const char* input, int input_len, char* output) 
{
	int output_len = 0;
	char current = *(input + input_len - 1);

	if (isChinese(current) == false)
	{
		output_len = input_len;
		strncpy(output, input, output_len);
		return output_len;
	}
	output_len = input_len;
	if (MoveEnglish(input, input_len) % 2 != 0) 
	{
		output_len--;
	}
	strncpy(output, input, (size_t)output_len);

	return output_len;
}

int CommonTools::HalfChinese_UTF8(const char* input, int input_len, char* output)
{
	int output_len = 0;
	char current = *(input + input_len - 1);

	if (isChinese(current) == false)
	{
		output_len = input_len;
		strncpy(output, input, output_len);
		return output_len;
	}

	//汉字
	output_len = input_len;
	//1110xxxx 10xxxxxx 10xxxxxx
	//第二位和第三位的范围是10000000~10ffffff，转成十六进制是0x80~0xbf，在这个范围内都说明是汉字被截断
	while ((current & 0xff) < 0xc0 && (current & 0xff) >= 0x80)
	{
		output_len--;
		current = *(input + output_len);
	}
	strncpy(output, input, output_len);

	return output_len;
}

std::string trim(const std::string& str)
{
	std::string::size_type pos = str.find_first_not_of(' ');

	if (pos == std::string::npos) 
	{
		return str;
	}

	std::string::size_type pos2 = str.find_last_not_of(' ');

	if (pos2 != std::string::npos) 
	{
		return str.substr(pos, pos2 - pos + 1);
	}

	return str.substr(pos);
}

void split(const std::string& str, std::vector<std::string>* ret_, std::string sep = ",")
{
	if (str.empty()) 
	{
		return;
	}

	std::string tmp;
	std::string::size_type pos_begin = str.find_first_not_of(sep);
	std::string::size_type comma_pos = 0;

	while (pos_begin != std::string::npos) 
	{
		comma_pos = str.find(sep, pos_begin);

		if (comma_pos != std::string::npos) 
		{
			tmp = str.substr(pos_begin, comma_pos - pos_begin);
			pos_begin = comma_pos + sep.length();
		}
		else 
		{
			tmp = str.substr(pos_begin);
			pos_begin = comma_pos;
		}

		if (!tmp.empty()) {
			ret_->push_back(tmp);
			tmp.clear();
		}
	}
}

bool DateVerify(int year, int month, int day)
{
	//只允许1990-01-01 到 2169-06-06
	if (month < 1 || month > 12 || day < 1 || day > 31
		|| year < 1990 || year > 2169) 
	{
		return false;
	}

	switch (month) 
	{
		case 4:
		case 6:
		case 9:
		case 11:
			if (day > 30) 
			{   
				// 4.6.9.11月天数不能大于30
				return false;
			}
			break;
		case 2:
		{
			bool bLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
			if ((bLeapYear && day > 29) || (!bLeapYear && day > 28)) 
			{
				// 闰年2月不能大于29天;平年2月不能大于28天
				return false;
			}
		}
		break;
		default:
			break;
	}


	if (2169 == year)
	{
		if (month > 6)
		{
			return false;
		}
		else if (6 == month)
		{
			if (day > 6)
			{
				return false;
			}
		}
	}


	return true;
}

// 校验yyyy/mm/dd
bool CommonTools::CheckDateValid(const std::string& strDate)
{
	std::string strPureDate = trim(strDate);
	if (strPureDate.length() < 8 || strPureDate.length() > 10) 
	{
		return false;
	}

	std::vector<std::string> vecFields;
	split(strPureDate, &vecFields, "/");

	if (vecFields.size() != 3)
	{
		return false;
	}

	// TODO:这里最好再下判断字符转换是否成功
	int nYear = atoi(vecFields[0].c_str());
	int nMonth = atoi(vecFields[1].c_str());
	int nDay = atoi(vecFields[2].c_str());

	return DateVerify(nYear, nMonth, nDay);
}

// 校验HH:MM:SS
bool CommonTools::CheckTimeValid(const std::string& strTime)
{
	std::string strPureTime = trim(strTime);
	if (strPureTime.length() < 5 || strPureTime.length() > 8) {
		return false;
	}

	std::vector<std::string> vecFields;
	split(strPureTime, &vecFields, ":");

	if (vecFields.size() != 3) {
		return false;
	}

	int nHour = atoi(vecFields[0].c_str());
	int nMinute = atoi(vecFields[1].c_str());
	int nSecond = atoi(vecFields[2].c_str());

	bool bValid = (nHour >= 0 && nHour <= 23);
	bValid = bValid && (nMinute >= 0 && nMinute <= 59);
	bValid = bValid && (nSecond >= 0 && nSecond <= 59);

	return bValid;
}

// 日期格式为: yyyy/mm/dd || yyyy/mm/dd HH:MM:SS
bool CommonTools::CheckDateTimeValid(const std::string& strDateTime)
{
	std::string strPureDateTime = trim(strDateTime);

	std::vector<std::string> vecFields;
	split(strPureDateTime, &vecFields, " ");

	if (vecFields.size() != 1 && vecFields.size() != 2) {
		return false;
	}

	// 仅有日期
	if (vecFields.size() == 1) {
		return CheckDateValid(vecFields[0]);
	}

	return CheckDateValid(vecFields[0]) && CheckTimeValid(vecFields[1]);
}

std::string CommonTools::GetMacAddress()
{
#ifdef WIN32
	std::string macAddress;
	char mac[1024] = { 0 };

	NCB ncb;
	typedef struct _ASTAT_
	{
		ADAPTER_STATUS   adapt;
		NAME_BUFFER   NameBuff[30];
	}ASTAT, * PASTAT;

	ASTAT Adapter;

	typedef struct _LANA_ENUM
	{
		UCHAR   length;
		UCHAR   lana[MAX_LANA];
	}LANA_ENUM;

	LANA_ENUM lana_enum;
	UCHAR uRetCode;

	memset(&ncb, 0, sizeof(ncb));
	memset(&lana_enum, 0, sizeof(lana_enum));
	ncb.ncb_command = NCBENUM;
	ncb.ncb_buffer = (unsigned char*)&lana_enum;
	ncb.ncb_length = sizeof(LANA_ENUM);
	uRetCode = Netbios(&ncb);

	if (uRetCode != NRC_GOODRET)
		return macAddress;

	for (int lana = 0; lana < lana_enum.length; lana++)
	{
		ncb.ncb_command = NCBRESET;
		ncb.ncb_lana_num = lana_enum.lana[lana];
		uRetCode = Netbios(&ncb);
		if (uRetCode == NRC_GOODRET)
			break;
	}

	if (uRetCode != NRC_GOODRET)
		return macAddress;

	memset(&ncb, 0, sizeof(ncb));
	ncb.ncb_command = NCBASTAT;
	ncb.ncb_lana_num = lana_enum.lana[0];
	strcpy((char*)ncb.ncb_callname, "*");
	ncb.ncb_buffer = (unsigned char*)&Adapter;
	ncb.ncb_length = sizeof(Adapter);
	uRetCode = Netbios(&ncb);

	if (uRetCode != NRC_GOODRET)
		return macAddress;

	sprintf(mac, "%02X-%02X-%02X-%02X-%02X-%02X",
		Adapter.adapt.adapter_address[0],
		Adapter.adapt.adapter_address[1],
		Adapter.adapt.adapter_address[2],
		Adapter.adapt.adapter_address[3],
		Adapter.adapt.adapter_address[4],
		Adapter.adapt.adapter_address[5]);

	macAddress = mac;

	return macAddress;
#else
	std::string macAddress;
	char mac[1024] = { 0 };

	struct ifreq ifreq;
	int sock;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return macAddress;
	}
	strcpy(ifreq.ifr_name, "eth0");    //Currently, only get eth0

	if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
	{
		close(sock);
		perror("ioctl");
		return macAddress;
	}

	sprintf(mac, "%02X-%02X-%02X-%02X-%02X-%02X", 
		(unsigned char)ifreq.ifr_hwaddr.sa_data[0], 
		(unsigned char)ifreq.ifr_hwaddr.sa_data[1], 
		(unsigned char)ifreq.ifr_hwaddr.sa_data[2], 
		(unsigned char)ifreq.ifr_hwaddr.sa_data[3], 
		(unsigned char)ifreq.ifr_hwaddr.sa_data[4], 
		(unsigned char)ifreq.ifr_hwaddr.sa_data[5]);

	macAddress = mac;

	close(sock);

	return macAddress;
#endif	
}

#ifdef WIN32
/**
 * @brief 获取注册表数据
 * @param hRoot 根键
 * @param szSubKey 子键
 * @param szValueName 数据项名
 * @param szRegInfo 数据
 */
BOOL GetRegInfo(HKEY hRoot, LPCTSTR szSubKey, LPCTSTR szValueName, LPSTR szRegInfo)
{
	HKEY hKey;
	DWORD dwType = REG_SZ;
	DWORD dwLenData = (DWORD)strlen(szRegInfo);
	LONG lRes = RegCreateKeyEx(hRoot, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	if (lRes != ERROR_SUCCESS)
	{
		if (lRes == 5)
		{
			printf("Please use Administrator Privilege !\n");
		}
		else
		{
			printf("Get Register Info Error! Error Code is ");
			printf("%ld\n", lRes);
		}
		RegCloseKey(hKey);
		RegCloseKey(hRoot);
		return false;
	}
	RegQueryValueEx(hKey, szValueName, 0, &dwType, NULL, &dwLenData);
	lRes = RegQueryValueEx(hKey, szValueName, 0, &dwType, (LPBYTE)szRegInfo, &dwLenData);
	if (lRes != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		RegCloseKey(hRoot);
		return false;
	}
	RegCloseKey(hKey);
	RegCloseKey(hRoot);
	return true;
}

/**
 * @brief 验证注册信息是否是PCI物理网卡(需要以管理员权限运行程序)
 * @param pIpAdapterInfo 指向网卡数据的指针
 */
int IsPCINetCard(const PIP_ADAPTER_INFO pIpAdapterInfo)
{
	//通过注册表特征去除非物理网卡
	CHAR szRegSubKey[255] = "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\";
	CHAR szNetCardRegInfo[255] = "\0";
	StringCchCat(szRegSubKey, sizeof(szRegSubKey), pIpAdapterInfo->AdapterName);
	StringCchCat(szRegSubKey, sizeof(szRegSubKey), "\\Connection");
	if (!GetRegInfo(HKEY_LOCAL_MACHINE, szRegSubKey, "PnPInstanceId", szNetCardRegInfo))
	{
		return -2;
	}
	if (strncmp(szNetCardRegInfo, "PCI", 3) == 0) return 0;
	else return -1;

}
#endif

std::string CommonTools::GetMacAddressExt()
{
#ifdef WIN32
	std::list<std::string> mac_list;
	std::list<std::string> pci_list;

	//net_ada_list nal = net_adapter_helper::get_instance().get_info_win();

	std::unique_ptr< IP_ADAPTER_INFO> pai(new(std::nothrow) IP_ADAPTER_INFO);
	// 1. failed to apply space
	if (nullptr == pai || NULL == pai)
		return "";
	// 2. to get the size of IP_ADAPTER_INFO structure
	unsigned long iai_size = sizeof(IP_ADAPTER_INFO);
	// 3.调用GetAdaptersInfo函数, 填充pIpAdapterInfo指针变量; 其中stSize参数既是一个输入量也是一个输出量
	int ret_val = GetAdaptersInfo(pai.get(), &iai_size);
	if (ERROR_BUFFER_OVERFLOW == ret_val)
	{
		pai.release();
		//重新申请内存空间用来存储所有网卡信息
		pai.reset((IP_ADAPTER_INFO*)(new(std::nothrow) char[iai_size]));
		if (nullptr == pai || NULL == pai)
		{
			return "";
		}
		//再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
		ret_val = GetAdaptersInfo(pai.get(), &iai_size);
	}
	if (ERROR_SUCCESS == ret_val)
	{
		// 3. to get information
		IP_ADAPTER_INFO* ppai = pai.get();
		while (ppai)
		{
			// mac
			std::string str_mac;
			for (DWORD i = 0; i < ppai->AddressLength; i++)
			{
				if (i < ppai->AddressLength - 1)
				{
					char buf[32] = { 0 };
					sprintf(buf, "%02X-", ppai->Address[i]);
					str_mac += buf;
				}
				else
				{
					char buf[32] = { 0 };
					sprintf(buf, "%02X", ppai->Address[i]);
					str_mac += buf;
				}
			}

			if (0 == IsPCINetCard(ppai))
			{
				pci_list.push_back(str_mac);
			}
			else
			{
				mac_list.push_back(str_mac);
			}

			ppai = ppai->Next;
		} // end while
	}
	else
	{
		return "";// error
	}

	for (std::list<std::string>::iterator it = pci_list.begin(); it != pci_list.end(); ++it)
	{
		return *it;
	}

	for (std::list<std::string>::iterator it2 = mac_list.begin(); it2 != mac_list.end(); ++it2)
	{
		if (!it2->empty())
		{
			return *it2;
		}
	}

	return "";
#else
	return GetMacAddress();
#endif
}

#ifdef WIN32
bool GetAdapterInfo(int adapterNum, std::string& macOUT)
{
	NCB Ncb;
	memset(&Ncb, 0, sizeof(Ncb));
	Ncb.ncb_command = NCBRESET; // 重置网卡，以便我们可以查询
	Ncb.ncb_lana_num = adapterNum;
	if (Netbios(&Ncb) != NRC_GOODRET)
		return false;

	// 准备取得接口卡的状态块
	memset(&Ncb, sizeof(Ncb), 0);
	Ncb.ncb_command = NCBASTAT;
	Ncb.ncb_lana_num = adapterNum;
	strcpy((char*)Ncb.ncb_callname, "*");
	struct ASTAT
	{
		ADAPTER_STATUS adapt;
		NAME_BUFFER nameBuff[30];
	}adapter;
	memset(&adapter, sizeof(adapter), 0);
	Ncb.ncb_buffer = (unsigned char*)&adapter;
	Ncb.ncb_length = sizeof(adapter);
	if (Netbios(&Ncb) != 0)
		return false;
	char acMAC[32];
	sprintf(acMAC, "%02X-%02X-%02X-%02X-%02X-%02X",
		int(adapter.adapt.adapter_address[0]),
		int(adapter.adapt.adapter_address[1]),
		int(adapter.adapt.adapter_address[2]),
		int(adapter.adapt.adapter_address[3]),
		int(adapter.adapt.adapter_address[4]),
		int(adapter.adapt.adapter_address[5]));
	macOUT = acMAC;
	return true;
}
#endif // WIN32

bool CommonTools::GetAllMacAddress(std::list<std::string>& list)
{
#ifdef WIN32
	// 取得网卡列表
	LANA_ENUM adapterList;
	NCB Ncb;
	memset(&Ncb, 0, sizeof(NCB));
	Ncb.ncb_command = NCBENUM;
	Ncb.ncb_buffer = (unsigned char*)&adapterList;
	Ncb.ncb_length = sizeof(adapterList);
	Netbios(&Ncb);

	// 取得MAC
	for (int i = 0; i < adapterList.length; ++i)
	{
		std::string macOUT;
		if (GetAdapterInfo(adapterList.lana[i], macOUT))
		{
			if (macOUT.empty())
			{
				continue;
			}

			list.push_back(macOUT);
		}
	}
#else
	std::string addr_eth0;

	addr_eth0 = GetMacAddress();

	list.push_back(addr_eth0);

	std::string macAddress;
	char mac[1024] = { 0 };

	struct ifreq ifreq;
	int sock;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return false;
	}
	strcpy(ifreq.ifr_name, "eth1");

	if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
	{
		close(sock);
		perror("ioctl");
		return false;
	}

	sprintf(mac, "%02X-%02X-%02X-%02X-%02X-%02X",
		(unsigned char)ifreq.ifr_hwaddr.sa_data[0],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[1],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[2],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[3],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[4],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[5]);

	macAddress = mac;

	close(sock);

	list.push_back(macAddress);
#endif // WIN32

	return true;
}

bool CommonTools::GetAllMacAddressExt(std::list<std::string>& list)
{
#ifdef WIN32
	//net_ada_list nal = net_adapter_helper::get_instance().get_info_win();

	std::unique_ptr< IP_ADAPTER_INFO> pai(new(std::nothrow) IP_ADAPTER_INFO);
	// 1. failed to apply space
	if (nullptr == pai || NULL == pai)
		return false;
	// 2. to get the size of IP_ADAPTER_INFO structure
	unsigned long iai_size = sizeof(IP_ADAPTER_INFO);
	// 3.调用GetAdaptersInfo函数, 填充pIpAdapterInfo指针变量; 其中stSize参数既是一个输入量也是一个输出量
	int ret_val = GetAdaptersInfo(pai.get(), &iai_size);
	if (ERROR_BUFFER_OVERFLOW == ret_val)
	{
		pai.release();
		//重新申请内存空间用来存储所有网卡信息
		pai.reset((IP_ADAPTER_INFO*)(new(std::nothrow) char[iai_size]));
		if (nullptr == pai || NULL == pai)
		{
			return false;
		}
		//再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
		ret_val = GetAdaptersInfo(pai.get(), &iai_size);
	}
	if (ERROR_SUCCESS == ret_val)
	{
		// 3. to get information
		IP_ADAPTER_INFO* ppai = pai.get();
		while (ppai)
		{
			// mac
			std::string str_mac;
			for (DWORD i = 0; i < ppai->AddressLength; i++)
			{
				if (i < ppai->AddressLength - 1)
				{
					char buf[32] = { 0 };
					sprintf(buf, "%02X-", ppai->Address[i]);
					str_mac += buf;
				}
				else
				{
					char buf[32] = { 0 };
					sprintf(buf, "%02X", ppai->Address[i]);
					str_mac += buf;
				}
			}

			list.push_back(str_mac);

			ppai = ppai->Next;
		} // end while
	}
	else
	{
		return false;// error
	}

	return true;
#else
	return GetAllMacAddress(list);
#endif
}

std::string CommonTools::GetLocalIpAddress()
{
#ifdef WIN32
	WSADATA wsaData;
	char name[255];
	char* ip;
	PHOSTENT hostinfo;
	std::string ipStr;

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) == 0)
	{
		if (gethostname(name, sizeof(name)) == 0)
		{
			if ((hostinfo = gethostbyname(name)) != NULL)
			{
				ip = inet_ntoa(*(struct in_addr*)*hostinfo->h_addr_list);
				ipStr = ip;
			}
		}
		WSACleanup();
	}
	return ipStr;

#else
	int inet_sock;
	struct ifreq ifr;
	char ip[32] = { NULL };
	std::string strip;

	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (inet_sock < 0)
	{
		return "";
	}

	strcpy(ifr.ifr_name, "eth0");
	ioctl(inet_sock, SIOCGIFADDR, &ifr);
	strcpy(ip, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));

	if (0 == strlen(ip))
	{
		strcpy(ifr.ifr_name, "eth1");
		ioctl(inet_sock, SIOCGIFADDR, &ifr);
		strcpy(ip, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));
	}

	close(inet_sock);

	return string(ip);
#endif // WIN32

}

bool CommonTools::GetLocalIpAddress(std::list<std::string>& iplist)
{
	bool result = false;
	std::string strip;

#ifdef WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	char name[255] = { 0 };

	if (gethostname(name, sizeof(name)) == 0)
	{
		struct hostent* host = gethostbyname(name);
		if (host)
		{
			//IP地址
			for (int i = 0; host->h_addr_list[i]; i++)
			{
				strip = inet_ntoa(*(struct in_addr*)host->h_addr_list[i]);
				iplist.push_back(strip);
			}

			result = true;
		}
	}

	WSACleanup();

#else
	int inet_sock;
	struct ifreq ifr;
	char ip[32] = { NULL };

	inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (inet_sock < 0)
	{
		return "";
	}

	memset(&ifr, 0, sizeof(ifreq));
	strcpy(ifr.ifr_name, "eth0");
	ioctl(inet_sock, SIOCGIFADDR, &ifr);
	strcpy(ip, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));
	if (0 != strlen(ip))
	{
		strip = ip;
		iplist.push_back(strip);
	}

	memset(&ifr, 0, sizeof(ifreq));
	strcpy(ifr.ifr_name, "eth1");
	ioctl(inet_sock, SIOCGIFADDR, &ifr);
	strcpy(ip, inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));
	if (0 != strlen(ip))
	{
		strip = ip;
		iplist.push_back(strip);
	}

	close(inet_sock);

	result = true;

#endif // WIN32

	return result;
}

#ifdef WIN32
inline void FreeIpForwardTable(PMIB_IPFORWARDTABLE pIpRouteTab)
{
	if (pIpRouteTab != NULL)
	{
		::GlobalFree(pIpRouteTab);
		pIpRouteTab = NULL;
	}
}

inline PMIB_IPFORWARDTABLE GetIpForwardTable(BOOL bOrder)
{
	PMIB_IPFORWARDTABLE pIpRouteTab = NULL;
	DWORD dwActualSize = 0;

	// 查询所需缓冲区的大小  
	if (::GetIpForwardTable(pIpRouteTab, &dwActualSize, bOrder) == ERROR_INSUFFICIENT_BUFFER)
	{
		// 为MIB_IPFORWARDTABLE结构申请内存  
		pIpRouteTab = (PMIB_IPFORWARDTABLE)::GlobalAlloc(GPTR, dwActualSize);
		// 获取路由表  
		if (::GetIpForwardTable(pIpRouteTab, &dwActualSize, bOrder) == NO_ERROR)
			return pIpRouteTab;
		::GlobalFree(pIpRouteTab);
	}
	return NULL;
}
#endif

int CommonTools::GetCurrentUsingIp(std::string& strLoalhostIp)
{
#ifdef WIN32
	PMIB_IPFORWARDTABLE pIpRouteTable = GetIpForwardTable(TRUE);

	if (pIpRouteTable != NULL)
	{
		DWORD dwCurrIndex;
		struct in_addr inadDest;
		struct in_addr inadMask;
		struct in_addr inadGateway;
		char szDestIp[128] = { 0 };
		char szMaskIp[128] = { 0 };
		char szGatewayIp[128] = { 0 };
		DWORD IfIndex = 0;
		DWORD ForwardMetric1 = 0;

		if (pIpRouteTable->dwNumEntries > 0)
		{
			for (int i = 0; i < (int)pIpRouteTable->dwNumEntries; i++)
			{
				dwCurrIndex = pIpRouteTable->table[i].dwForwardIfIndex;
				// 目的地址  
				inadDest.s_addr = pIpRouteTable->table[i].dwForwardDest;
				strcpy_s(szDestIp, sizeof(szDestIp), inet_ntoa(inadDest));
				// 子网掩码  
				inadMask.s_addr = pIpRouteTable->table[i].dwForwardMask;
				strcpy_s(szMaskIp, sizeof(szDestIp), inet_ntoa(inadMask));
				// 网关地址  
				inadGateway.s_addr = pIpRouteTable->table[i].dwForwardNextHop;
				strcpy_s(szGatewayIp, sizeof(szDestIp), inet_ntoa(inadGateway));
				if ((strcmp(szDestIp, "0.0.0.0") == 0) && (strcmp(szMaskIp, "0.0.0.0") == 0))
				{
					if (i == 0)
					{
						ForwardMetric1 = pIpRouteTable->table[i].dwForwardMetric1;
						IfIndex = pIpRouteTable->table[i].dwForwardIfIndex;
						struct in_addr inadDest;
						inadDest.s_addr = pIpRouteTable->table[i].dwForwardDest;
					}
					else if (ForwardMetric1 > pIpRouteTable->table[i].dwForwardMetric1)
					{
						ForwardMetric1 = pIpRouteTable->table[i].dwForwardMetric1;
						IfIndex = pIpRouteTable->table[i].dwForwardIfIndex;
						struct in_addr inadDest;
						inadDest.s_addr = pIpRouteTable->table[i].dwForwardDest;
					}
				}
			}
		}
		else
		{
			//IPROUTETABLEZERO 
			FreeIpForwardTable(pIpRouteTable);
			return -1;
		}

		FreeIpForwardTable(pIpRouteTable);

		if (IfIndex > 0)
		{
			PBYTE m_pBuffer = new BYTE[MAX_PATH];
			if (NULL == m_pBuffer)
			{
				return -2;
			}

			ULONG m_ulSize = MAX_PATH;

			DWORD ipdwSize = 0;
			DWORD m_dwResult;
			PMIB_IPADDRTABLE pAddrTable;
			PMIB_IPADDRROW pAddrRow;
			in_addr ia;

			GetIpAddrTable((PMIB_IPADDRTABLE)m_pBuffer, &m_ulSize, TRUE);

			delete[] m_pBuffer;

			m_pBuffer = new BYTE[m_ulSize];
			if (NULL != m_pBuffer)
			{
				m_dwResult = GetIpAddrTable((PMIB_IPADDRTABLE)m_pBuffer, &m_ulSize, TRUE);
				if (m_dwResult == NO_ERROR)
				{
					pAddrTable = (PMIB_IPADDRTABLE)m_pBuffer;

					for (int x = 0; x < (int)pAddrTable->dwNumEntries; x++)
					{
						pAddrRow = (PMIB_IPADDRROW) & (pAddrTable->table[x]);

						ia.S_un.S_addr = pAddrRow->dwAddr;
						char IPMsg[100] = { 0 };
						if (IfIndex == pAddrRow->dwIndex)
						{
							LPCSTR psz = inet_ntoa(ia);
							if (psz)
							{
								strLoalhostIp = psz;
							}
							delete[] m_pBuffer;

							//成功
							return 0;
						}
					}
				}
				else
				{

				}
				delete[] m_pBuffer;
			}
		}
	}
	else
	{
		//NOIPROUTETABLE
		FreeIpForwardTable(pIpRouteTable);
		return 1;
	}

	return 1;
#else
	strLoalhostIp = GetLocalIpAddress();

	return 0;
#endif // WIN32

}

std::string CommonTools::GetSycGatewaySN()
{
	srand((unsigned int)time(NULL));

	ostringstream oss;
	oss.str("");

	oss << GetMacAddressExt();
	oss << "-";
	oss << rand() % 10000;

	return oss.str();
}

std::string CommonTools::Base64Decode(char* input, int length)
{
	std::string strRet;

	BIO* b64 = NULL;
	BIO* bmem = NULL;

	char* buffer = (char*)malloc(length);
	memset(buffer, 0, length);

	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_new_mem_buf(input, length);
	bmem = BIO_push(b64, bmem);
	int ret = BIO_read(bmem, buffer, length);
	strRet.append(buffer, ret);

	BIO_free_all(bmem);
	free(buffer);

	return strRet;
}

std::string CommonTools::Base64Encode(char* input, int length)
{
	BIO* bmem = NULL;
	BIO* b64 = NULL;
	BUF_MEM* bptr = NULL;

	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);
	BIO_set_close(b64, BIO_NOCLOSE);

	char* buff = (char*)malloc(bptr->length + 1);
	memcpy(buff, bptr->data, bptr->length);
	buff[bptr->length] = '\0';

	BIO_free_all(b64);

	return buff;
}

std::string CommonTools::Aes256Decrypt(const std::string& cipherText, const std::string& key, unsigned char* iv)
{
	AES_KEY aes_key;

	if (AES_set_decrypt_key((const unsigned char*)key.c_str(), 256, &aes_key) < 0)
	{
		//assert(false);
		return "";
	}

	std::string strRet;
	for (unsigned int i = 0; i < cipherText.length() / AES_BLOCK_SIZE; i++)
	{
		std::string str16 = cipherText.substr(i * AES_BLOCK_SIZE, AES_BLOCK_SIZE);
		unsigned char out[AES_BLOCK_SIZE];
		::memset(out, 0, AES_BLOCK_SIZE);
		AES_cbc_encrypt((const unsigned char*)str16.c_str(), out, AES_BLOCK_SIZE, &aes_key, iv, AES_DECRYPT);
		strRet += std::string((const char*)out, AES_BLOCK_SIZE);
	}

	return strRet;
}

std::string CommonTools::Aes256Decrypt(const unsigned char* cipherText, int len, const unsigned char* key, unsigned char* iv)
{
	AES_KEY aes_key;

	if (AES_set_decrypt_key(key, 256, &aes_key) < 0)
	{
		return "";
	}

	std::string strRet;
	for (int i = 0; i < len / AES_BLOCK_SIZE; i++)
	{
		unsigned char str16[AES_BLOCK_SIZE] = { 0 };
		memcpy(str16, cipherText + i * AES_BLOCK_SIZE, AES_BLOCK_SIZE);

		unsigned char out[AES_BLOCK_SIZE] = { 0 };
		AES_cbc_encrypt((const unsigned char*)str16, out, AES_BLOCK_SIZE, &aes_key, iv, AES_DECRYPT);
		strRet += std::string((const char*)out, AES_BLOCK_SIZE);
	}
	
	return strRet;
}

std::string CommonTools::Aes256Encrypt(const std::string& clearText, const std::string& key, unsigned char* iv)
{
	AES_KEY aes_key;
	if (AES_set_encrypt_key((const unsigned char*)key.c_str(), 256, &aes_key) < 0)
	{
		//assert(false);
		return "";
	}
	std::string strRet;
	std::string data_bak = clearText;
	unsigned int data_length = (unsigned int)data_bak.length();

	// ZeroPadding
	int padding = 0;
	if (data_bak.length() % (AES_BLOCK_SIZE) > 0)
	{
		padding = AES_BLOCK_SIZE - data_bak.length() % (AES_BLOCK_SIZE);
	}
	// 在一些软件实现中，即使是16的倍数也进行了16长度的补齐
	/*else
	{
		padding = AES_BLOCK_SIZE;
	}*/

	data_length += padding;
	while (padding > 0)
	{
		data_bak += '\0';
		padding--;
	}

	for (unsigned int i = 0; i < data_length / (AES_BLOCK_SIZE); i++)
	{
		std::string str16 = data_bak.substr(i * AES_BLOCK_SIZE, AES_BLOCK_SIZE);
		unsigned char out[AES_BLOCK_SIZE];
		::memset(out, 0, AES_BLOCK_SIZE);
		AES_cbc_encrypt((const unsigned char*)str16.c_str(), out, AES_BLOCK_SIZE, &aes_key, iv, AES_ENCRYPT);
		strRet += std::string((const char*)out, AES_BLOCK_SIZE);
	}
	return strRet;
}

std::string CommonTools::RsaDecrypt(const std::string& cipherText, const std::string& key)
{
	std::string strRet;
	RSA* rsa = RSA_new();
	BIO* keybio;
	keybio = BIO_new_mem_buf((unsigned char*)key.c_str(), -1);

	rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
	if (!rsa)
	{
		BIO_free_all(keybio);
		return std::string("");
	}

	int len = RSA_size(rsa);
	char* decryptedText = (char*)malloc(len + 1);
	memset(decryptedText, 0, len + 1);

	// 解密函数  
	int ret = RSA_private_decrypt((int)cipherText.length(), (const unsigned char*)cipherText.c_str(), (unsigned char*)decryptedText, rsa, RSA_PKCS1_PADDING);
	if (ret >= 0)
		strRet = std::string(decryptedText, ret);
	else 
		printf("%s", ERR_reason_error_string(ERR_get_error()));

	// 释放内存  
	free(decryptedText);
	BIO_free_all(keybio);
	RSA_free(rsa);

	return strRet;
}

std::string CommonTools::RsaEncrypt(const std::string& clearText, std::string& pubKey)
{
	char* chPublicKey = const_cast<char*>(pubKey.c_str());

	std::string strRet;
	BIO* keybio = BIO_new_mem_buf(chPublicKey, -1);

	if (NULL == keybio)
	{
		return "";
	}
 
	RSA* rsa = RSA_new();
	rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
	if (!rsa)
	{
		BIO_free_all(keybio);
		return std::string("");
	}

	int len = RSA_size(rsa);
	//int len = 1028;
	char* encryptedText = (char*)malloc(len + 1);
	memset(encryptedText, 0, len + 1);

	// 加密函数  
	int ret = RSA_public_encrypt((int)clearText.length(), (const unsigned char*)clearText.c_str(), (unsigned char*)encryptedText, rsa, RSA_PKCS1_PADDING);
	if (ret >= 0)
		strRet = std::string(encryptedText, ret);

	// 释放内存  
	free(encryptedText);
	BIO_free_all(keybio);
	RSA_free(rsa);

	return strRet;
}

//WIN获取当前用户目录
std::string CommonTools::GetLocalUsrPath()
{
#ifdef WIN32
	std::string path = "";
	const char* homeProfile = "USERPROFILE";
	char homePath[1024] = { 0 };

	unsigned int pathSize = GetEnvironmentVariable(homeProfile, homePath, 1024);

	if (pathSize == 0 || pathSize > 1024)
	{
		// 获取失败 或者 路径太长 
		int ret = GetLastError();
	}
	else
	{
		path = std::string(homePath);
	}

	return path;
#else
	return "";
#endif // WIN32
}

std::string CommonTools::UTCToBeijing(int type, unsigned int UTCyear, unsigned char UTCmonth, unsigned char UTCday, unsigned int UTChour, unsigned char UTCminute, unsigned char UTCsecond)
{
	int year = 0, month = 0, day = 0, hour = 0;
	int lastday = 0;// 月的最后一天日期
	int lastlastday = 0;//上月的最后一天日期

	year = UTCyear;
	month = UTCmonth;
	day = UTCday;
	hour = UTChour + 8;//UTC+8转换为北京时间

	if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
	{
		lastday = 31;
		if (month == 3)
		{
			if ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0))//判断是否为闰年
				lastlastday = 29;//闰年的2月为29天，平年为28天
			else
				lastlastday = 28;
		}
		if (month == 8)
			lastlastday = 31;
	}
	else if (month == 4 || month == 6 || month == 9 || month == 11)
	{
		lastday = 30;
		lastlastday = 31;
	}
	else
	{
		lastlastday = 31;
		if ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0))//闰年的2月为29天，平年为28天
			lastday = 29;
		else
			lastday = 28;
	}

	if (hour >= 24)//当算出的时大于或等于24：00时，应减去24：00，日期加一天
	{
		hour -= 24;
		day += 1;
		if (day > lastday)//当算出的日期大于该月最后一天时，应减去该月最后一天的日期，月份加上一个月
		{
			day -= lastday;
			month += 1;

			if (month > 12)//当算出的月份大于12，应减去12，年份加上1年
			{
				month -= 12;
				year += 1;
			}
		}
	}

	char bjttbuf[30] = { 0 };
	std::string time;

	if (0 == type)
	{
		sprintf((char*)bjttbuf, "%02d:%02d:%02d", hour, UTCminute, UTCsecond); //UTC日期时分秒转换成北京时间
	}
	else if (1 == type)
	{
		sprintf((char*)bjttbuf, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, UTCminute, UTCsecond); //UTC日期时分秒转换成北京时间
	}
	else if (2 == type)
	{
		sprintf((char*)bjttbuf, "%.4d%.2d%.2dT%.2d%.2d%.2dZ", year, month, day, hour, UTCminute, UTCsecond);
	}
	else
	{

	}

	time.append((char*)bjttbuf);

	return time;
}

std::string CommonTools::BeijingToUTC(int type, unsigned int UTCyear, unsigned char UTCmonth, unsigned char UTCday, unsigned int UTChour, unsigned char UTCminute, unsigned char UTCsecond)
{
	int year = 0, month = 0, day = 0, hour = 0;
	int lastday = 0;// 月的最后一天日期
	int lastlastday = 0;//上月的最后一天日期

	year = UTCyear;
	month = UTCmonth;
	day = UTCday;
	hour = UTChour - 8;//UTC-8转换为UTC

	if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
	{
		lastday = 31;
		if (month == 3)
		{
			if ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0))//判断是否为闰年
				lastlastday = 29;//闰年的2月为29天，平年为28天
			else
				lastlastday = 28;
		}
		if (month == 8)
			lastlastday = 31;
	}
	else if (month == 4 || month == 6 || month == 9 || month == 11)
	{
		lastday = 30;
		lastlastday = 31;
	}
	else
	{
		lastlastday = 31;
		if ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0))//闰年的2月为29天，平年为28天
			lastday = 29;
		else
			lastday = 28;
	}

	if (hour < 0)//当算出的时小于24：00时，应加上24：00，日期减一天
	{
		hour += 24;
		day -= 1;
		if (day <= 0)//当算出的日期小于等于0时，应最后一天的日期，月份减少一个月
		{
			day = lastday;
			month -= 1;

			if (month <= 0)//当算出的月份小于等于0，应为12，年份减少1年
			{
				month = 12;
				year -= 1;
			}
		}
	}

	char bjttbuf[30] = { 0 };
	std::string time;

	if (0 == type)
	{
		sprintf((char*)bjttbuf, "%02d:%02d:%02d", hour, UTCminute, UTCsecond); //UTC日期时分秒转换成北京时间
	}
	else if (1 == type)
	{
		sprintf((char*)bjttbuf, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, UTCminute, UTCsecond); //UTC日期时分秒转换成北京时间
	}
	else if (2 == type)
	{
		sprintf((char*)bjttbuf, "%.4d%.2d%.2dT%.2d%.2d%.2dZ", year, month, day, hour, UTCminute, UTCsecond);
	}
	else if (3 == type)
	{
		sprintf((char*)bjttbuf, "%02d%02d%02d", hour, UTCminute, UTCsecond);
	}
	else
	{

	}

	time.append((char*)bjttbuf);

	return time;
}

std::string CommonTools::ClearAllSpace(std::string& src)
{
	int index = 0;
	std::string str = src;

	if (!str.empty()) 
	{
		//string::npos 表示查找没有匹配
		while ((index = (int)str.find(' ', index)) != std::string::npos) {
			str.erase(index, 1);
		}
	}

	return str;
}

int CommonTools::GetCurrDoubleDigit(double num)
{
	num = num - (int)num;    
	for (int i = 0; i < 15; i++) 
	{ 
		num *= 10;        
		if (num - (int)num == 0) 
		{ 
			return i + 1;
		} 
	} 

	return 0;
}

#ifdef WIN32
void GetCpuInfo(std::string& chProcessorType, DWORD& dwMaxClockSpeed)
{
	std::string strPath = _T("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0");//注册表子键路径
	CRegKey regkey;//定义注册表类对象
	LONG lResult;//LONG型变量－反应结果
	lResult = regkey.Open(HKEY_LOCAL_MACHINE, LPCTSTR(strPath.c_str()), KEY_ALL_ACCESS); //打开注册表键
	if (lResult != ERROR_SUCCESS)
	{
		return;
	}

	//查询CPU主频
	DWORD dwValue;
	if (ERROR_SUCCESS == regkey.QueryDWORDValue(_T("~MHz"), dwValue))
	{
		dwMaxClockSpeed = dwValue;
	}
	regkey.Close();//关闭注册表

	//获取CPU核心数目
	SYSTEM_INFO si;
	memset(&si, 0, sizeof(SYSTEM_INFO));
	GetSystemInfo(&si);

	switch (si.dwProcessorType)
	{
	case PROCESSOR_INTEL_386:
	{
		chProcessorType = "Intel 386 processor";
	}
	break;
	case PROCESSOR_INTEL_486:
	{
		chProcessorType = "Intel 486 Processor";
	}
	break;
	case PROCESSOR_INTEL_PENTIUM:
	{
		chProcessorType = "Intel Pentium Processor";
	}
	break;
	case PROCESSOR_INTEL_IA64:
	{
		chProcessorType = "Intel IA64 Processor";
	}
	break;
	case PROCESSOR_AMD_X8664:
	{
		chProcessorType = "AMD X8664 Processor";
	}
	break;
	default:
		chProcessorType = "UNKNOW";
		break;
	}
}

void GetDiskInfo(unsigned long long& total, unsigned long long& free)
{
	//通过GetLogicalDriveStrings()函数获取所有驱动器字符串信息长度
	int DSLength = GetLogicalDriveStrings(0, NULL);

	CHAR* DStr = new CHAR[DSLength];
	memset(DStr, 0, DSLength);

	//通过GetLogicalDriveStrings将字符串信息复制到堆区数组中,其中保存了所有驱动器的信息。
	GetLogicalDriveStrings(DSLength, DStr);

	int DType;

	BOOL fResult;
	unsigned _int64 i64FreeBytesToCaller;
	unsigned _int64 i64TotalBytes;
	unsigned _int64 i64FreeBytes;

	//读取各驱动器信息，由于DStr内部数据格式是A:\NULLB:\NULLC:\NULL，所以DSLength/4可以获得具体大循环范围
	for (int i = 0; i < DSLength / 4; ++i)
	{
		Sleep(10);
		CHAR* strdriver = DStr + (int)i * 4;
		DType = GetDriveType(strdriver);//GetDriveType函数，可以获取驱动器类型，参数为驱动器的根目录
		
		//本地磁盘
		if (DRIVE_FIXED != DType)
		{
			continue;
		}

		//GetDiskFreeSpaceEx函数，可以获取驱动器磁盘的空间状态,函数返回的是个BOOL类型数据
		fResult = GetDiskFreeSpaceEx(strdriver,
			(PULARGE_INTEGER)&i64FreeBytesToCaller,
			(PULARGE_INTEGER)&i64TotalBytes,
			(PULARGE_INTEGER)&i64FreeBytes);

		if (fResult)
		{
			total = i64TotalBytes;
			free = i64FreeBytesToCaller;
		}	
	}

	if (DStr)
	{
		delete[] DStr;
	}
}

#endif // WIN32

std::string CommonTools::GetCpuType()
{
#ifdef WIN32
	//软网关暂时不需要
	#if 0
	std::string type;
	DWORD speed;
	GetCpuInfo(type, speed);

	return type;
	#else
	return "";
	#endif

#else
	std::string cpuType;

	std::fstream file("/proc/cpuinfo", std::fstream::in);

	if (file.is_open())
	{
		if (!file)
			return "";

		std::string text;
		size_t pos = 0;

		while (std::getline(file, text))
		{
			//model name	: ARMv7 Processor rev 5 (v7l)
			pos = text.find("model name");
			if (text.npos != pos)
			{
				break;
			}
		}
		//获取cpu类型
		pos = text.find(":");
		if (text.npos != pos)
		{
			cpuType = text.substr(pos + 1, text.size());
		}

		file.close();
	}

	return cpuType;
#endif //WIN32
}

std::string CommonTools::GetCpuRate()
{
#ifdef WIN32

	//软网关暂时不需要
	#if 0
	std::string type;
	DWORD speed;
	GetCpuInfo(type, speed);

	return std::to_string(speed);
	#else
	return "";
	#endif

#else
	std::string cpuRate;

	std::fstream file("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", std::fstream::in);

	if (file.is_open())
	{
		if (!file)
			return "";

		std::getline(file, cpuRate);

		file.close();
	}

	return cpuRate;
#endif //WIN32
}

double getCpuUse(CPU_OCCUPY* o, CPU_OCCUPY* n)
{
	unsigned long od, nd;
	od = (unsigned long)(o->user + o->nice + o->system + o->idle + o->lowait + o->irq + o->softirq); //第一次(用户+优先级+系统+空闲)的时间再赋给od
	nd = (unsigned long)(n->user + n->nice + n->system + n->idle + n->lowait + n->irq + n->softirq); //第二次(用户+优先级+系统+空闲)的时间再赋给od
	double sum = nd - od;
	double idle = n->idle - o->idle;
	return (sum - idle) / sum;
}


#ifdef WIN32
__int64 Filetime2Int64(const FILETIME& ftime)
{
	LARGE_INTEGER li;
	li.LowPart = ftime.dwLowDateTime;
	li.HighPart = ftime.dwHighDateTime;
	return li.QuadPart;
}

__int64 CompareFileTime2(const FILETIME& preTime, const FILETIME& nowTime)
{
	return Filetime2Int64(nowTime) - Filetime2Int64(preTime);

}
#endif // WIN32

std::string CommonTools::GetCpuUse()
{
#ifdef WIN32

	//软网关暂时不需要
	#if 0
	FILETIME preIdleTime;
	FILETIME preKernelTime;
	FILETIME preUserTime;
	GetSystemTimes(&preIdleTime, &preKernelTime, &preUserTime);

	Sleep(500);

	FILETIME idleTime;
	FILETIME kernelTime;
	FILETIME userTime;
	GetSystemTimes(&idleTime, &kernelTime, &userTime);

	auto idle = CompareFileTime2(preIdleTime, idleTime);
	auto kernel = CompareFileTime2(preKernelTime, kernelTime);
	auto user = CompareFileTime2(preUserTime, userTime);

	if (kernel + user == 0)
		return "0";

	int64_t data = 100 * (kernel + user - idle) / (kernel + user);

	return std::to_string(data);
	#else
	return "";
	#endif

#else
	CPU_OCCUPY old_cpu_occupy;
	CPU_OCCUPY cpu_occupy;
	std::string cpu_use;

	int cnt = 0;

	do
	{
		if (cnt > 1)
		{
			//只循环2次获取2个时间戳之间的CPU占用(即时利用率)
			break;
		}

		FILE* fd = NULL;       // 定义打开文件的指针
		char buff[256]; // 定义个数组，用来存放从文件中读取CPU的信息

		fd = fopen("/proc/stat", "r");
		if (NULL == fd)
		{
			break;
		}

		// 读取第一行的信息，cpu整体信息
		fgets(buff, sizeof(buff), fd);

		// 返回与"cpu"在buff中的地址，如果没有，返回空指针
		if (NULL == strstr(buff, "cpu"))
		{
			break;
		}

		// 从字符串格式化输出
		sscanf(buff, "%s %u %u %u %u %u %u %u", cpu_occupy.name, &cpu_occupy.user, &cpu_occupy.nice, &cpu_occupy.system, &cpu_occupy.idle, &cpu_occupy.lowait, &cpu_occupy.irq, &cpu_occupy.softirq);
		// CPU的占用率 = （当前时刻的任务占用cpu总时间-前一时刻的任务占用cpu总时间）/ （当前时刻 - 前一时刻的总时间）

		int data = getCpuUse(&old_cpu_occupy, &cpu_occupy) * 100;
		cpu_use = std::to_string(data);
		old_cpu_occupy = cpu_occupy;

		sleep_ms(500);
		++cnt;

		fclose(fd);
		fd = NULL;
	} while (true);

	return cpu_use;
#endif
}

double CommonTools::GetCpuTemperature()
{
#ifdef WIN32
	return 0;
#else
	std::string cpu_temp;

	do
	{
		FILE* fd = NULL;       // 定义打开文件的指针
		char buff[256]; // 定义个数组，用来存放从文件中读取CPU的信息

		fd = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
		if (NULL == fd)
		{
			break;
		}

		// 读取温度信息
		fgets(buff, sizeof(buff), fd);
		cpu_temp = buff;

		fclose(fd);
		fd = NULL;
	} while (false);

	return (atof(cpu_temp.c_str()) / 1000);
#endif
}

bool CommonTools::GetNetStat()
{
#ifdef WIN32
	Sleep(100);
	CoInitialize(NULL);
	//  通过NLA接口获取网络状态    
	IUnknown* pUnknown = NULL;
	BOOL   bOnline = false;
	//是否在线      
	HRESULT Result = CoCreateInstance(CLSID_NetworkListManager, NULL, CLSCTX_ALL, IID_IUnknown, (void**)&pUnknown);
	if (SUCCEEDED(Result)) {
		INetworkListManager* pNetworkListManager = NULL;
		if (pUnknown)            Result = pUnknown->QueryInterface(IID_INetworkListManager, (void**)&pNetworkListManager);
		if (SUCCEEDED(Result)) {
			VARIANT_BOOL IsConnect = VARIANT_FALSE;
			if (pNetworkListManager)            Result = pNetworkListManager->get_IsConnectedToInternet(&IsConnect);
			if (SUCCEEDED(Result)) {
				bOnline = (IsConnect == VARIANT_TRUE) ? true : false;
			}
		}        if (pNetworkListManager)            pNetworkListManager->Release();
	}    if (pUnknown)        pUnknown->Release();
	CoUninitialize();

	return bOnline;
#else
	//网络状态检测能否PING华为云服务器
	system("ping 49.4.80.243 -c 2 -w 2 > ./temp/netlog");
	sleep_ms(2000);

	//把文件一行一行读取放入vector
	ifstream infile;
	string s;
	vector<string> v;

	infile.open("./temp/netlog");
	while (infile)
	{
		getline(infile, s);
		if (infile.fail())
			break;
		v.push_back(s);
	}
	infile.close();

	system("rm ./temp/netlog");

	//读取倒数第二行 2 packets transmitted, 2 received, 0% packet loss, time 1001ms
	if (v.size() > 1)
	{
		string data = v[v.size() - 2];
		int iPos = data.find("received,");
		if (iPos != -1)
		{
			data = data.substr(iPos + 10, 3);//截取字符串返回packet loss
			int  n = atoi(data.c_str());
			if (n == 0)
				return true;
			else
				return false;
		}
	}
	else
	{
		return false;
	}
#endif
}

bool CommonTools::GetSIMCardStatus()
{
#ifdef WIN32
	return false;
#else
	//获取4G信号写入4gSignal文件
	std::string cmd = "echo -e \"AT+CPIN?\r\n\" | microcom -t 10 /dev/ttyUSB2  > ./temp/4gSIMCard";
	system(cmd.c_str());

	bool status = false;
	std::fstream file("./temp/4gSIMCard", std::fstream::in);

	if (file.is_open())
	{
		if (!file)
			return false;

		std::string text;
		size_t pos = 0;

		while (std::getline(file, text))
		{
			//+CPIN: READY
			pos = text.find("+CPIN: READY");
			if (text.npos != pos)
			{
				status = true;
				break;
			}
		}

		file.close();
	}

	//删除临时文件4gSignal
	cmd = "rm ./temp/4gSIMCard";
	system(cmd.c_str());

	return status;
#endif // WIN32
}

unsigned long long CommonTools::GetSysTotalRam()
{
#ifdef WIN32

	//软网关暂时不需要
#if 0
	MEMORYSTATUS ms;
	::GlobalMemoryStatus(&ms);
	return (ms.dwTotalPhys / 1024);
#else
	return 0;
#endif
	
#else
	int mem_free = 0;//空闲的内存，=总内存-使用了的内存
	int mem_total = 0; //当前系统可用总内存

	char name[20] = { 0 };

	do
	{
		FILE* fp;
		char buf1[128] = { 0 }, buf2[128] = { 0 };
		int buff_len = 128;

		fp = fopen("/proc/meminfo", "r");
		if (fp == NULL)
		{
			break;
		}

		if (NULL == fgets(buf1, buff_len, fp) || NULL == fgets(buf2, buff_len, fp))
		{
			break;
		}

		fclose(fp);

		sscanf(buf1, "%s%d", name, &mem_total);
		sscanf(buf2, "%s%d", name, &mem_free);

	} while (false);

	return mem_total;
#endif
}

unsigned long long CommonTools::GetSysFreeRam()
{
#ifdef WIN32

	//软网关暂时不需要
#if 0
	MEMORYSTATUS ms;
	::GlobalMemoryStatus(&ms);
	return (ms.dwAvailPhys / 1024);
#else
	return 0;
#endif


#else
	int mem_free = 0;//空闲的内存，=总内存-使用了的内存
	int mem_total = 0; //当前系统可用总内存

	char name[20] = { 0 };

	do
	{
		FILE* fp;
		char buf1[128] = { 0 }, buf2[128] = { 0 };
		int buff_len = 128;

		fp = fopen("/proc/meminfo", "r");
		if (fp == NULL)
		{
			break;
		}

		if (NULL == fgets(buf1, buff_len, fp) || NULL == fgets(buf2, buff_len, fp))
		{
			break;
		}

		fclose(fp);

		sscanf(buf1, "%s%d", name, &mem_total);
		sscanf(buf2, "%s%d", name, &mem_free);

	} while (false);

	return mem_free;
#endif
}

unsigned long long CommonTools::GetSysUseRam()
{
#ifdef WIN32

	//软网关暂时不需要
#if 0
	MEMORYSTATUS ms;
	::GlobalMemoryStatus(&ms);
	return ms.dwMemoryLoad;
#else
	return 0;
#endif

#else
	int mem_free = 0;//空闲的内存，=总内存-使用了的内存
	int mem_total = 0; //当前系统可用总内存

	char name[20] = { 0 };

	do
	{
		FILE* fp;
		char buf1[128] = { 0 }, buf2[128] = { 0 };
		int buff_len = 128;

		fp = fopen("/proc/meminfo", "r");
		if (fp == NULL)
		{
			break;
		}

		if (NULL == fgets(buf1, buff_len, fp) || NULL == fgets(buf2, buff_len, fp))
		{
			break;
		}

		fclose(fp);

		sscanf(buf1, "%s%d", name, &mem_total);
		sscanf(buf2, "%s%d", name, &mem_free);

	} while (false);

	return mem_total - mem_free;
#endif
}

unsigned long long CommonTools::GetSysTotalRom()
{
#ifdef WIN32

	//软网关暂时不需要
#if 0
	unsigned long long total, free;

	GetDiskInfo(total, free);

	return total;
#else
	return 0;
#endif

#else
	struct statfs diskInfo;

	statfs("/", &diskInfo);
	unsigned long long blocksize = diskInfo.f_bsize; //每个block里包含的字节数
	unsigned long long totalsize = blocksize * diskInfo.f_blocks; //总的字节数，f_blocks为block的数目

	return totalsize;
#endif 
}

unsigned long long CommonTools::GetSysFreeRom()
{
#ifdef WIN32

	//软网关暂时不需要
#if 0
	unsigned long long total, free;

	GetDiskInfo(total, free);

	return free;
#else
	return 0;
#endif

#else
	struct statfs diskInfo;

	statfs("/", &diskInfo);
	unsigned long long blocksize = diskInfo.f_bsize; //每个block里包含的字节数
	unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小
	unsigned long long availableDisk = diskInfo.f_bavail * blocksize; //可用空间大小

	return availableDisk;
#endif 
}

void CommonTools::DeleteNumber(std::string& str)
{
	if (0 == str.size())
	{
		return;
	}

	int beforepos = 0, afterpos = 0;

	while (str[beforepos])
	{
		if (!(str[beforepos] >= '0' && str[beforepos] <= '9'))
		{
			str[afterpos++] = str[beforepos];
		}

		++beforepos;
	}

	str = str.substr(0, afterpos);
}

char* CommonTools::FilterNoneUtf8Chars(char* src, int* len)
{
	unsigned char* p = (unsigned char*)src;
	unsigned char* pSub = NULL;
	unsigned char* pStrEnd = (unsigned char*)src + (*len);
	unsigned char* pCharEnd = NULL;
	int bytes = 0;
	unsigned char* filtered = NULL;
	unsigned char* pDest = NULL;
	unsigned char* pInvalidCharStart = NULL;

	while (p < pStrEnd)
	{
		if (*p < 0x80)
		{
			p++;
			continue;
		}

		if ((*p & 0xE0) == 0xC0) //110xxxxx

		{
			bytes = 1;
		}
		else if ((*p & 0xF0) == 0xE0) //1110xxxx

		{
			bytes = 2;
		}
		else if ((*p & 0xF8) == 0xF0) //11110xxx

		{
			bytes = 3;
		}
		else if ((*p & 0xFC) == 0xF8) //111110xx


		{
			bytes = 4;
		}
		else if ((*p & 0xFE) == 0xFC) //1111110x

		{
			bytes = 5;
		}
		else
		{
			pInvalidCharStart = p;
			break;
		}

		p++;
		pCharEnd = p + bytes;
		if (pCharEnd > pStrEnd)
		{
			pInvalidCharStart = p - 1;
			break;
		}

		for (; p < pCharEnd; p++)
		{
			if ((*p & 0xC0) != 0x80)
			{
				break;
			}
		}

		if (p != pCharEnd)
		{
			pInvalidCharStart = pCharEnd - (bytes + 1);
			break;
		}
	}

	if (pInvalidCharStart == NULL) //all chars are valid

	{
		return src;
	}


	filtered = (unsigned char*)malloc(sizeof(char) * (*len));
	if (filtered == NULL)
	{
		*len = 0;
		*src = '\0';
		return src;
	}

	pDest = filtered;
	bytes = (char*)pInvalidCharStart - src;
	if (bytes > 0)
	{
		memcpy(pDest, src, bytes);
		pDest += bytes;
	}

	p = pInvalidCharStart + 1; //skip this invalid char

	while (p < pStrEnd)
	{
		if (*p < 0x80)
		{
			*pDest++ = *p++;
			continue;
		}

		if ((*p & 0xE0) == 0xC0) //110xxxxx

		{
			bytes = 1;
		}
		else if ((*p & 0xF0) == 0xE0) //1110xxxx

		{
			bytes = 2;
		}
		else if ((*p & 0xF8) == 0xF0) //11110xxx

		{
			bytes = 3;
		}
		else if ((*p & 0xFC) == 0xF8) //111110xx

		{
			bytes = 4;
		}
		else if ((*p & 0xFE) == 0xFC) //1111110x

		{
			bytes = 5;
		}

		else //invalid char

		{
			p++;
			continue;
		}

		pSub = p + 1;
		pCharEnd = pSub + bytes;
		if (pCharEnd > pStrEnd)
		{
			p++;
			continue;
		}

		for (; pSub < pCharEnd; pSub++)
		{
			if ((*pSub & 0xC0) != 0x80)
			{
				break;
			}
		}

		if (pSub != pCharEnd)
		{
			p++;
			continue;
		}

		bytes += 1;
		memcpy(pDest, pSub - bytes, bytes);
		pDest += bytes;
		p += bytes;
	}

	*len = pDest - filtered;
	memcpy(src, filtered, *len);
	*(src + (*len)) = '\0';

	free(filtered);

	return src;
}

std::string CommonTools::GetCmdResult(const std::string& strCmd)
{
	char* buf = new char[10240];
	if (NULL == buf)
	{
		return "";
	}

	memset(buf, 0, 10240);

	FILE* pf = NULL;

	#ifdef WIN32
	pf = _popen(strCmd.c_str(), "r");
	#else
	pf = popen(strCmd.c_str(), "r");
	#endif // WIN32

	if (pf == NULL)
	{
		delete []buf;
		return "";
	}

	std::string strResult;
	while (fgets(buf, sizeof buf, pf))
	{
		strResult += buf;
	}

	#ifdef WIN32
	_pclose(pf);
	#else
	pclose(pf);
	#endif // WIN32

	unsigned int iSize = (unsigned int)strResult.size();
	if (iSize > 0 && strResult[iSize - 1] == '\n')  // linux
	{
		strResult = strResult.substr(0, iSize - 1);
	}

	delete []buf;

	return strResult;
}

bool CommonTools::ChangeAsciiMsgToHex(const unsigned char* asciiMsg, unsigned int len, unsigned char* hexBuff)
{
	if (NULL == asciiMsg)
	{
		return false;
	}

	unsigned char _msg[1024] = { 0 };

	for (size_t i = 0; i < len; i++)
	{
		if (0x40 < asciiMsg[i])
		{
			_msg[i] = asciiMsg[i] - 0x37;
		}
		else
		{
			_msg[i] = asciiMsg[i] - 0x30;
		}
	}

	for (size_t i = 0; i < len; i++)
	{
		hexBuff[i] = ((_msg[i * 2] << 4) & 0xf0) | _msg[i * 2 + 1];
	}

	return true;
}
