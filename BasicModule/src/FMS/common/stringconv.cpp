#include <iostream>
#include <memory.h>
#include <stdlib.h>

using namespace std;

#define MSG_BODY_SIZE	102400

#ifdef _WIN32
#include <windows.h>


string GbkToUtf8(const char *src_str)
{
	wchar_t* wstr = NULL;
	char* str = NULL;

	string strTemp;
	int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);

	do
	{
		wstr = new wchar_t[len + 1];
		if (NULL == wstr)
		{
			break;
		}

		memset(wstr, 0, len + 1);
		MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
		len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);

		str = new char[len + 1];
		if (NULL == wstr)
		{
			break;
		}

		memset(str, 0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
		strTemp = str;

	} while (false);

	if (wstr) delete[] wstr;
	if (str) delete[] str;

	return strTemp;
}

string Utf8ToGbk(const char *src_str)
{
	wchar_t* wstr = NULL;
	char* str = NULL;

	string strTemp;
	int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);

	do
	{
		wstr = new wchar_t[len + 1];
		if (NULL == wstr)
		{
			break;
		}

		memset(wstr, 0, len * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wstr, len);
		len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);

		str = new char[len + 1];
		if (NULL == str)
		{
			break;
		}

		memset(str, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
		strTemp = str;

	} while (false);

	if (wstr) delete[] wstr;
	if (str) delete[] str;

	return strTemp;
}

/*********************************************************************
函数名      : multiByteToUnicode
功能        : 将多字节字符串转换为Unicode字符串（例如：gbk字符串转为unicode字符串；utf-8字符串转为unicode字符串）
参数        :
			  multiByteStr：指向待转换的字符串的缓冲区
			  multiByteStrLen：指定由参数multiByteStr指向的字符串中字节的个数
			  multiByteCharsetFlag：指定执行转换的多字节字符所使用的字符集，可选值有CP_ACP（gbk/gb2312）、CP_UTF8（utf-8）
			  unicodeStr：指向接收被转换字符串的缓冲区
			  unicodeStrLen：指定由参数unicodeStr指向的缓冲区的字节数
返回值      : 成功时，返回解析好的字节个数，错误时，返回相应错误码（-1：待转换的字符串为空；-2：待转换的字符串的长度小于等于0；-3：字符集设置不合理，必须是CP_ACP和CP_UTF8之一；-4：接收转换字符串缓存区为空；-5：接收转换字符串缓存区长度小于等于0；-6：接收被转换字符串的缓冲区小于实际需要的字节数）
*********************************************************************/
int multibyteToUnicode(const char* multiByteStr, int multiByteStrLen, UINT multiByteCharsetFlag, char* unicodeStr, int unicodeStrLen)
{
	int requiredUnicode = 0;
	WCHAR* tmp = NULL;

	do
	{
		if (multiByteStr == NULL)
		{
			requiredUnicode = -1;
			break;
		}

		if (multiByteStrLen <= 0)
		{
			requiredUnicode = -2;
			break;
		}

		if (multiByteCharsetFlag != CP_ACP && multiByteCharsetFlag != CP_UTF8)
		{
			requiredUnicode = -3;
			break;
		}

		if (unicodeStr == NULL)
		{
			requiredUnicode = -4;
			break;
		}

		if (unicodeStrLen <= 0)
		{
			requiredUnicode = -5;
			break;
		}

		//此种情况用来获取转换所需的wchar_t的个数
		requiredUnicode = MultiByteToWideChar(multiByteCharsetFlag, 0, multiByteStr, multiByteStrLen, NULL, 0);

		//接收被转换字符串的缓冲区小于实际需要的字节数
		if (sizeof(WCHAR) * requiredUnicode > unicodeStrLen)
		{
			requiredUnicode = -6;
			break;
		}

		//动态分配unicode临时缓存区
		tmp = new WCHAR[requiredUnicode];

		if (NULL == tmp)
		{
			requiredUnicode = -7;
			break;
		}

		//将临时缓存区数据清零
		wmemset(tmp, 0, requiredUnicode);

		//执行真实转换，并将转换后的unicode串写到tmp缓存区
		int parsedUnicode = MultiByteToWideChar(multiByteCharsetFlag, 0, multiByteStr, multiByteStrLen, tmp, requiredUnicode);

		//判断真实解析的unicode字符数和分配的字符数是否一致
		if (parsedUnicode != requiredUnicode)
		{
			requiredUnicode = -8;
			break;
		}

		//将目标unicode缓存区清零
		memset(unicodeStr, 0, unicodeStrLen);

		//将数据由临时缓存区拷贝到目标unicode缓存区
		memcpy(unicodeStr, tmp, sizeof(WCHAR) * requiredUnicode);


	} while (false);


	if (tmp)
	{
		delete[] tmp;
		tmp = NULL;
	}

	//返回解析好的总字节数
	if (requiredUnicode > 0){ requiredUnicode = sizeof(WCHAR) * requiredUnicode; }

	return requiredUnicode;
}

