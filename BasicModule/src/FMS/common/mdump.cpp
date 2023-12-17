
#pragma  warning(disable:4996)//ȥ����run-time library routines���뾯��

#include"mdump.h"

#ifdef _WIN32

#include <windows.h>
#include <Dbghelp.h>
#include  <io.h>
#include  <time.h>
#include <direct.h>
#include<string>

#endif


#define	R_A_S	200 * 1024 * 1024	//������ַ�ռ䡣


#ifdef _WIN32
//���������ڴ���������������Ϊ���˳�ʱ��ͨ�����������Զ��ͷ��ڴ�
class CReserveVirtualMemManage
{
public:
	CReserveVirtualMemManage():m_pAddrsSpace(NULL)
	{};

	void GetAddrsSpace(int dwSize)
	{
		ReleaseAddrsSpace();
		m_pAddrsSpace = VirtualAlloc(0, dwSize, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	};

	void ReleaseAddrsSpace()
	{
		if(m_pAddrsSpace)
		{
			VirtualFree(m_pAddrsSpace, 0, MEM_RELEASE);
			m_pAddrsSpace = NULL;
		}
	}

	~CReserveVirtualMemManage(){ ReleaseAddrsSpace(); };

private:
	 void*  m_pAddrsSpace;

};


static CReserveVirtualMemManage g_ReserveVirtualMemManage;
static bool g_IsDumpWithFullMemory = true;

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
										 CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
										 CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
										 CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
										 );



static void MsgError( const char* sDst )
{
	LPVOID lpMsgBuf;
	DWORD errNo = GetLastError();

	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errNo,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);

	char errMsg[1024] = {0};
	sprintf_s(errMsg,"%s (%s)",sDst, (char*)lpMsgBuf);

	::MessageBoxA( NULL, errMsg, "����", MB_OK ); 
	LocalFree( lpMsgBuf );
}



//��ȡdump�ļ�·����
static bool GetDumpFilePath(std::string& strDumpPath,bool bManual)
{

	char szModPath[_MAX_PATH];

	if (!GetModuleFileNameA( NULL, szModPath, _MAX_PATH ))
		return false;

	std::string strMode = "auto";
	if ( bManual )
		strMode = "manual";

	std::string strModPath = szModPath;

	std::string strProgName = strModPath.substr(strModPath.rfind("\\")+1);

	strModPath =  strModPath.substr(0, strModPath.rfind("\\"));


	std::string sDumpPath = strModPath + "\\Dump";

	//���������,����dumpĿ¼
	if (_access(sDumpPath.c_str(),0) != 0)
	{
		if( _mkdir(sDumpPath.c_str())!=0)
			return false;
	}


	//�ļ�������ʱ���
	time_t   tval; 
	struct   tm   *now; 
	CHAR   buf[255] = {0};   
	tval   =   time(NULL);   
	now   =   localtime(&tval);

	if ( now )
	{
		sprintf_s(buf,   "\\%s(%4d%02d%02d %02d%02d%02d %s).dmp",   strProgName.c_str(),
			now->tm_year+1900,   now->tm_mon+1,   now->tm_mday,
			now->tm_hour,   now->tm_min,   now->tm_sec, strMode.c_str());  
	}
	else   // ��ȡʱ��ʧ�ܵ����
	{
		sprintf_s(buf,   "\\%s(%4d%02d%02d %02d%02d%02d %s).dmp",   strProgName.c_str(),
			1900, 1, 1, 0, 0, 0,strMode.c_str());  
	}


	strDumpPath = sDumpPath + buf;

	return true;
}


//�������������
static bool ApplicationCrashHandler(EXCEPTION_POINTERS *pException,bool IsDumpWithFullMemory,bool bManual)
{

	HMODULE hDll = NULL;
	char szModPath[_MAX_PATH];


	//��ȡ��������·��
	std::string strModPath = "";

	//���Լ���ָ���汾 
	if (GetModuleFileNameA( NULL, szModPath, _MAX_PATH ))
	{

		strModPath = szModPath;
		strModPath =  strModPath.substr(0, strModPath.rfind("\\"));

		std::string strDbgHelpPath = strModPath + "\\DBGHELP.DLL";
		hDll = ::LoadLibraryA( strDbgHelpPath.c_str() );

	}

	if (hDll==NULL)
	{
		// ��������汾 
		hDll = ::LoadLibraryA( "DBGHELP.DLL" );
	}

	if(!hDll)//���ռ���dll���ɹ�
	{
		MsgError("������������δ֪�����˳�!����DBGHELP.DLLʧ��!");
		return false;

	}


	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if (pDump)
		{


			std::string strDumpPath;
			if(!GetDumpFilePath(strDumpPath,bManual))
			{
				FreeLibrary(hDll);
				MsgError("������������δ֪�����˳�!��ȡ�ļ���ʧ��!");
				return false;

			}


			HANDLE hFile = ::CreateFileA(  strDumpPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL );

			

			if (hFile!=INVALID_HANDLE_VALUE) 
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
			
				ExInfo.ThreadId = ::GetCurrentThreadId();
				ExInfo.ExceptionPointers = pException;
				//ExInfo.ClientPointers = NULL;
				ExInfo.ClientPointers = true;


				PMINIDUMP_EXCEPTION_INFORMATION pExceptInfo = NULL;

				if(pException)
					pExceptInfo = &ExInfo;

				_MINIDUMP_TYPE eDumpType = MiniDumpWithFullMemory;
				if(IsDumpWithFullMemory)
					eDumpType = MiniDumpWithFullMemory;
				else
					eDumpType = MiniDumpNormal;



				// write the dump
				BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile,  eDumpType, pExceptInfo, NULL, NULL );

				
				if (bOK)
				{

					::CloseHandle(hFile);
					::FreeLibrary(hDll);
					return true;
				}
				else
				{
					::CloseHandle(hFile);
					MsgError("������������δ֪�����˳�!����dump�ļ�ʧ��!");
					::FreeLibrary(hDll);
					return false;
				}

 

			}
			else
			{
				MsgError("������������δ֪�����˳�!����dump�ļ�ʧ��!");
				::FreeLibrary(hDll);
				return false;


			} 

		}
		::FreeLibrary(hDll);
	}

	return false;


}

