#include <iostream>

#include "common/OBasisInit.h"
#include "common/mdump.h"
#include "core/OServer.h"

int main()
{
    OBasisInit::InitWin32();
    MiniDump();

    if (OServer::GetInstance()->Init("fms.config"))
    {
        OServer::GetInstance()->Run();
    }
    else
    {
        printf("OServer Init fail");
    }

    OServer::GetInstance()->Loop();

    OServer::GetInstance()->Deinit();

    return 0;
}