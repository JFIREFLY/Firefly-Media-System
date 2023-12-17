#include "OBasisInit.h"
#ifdef WIN32
#include <windows.h>
#endif // WIN32

OBasisInit::OBasisInit()
{
}


OBasisInit::~OBasisInit()
{
}

void OBasisInit::InitWin32()
{
#ifdef WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}
