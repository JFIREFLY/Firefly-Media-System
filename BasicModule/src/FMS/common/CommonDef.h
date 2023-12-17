#ifndef _COMMON_DEF_H_
#define _COMMON_DEF_H_

#include <map>
#include <string>

typedef struct tagPacket_T
{
	char* data;
	int size;
	int type;
	int repush_times;
	std::map<std::string, std::string> param; //À©Õ¹²ÎÊý
	tagPacket_T()
	{
		data = NULL;
		size = 0;
		type = 0;
		repush_times = 0;
	}
}PACKET_T, * LPPACKET_T;

#endif //_COMMON_DEF_H_

