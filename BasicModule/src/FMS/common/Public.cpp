#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif // WIN32
#include <memory.h>
#include <cstdlib>
#include <algorithm>
#include <stdio.h>

#include "Public.h"


PACKET_T* GetPacket(int size)
{
	char* data = (char*)malloc(size);
	if (data)
	{
		memset(data, 0, size);
	}
	else
	{
		return NULL;
	}

	PACKET_T* packet = (PACKET_T*)malloc(sizeof(PACKET_T));
	if (packet)
	{
		packet->data = data;
		packet->size = 0;
		packet->type = 0;
	}
	else
	{
		free(data);
		data = NULL;

		return NULL;
	}

	return packet;
}

void FreePacket(PACKET_T* packet)
{
	if (packet)
	{
		if (packet->data)
		{
			free(packet->data);
			packet->data = NULL;
		}

		free(packet);
		packet = NULL;
	}
}

void sleep_ms(int ms)
{
	#ifdef WIN32
		Sleep(ms);
	#else
		usleep(ms*1000);
	#endif // WIN32
}


std::string GetCurrentDirPath(void)
{
	std::string temp;

	#ifdef WIN32
	char curPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, curPath, sizeof(curPath));
	temp = curPath;
	temp = temp.substr(0, temp.rfind('\\'));
	#else
	char curPath[260] = { 0 };
	readlink("/proc/self/exe", curPath, sizeof(curPath));
	temp = curPath;
	temp = temp.substr(0, temp.rfind('/'));
	#endif // WIN32

	return temp;
}
