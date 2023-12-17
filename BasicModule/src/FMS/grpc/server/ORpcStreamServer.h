#ifndef __RPC_STREAM_SERVER__
#define __RPC_STREAM_SERVER__

#include <string>
#include <list>
#include <grpcpp/server_builder.h>


//ʹ��˵����
// ����ʵ�֣�
//1.��RpcServerTemplate.h�У���������ӦPB�ṹ��RequestTemplateģ�庯��������ʵ�̶ֹ������ձ�������ģ��
//2.��RpcBusinessTemplate.h�У���������ӦPB�ṹ��MsgCallbackTemplateģ�庯�����յ���Ϣʱ����øĺ���������ʵ���Զ���
//3.��ORpcStreamServer.cpp�У�ע���ӦPB�ṹ�ķ���
//
// �÷���
// ʵ���������Start����


namespace RpcServer {

class ORpcStreamServer
{
public:
	ORpcStreamServer();
	~ORpcStreamServer();

	int Start(unsigned short port);
	void Stop();

private:
	void Register();

private:
	std::unique_ptr<grpc::Server> m_server;
	grpc::ServerBuilder m_builder;
	std::list<void*> m_handle_list;

private:
	std::mutex m_mutex;
};

}; //RpcServer

#endif