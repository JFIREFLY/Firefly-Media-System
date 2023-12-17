#ifndef _COMMON_TOOLS_H_
#define _COMMON_TOOLS_H_

#include <string>
#include <ctime>
#include <list>

typedef struct times
{
	int Year;
	int Mon;
	int Day;
	int Hour;
	int Min;
	int Second;
}Times;

// ����һ��cpu occupy�Ľṹ�壬�������CPU����Ϣ
typedef struct CPUPACKED
{
	char name[20];       //����һ��char���͵�������name��20��Ԫ��
	unsigned int user;   //����һ���޷��ŵ�int���͵�user
	unsigned int nice;   //����һ���޷��ŵ�int���͵�nice
	unsigned int system; //����һ���޷��ŵ�int���͵�system
	unsigned int idle;   //����һ���޷��ŵ�int���͵�idle
	unsigned int lowait;
	unsigned int irq;
	unsigned int softirq;
} CPU_OCCUPY;

class CommonTools
{
public:
	virtual ~CommonTools() {};

	static std::string GetGUID();
	static std::string GetGUID2();
	static std::string GetGUID3();
	static unsigned short GetRandU16();
	static std::string GetRandStr(int len);
	static Times stamp_to_standard(unsigned int stampTime);
	static int standard_to_stamp(const char* str_time);
	static long long standard_to_stamp2(const char* str_time);
	static long long standard_to_stamp3(const char* str_time);
	static long long standard_to_stamp4(const char* str_time);
	static long long standard_to_stamp5(const char* str_time);
	static long long standard_to_stamp6(const char* str_time);
	static long long standard_to_stamp_ext(const char* str_time);
	static void StringToDatetime(std::string str, tm& tm_);
	static void StringToDatetime2(std::string str, tm& tm_);
	static void StringToDatetime3(std::string str, tm& tm_);
	static void StringToTimeForm1(std::string& str_src, std::string& str_dst);
	static std::string GetLocalTimeForm1();
	static std::string GetLocalTimeForm2();
	static std::string GetLocalTimeForm3();
	static std::string GetLocalTimeForm4();
	static std::string GetLocalTimeForm5();
	static std::string GetLocalTimeForm6();
	static std::string GetLocalTimeForm7();
	static std::string GetLocalTimeForm8();
	static std::string GetLocalTimeForm9();
	static int64_t GetLocalTimeStamp_ns();
	static int64_t GetLocalTimeStamp_ms();
	static int64_t GetLocalTimeStamp_s();

	static int GetLocalMillisecond();
	static std::string GetLocalTimeForm10();

	static std::string GetUtcTimeForm1();
	static std::string GetUtcTimeForm2();
	static std::string GetUtcTimeForm3();
	static std::string GetUtcTimeForm4();

	//static std::string ConvertUtcToLocalTime(struct tm* t2, const char* date);

	static std::string Hex2String(const unsigned char* pData, size_t nSize);
	static std::string String2Hex(std::string str, std::string separator = "");

	static std::string Dec2BinString(unsigned long long dec);

	static  std::string ToString(const int value);
	static  std::string ToString(const double value);

	static double String2Double(const std::string& str);

	static int String2Int(const std::string& str);
	static int HexString2Int(const std::string& str);

	static std::string  GetTimeString(const std::string& strDate, bool ignoreNowOnly = false);

	static void String2Time(std::string time_str, struct tm& tmbuf);
	static void String2TimeExt(std::string time_str, struct tm& tmbuf);

	static time_t String2Time(std::string time_str, bool standard = true);

	static bool CalcTimeLength(std::string start, std::string last, std::string& str_day, std::string& str_hour, std::string& str_minute);
	static bool CalcTimeLengthExt(std::string start, std::string last, unsigned long& timelength, bool standard = true);

	static uint16_t usMBCRC16(uint8_t * pucFrame, uint8_t usLen);

	static std::string DateToString(time_t time);
	static std::string DateToStringExt(long long time);
	static std::string DateToStringExt2(long long time);
	static std::string DateToStringExt3(long long time);
	static std::string DateTimeToString(long long time);
	static std::string DateTimeToStringExt(long long time);

	static bool IsChinese(const std::string& str);
	static int GetSubStrLen(const char* string, int len);
	static int HalfChinese_GBK(const char* input, int input_len, char* output);
	static int HalfChinese_UTF8(const char* input, int input_len, char* output);

	static bool CheckDateValid(const std::string& strDate);
	static bool CheckTimeValid(const std::string& strTime);
	static bool CheckDateTimeValid(const std::string& strDateTime);

	static std::string GetMacAddress();
	static std::string GetMacAddressExt();
	static bool GetAllMacAddress(std::list<std::string>& list);
	static bool GetAllMacAddressExt(std::list<std::string>& list);

	static std::string GetLocalIpAddress();
	static bool GetLocalIpAddress(std::list<std::string>& iplist);

	static int GetCurrentUsingIp(std::string& ip);
	static std::string GetSycGatewaySN();

	static std::string Base64Encode(char* input, int length);
	static std::string Base64Decode(char* input, int length);
	static std::string Aes256Decrypt(const unsigned char* cipherText, int len, const unsigned char* key, unsigned char* iv);
	static std::string Aes256Decrypt(const std::string& cipherText, const std::string& key, unsigned char* iv);
	static std::string Aes256Encrypt(const std::string& clearText, const std::string& key, unsigned char* iv);
	static std::string RsaDecrypt(const std::string& cipherText, const std::string& key);
	static std::string RsaEncrypt(const std::string& clearText, std::string& pubKey);

	static std::string GetLocalUsrPath();

	static std::string UTCToBeijing(int type, unsigned int UTCyear, unsigned char UTCmonth, unsigned char UTCday, unsigned int UTChour, unsigned char UTCminute, unsigned char UTCsecond);
	static std::string BeijingToUTC(int type, unsigned int UTCyear, unsigned char UTCmonth, unsigned char UTCday, unsigned int UTChour, unsigned char UTCminute, unsigned char UTCsecond);

	//ȥ��ȫ���ո�
	static std::string ClearAllSpace(std::string& src);

	static int GetCurrDoubleDigit(double num);

	//��ȡCPU�ͺ�
	static std::string GetCpuType();
	//��ȡCPU��Ƶ
	static std::string GetCpuRate();
	//��ȡCPUʹ����(��ʵ��Linux)
	static std::string GetCpuUse();
	//��ȡCPU�¶�(��ʵ��Linux)
	static double GetCpuTemperature();
	//��ȡ����״̬(��ʵ��Linux)
	static bool GetNetStat();
	//��ȡ4G SIM���Ƿ����
	static bool GetSIMCardStatus();
	//��ȡϵͳ�ڴ�����
	static unsigned long long GetSysTotalRam();
	//��ȡϵͳ�����ڴ�
	static unsigned long long GetSysFreeRam();
	//��ȡϵͳ�����ڴ�
	static unsigned long long GetSysUseRam();
	//��ȡӲ�����ڴ�
	static unsigned long long GetSysTotalRom();
	//��ȡӲ��ʣ���ڴ�
	static unsigned long long GetSysFreeRom();

	//ɾ���ַ����е�����
	static void DeleteNumber(std::string& str);

	//ɾ��utf8�ַ�������Ч�ַ�
	static char* FilterNoneUtf8Chars(char* src, int* len);

	//ִ��cmdָ����ؽ��
	static std::string GetCmdResult(const std::string& strCmd);

	static bool ChangeAsciiMsgToHex(const unsigned char* asciiMsg, unsigned int len, unsigned char* hexBuff);
};

#endif //_COMMON_TOOLS_H_