/*********************************************************************
函数名      : unicodeToMultibyte
功能        : 将Unicode字符串转换为多字节字符串（例如：将unicode字符串转换为gb2312/utf-8编码的字符串）
参数        :
			  unicodeStr：指向待转换的unicode字符串的缓冲区，该字符串必须是小端字节序
			  unicodeStrLen：指定由参数unicodeStr指向的字符串中字节的个数
			  multiByteStr：指向接收被转换字符串的缓冲区
			  multiByteStrLen：指定由参数multiByteStr指向的缓冲区的字节数
			  multiByteCharsetFlag：指定目标字符串所使用的字符集，可选值有CP_ACP（gbk/gb2312）、CP_UTF8（utf-8）
返回值      : 成功时，返回解析好的字节个数，错误时，返回相应错误码（-1：待转换的字符串为空；-2：待转换的字符串的长度小于等于0；-3：接收转换字符串缓存区为空；-4：接收转换字符串缓存区长度小于等于0；-5：字符集设置不合理，必须是CP_ACP和CP_UTF8之一；-6：接收被转换字符串的缓冲区小于实际需要的字节数）
*********************************************************************/
int unicodeToMultibyte(const char* unicodeStr, int unicodeStrLen, char* multiByteStr, int multiByteStrLen, UINT multiByteCharsetFlag)
{
	int parsedByte = 0;
	WCHAR* tmp = NULL;

	do
	{
		if (unicodeStr == NULL)
		{
			parsedByte = -1;
			break;
		}

		if (unicodeStrLen <= 0)
		{
			parsedByte = -2;
			break;
		}

		if (multiByteStr == NULL)
		{
			parsedByte = -3;
			break;
		}

		if (multiByteStrLen <= 0)
		{
			parsedByte = -4;
			break;
		}

		//目标字符串所使用的字符集既不是CP_ACP（gbk/gb2312）又不是CP_UTF8（utf-8）
		if (multiByteCharsetFlag != CP_ACP && multiByteCharsetFlag != CP_UTF8)
		{
			parsedByte = -5;
			break;
		}

		//动态分配用于存放原始Unicode字符串的临时缓存区
		tmp = new WCHAR[unicodeStrLen / 2];

		if (NULL == tmp)
		{
			parsedByte = -6;
			break;
		}

		//将临时缓存区清零
		wmemset(tmp, 0, unicodeStrLen / 2);

		//将原始Unicode字符串拷贝到临时缓存区
		memcpy(tmp, unicodeStr, unicodeStrLen);

		//获取用于存放目标字符串的字节数
		int requiredByte = WideCharToMultiByte(multiByteCharsetFlag, 0, tmp, unicodeStrLen / 2, NULL, 0, NULL, NULL);

		//接收被转换字符串的缓冲区小于实际需要的字节数
		if (requiredByte > multiByteStrLen)
		{
			parsedByte = -7;
			break;
		}

		//将目标缓存区清零
		memset(multiByteStr, 0, multiByteStrLen);

		//执行真正转换，将转换后的多字节字符串存放到目标缓存区中，并返回实际解析的字节数
		parsedByte = WideCharToMultiByte(multiByteCharsetFlag, 0, tmp, unicodeStrLen / 2, multiByteStr, multiByteStrLen, NULL, NULL);

		//实际解析的字节数不正确
		if (parsedByte != requiredByte)
		{
			parsedByte = -8;
			break;
		}

	} while (false);


	if (tmp)
	{
		delete[]tmp;
		tmp = NULL;
	}

	//返回解析好的总字节数
	return parsedByte;
}

