#ifndef _H_MDUMP_H_
#define _H_MDUMP_H_



#ifdef  __cplusplus   
extern "C" {
#endif 

//����UnhandledExceptionFilter,���������������Ӧdump�ļ�������ڳ���һ��ʼ�ĵط����á�
//IsDumpWithFullMemory�Ƿ����������ڴ��dump�ļ�(dump���ܶ�)��true��������dump��false����normal��dump
//IsPreventOtherExceptionHandling������ó�true�����������ط���SetUnhandledExceptionFilter�ĵ���
 void  MiniDump(bool IsDumpWithFullMemory = true,
			  bool IsPreventOtherExceptionHandling = true);


//IsDumpWithFullMemory�Ƿ����������ڴ��dump�ļ�(dump���ܶ�)��true��������dump��false����normal��dump
 bool   getDump(bool IsDumpWithFullMemory = true);


#ifdef  __cplusplus
}


#endif


#endif