static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter2( __in LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter )
{
	//printf("enter SetUnhandledExceptionFilter....\n");
	return 0;
}

static bool DisableSetUnhandledExceptionFilter()
{

	HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
	if (hKernel32 == NULL) 
	{
		MsgError("����kernel32.dll��غ�����ַʧ��!");
		return false;
	}

	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if(pOrgEntry == NULL) 
		return false;

	DWORD dwOldProtect = 0;
	SIZE_T jmpSize = 5;
#ifdef _WIN64
	jmpSize = 12;
#endif

	BOOL bProt = VirtualProtect(pOrgEntry, jmpSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	if (!bProt)
	{
		MsgError("����VirtualProtectʧ��!");

		return false;
	}

	
	void *pNewFunc = &SetUnhandledExceptionFilter2;
	uintptr_t dwOrgEntryAddr = (uintptr_t) pOrgEntry;
	uintptr_t dwNewEntryAddr = (uintptr_t) pNewFunc;
#ifdef _WIN64
	BYTE newJump[12] = {0x48,0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x50,0xC3};
	memcpy(&newJump[2],&dwNewEntryAddr, sizeof(dwNewEntryAddr) );
#else
	BYTE newJump[64];
	dwOrgEntryAddr += jmpSize; // add 5 for 5 op-codes for jmp rel32
	
	uintptr_t dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;
	// JMP rel32: Jump near, relative, displacement relative to next instruction.
	newJump[0] = 0xE9;  // JMP rel32
	memcpy(&newJump[1], &dwRelativeAddr, sizeof(dwOrgEntryAddr));
#endif

	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(), pOrgEntry, newJump, jmpSize, &bytesWritten);
	if (!bRet)
	{
		MsgError("����WriteProcessMemoryʧ��!");
		return false;
	}

	DWORD dwBuf;
	VirtualProtect(pOrgEntry, jmpSize, dwOldProtect, &dwBuf);
	return true;

}

static long DoMiniDump( void* pExceptionInfo,bool IsDumpWithFullMemory,bool bManual=false )
{
	g_ReserveVirtualMemManage.ReleaseAddrsSpace();


	if(pExceptionInfo)
	{
		ApplicationCrashHandler( (EXCEPTION_POINTERS*)pExceptionInfo,IsDumpWithFullMemory,bManual ); //�˴������������ַ�ռ䣬���򽫻��˳���
	}
	else
	{
		ApplicationCrashHandler( NULL ,IsDumpWithFullMemory,bManual);
		g_ReserveVirtualMemManage.GetAddrsSpace(R_A_S); 
	}

	return EXCEPTION_EXECUTE_HANDLER;

}

static void purecall_handler(void)
{
	//printf("enter purecall_handler handle\n");
	DoMiniDump( NULL ,g_IsDumpWithFullMemory);
} 



static void invalidparameter_handle(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
	//printf("enter invalidparameter_handle handle\n");
	DoMiniDump( (void*)expression ,g_IsDumpWithFullMemory);
}
#endif


//�ֶ�����dump�ļ�
bool getDump(bool IsDumpWithFullMemory )
{
#ifdef _WIN32
	DoMiniDump( NULL ,IsDumpWithFullMemory,true); 
#endif
	return true;


}


#ifdef _WIN32
static LONG WINAPI my_exception_handle(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	
	if(ExceptionInfo)
	{
		return DoMiniDump( (void*)ExceptionInfo ,g_IsDumpWithFullMemory);
	}
	else
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}
}
#endif


//�������ע�ắ��
//IsPreventOtherExceptionHandling������ó�true�����������ط���SetUnhandledExceptionFilter�ĵ���
void MiniDump(bool IsDumpWithFullMemory,
			  bool IsPreventOtherExceptionHandling)
{
#ifdef _WIN32
	::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
	::SetUnhandledExceptionFilter( (LPTOP_LEVEL_EXCEPTION_FILTER)my_exception_handle );
	::_set_purecall_handler(purecall_handler);
	::_set_invalid_parameter_handler(invalidparameter_handle);

	g_IsDumpWithFullMemory = IsDumpWithFullMemory;

	//���뱣�������ַ�ռ�
	g_ReserveVirtualMemManage.GetAddrsSpace(R_A_S);

	if(IsPreventOtherExceptionHandling)
		DisableSetUnhandledExceptionFilter();
#endif

	return;
}