std::string GbkToUnicode(const std::string& strGbk)
{
	std::string dstStr;
	char* dstBuf = new char[MSG_BODY_SIZE];

	int dstLenght = multibyteToUnicode(strGbk.c_str(), (int)strGbk.length(), CP_ACP, dstBuf, MSG_BODY_SIZE);

	if (dstLenght > 0)
	{
		dstStr.append(dstBuf, dstLenght);
	}

	if (dstBuf)
	{
		delete[] dstBuf;
		dstBuf = NULL;
	}

	return dstStr;
}

std::string UnicodeToGbk(const std::string& strUnicode)
{
	std::string dstStr;
	char* dstBuf = new char[MSG_BODY_SIZE];

	int dstLenght = unicodeToMultibyte(strUnicode.c_str(), (int)strUnicode.length(), dstBuf, MSG_BODY_SIZE, CP_ACP);

	if (dstLenght > 0)
	{
		dstStr.append(dstBuf, dstLenght);
	}

	if (dstBuf)
	{
		delete[] dstBuf;
		dstBuf = NULL;
	}

	return dstStr;
}

std::string Utf8ToUnicode(const std::string& strUtf8)
{
	std::string dstStr;
	char* dstBuf = new char[MSG_BODY_SIZE];

	int dstLenght = multibyteToUnicode(strUtf8.c_str(), (int)strUtf8.length(), CP_UTF8, dstBuf, MSG_BODY_SIZE);

	if (dstLenght > 0)
	{
		dstStr.append(dstBuf, dstLenght);
	}

	if (dstBuf)
	{
		delete[] dstBuf;
		dstBuf = NULL;
	}

	return dstStr;
}

std::string UnicodeToUtf8(const std::string& strUnicode)
{
	std::string dstStr;
	char* dstBuf = new char[MSG_BODY_SIZE];

	int dstLenght = unicodeToMultibyte(strUnicode.c_str(), (int)strUnicode.length(), dstBuf, MSG_BODY_SIZE, CP_UTF8);

	if (dstLenght > 0)
	{
		dstStr.append(dstBuf, dstLenght);
	}

	if (dstBuf)
	{
		delete[] dstBuf;
		dstBuf = NULL;
	}

	return dstStr;
}

#else
#include <iconv.h>
#include "stringconv.h"

std::string code_convert(char *source_charset, char *to_charset, const std::string& sourceStr) //sourceStr是源编码字符串  
{
	char outbuf[MSG_BODY_SIZE] = { 0 };
	size_t outlen = MSG_BODY_SIZE;

	do
	{
		size_t inlen = sourceStr.size();
		char* inbuf = (char*)sourceStr.c_str();

		memset(outbuf, 0, outlen);

		iconv_t cd = iconv_open(to_charset, source_charset);
		if (0 == cd)
		{
			break;
		}

		char* poutbuf = outbuf;
		if (iconv(cd, &inbuf, &inlen, &poutbuf, &outlen) == -1)
		{
			iconv_close(cd);
			break;
		}

		iconv_close(cd);

	} while (false);

	std::string strTemp;
	strTemp.append(outbuf, MSG_BODY_SIZE - outlen);
	return strTemp;
}

std::string GbkToUtf8(const char *src_str)
{
	return code_convert("gb2312", "utf-8", src_str);
}

std::string Utf8ToGbk(const char *src_str)
{
	return code_convert("utf-8", "gb2312", src_str);
}

std::string GbkToUnicode(const std::string& strGbk)
{
	//gbk转unicode,"UCS-2LE"代表unicode小端模式  
	return code_convert("gb2312", "UCS-2LE", strGbk);
}

std::string UnicodeToGbk(const std::string& strUnicode)
{
	return code_convert("UCS-2LE", "gb2312", strUnicode);
}

std::string Utf8ToUnicode(const std::string& strUtf8)
{
	//gbk转unicode,"UCS-2LE"代表unicode小端模式  
	return code_convert("utf-8", "UCS-2LE", strUtf8);
}

std::string UnicodeToUtf8(const std::string& strUnicode)
{
	return code_convert("UCS-2LE", "utf-8", strUnicode);
}

#endif
