#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include "CommonDef.h"

#include <string>
#ifdef WIN32
#include <direct.h>
#include <Windows.h>
#endif // WIN32


#define DECLARE_SINGTON(className)  \
	public:\
	static className * GetInstance();\
	private:\
	static className  m_Instance;\
	className(const className& obj);\
	className & operator = (const className& obj);

#define IMPLEMENT_SINGTON(className)\
	className className::m_Instance;\
	className * className::GetInstance(){\
	return &m_Instance;\
}


PACKET_T* GetPacket(int size);
void FreePacket(PACKET_T* packet);

void sleep_ms(int ms);

std::string GetCurrentDirPath(void);


#endif//_PUBLIC_H_
