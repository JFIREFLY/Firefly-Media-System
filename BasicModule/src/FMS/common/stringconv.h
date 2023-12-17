#ifndef __STRING_CONV_H__
#define __STRING_CONV_H__
#include<string>


std::string GbkToUtf8(const char* src_str);
std::string Utf8ToGbk(const char* src_str);

std::string GbkToUtf8(const char *src_str);
std::string Utf8ToGbk(const char *src_str);

std::string GbkToUnicode(const std::string& strGbk);
std::string UnicodeToGbk(const std::string& strUnicode);

std::string Utf8ToUnicode(const std::string& strUtf8);
std::string UnicodeToUtf8(const std::string& strUnicode);

#endif